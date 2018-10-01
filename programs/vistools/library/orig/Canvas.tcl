# Canvas.tcl - itcl class based on the Tk canvas widget
# 
# Features: 
# - automatic appearing and disappearing scrollbars (needs work...)
# - some support for resizing and rotating
#
# Usage: Canvas $this.c [options]
# (see public variables for options)
#
# You can access the underlying canvas widget as $this.c.canvas...
#
# Author: Allan Brighton (allan@piano.sta.sub.org)


itcl_class Canvas {
    inherit FrameWidget

    # create the canvas and scrollbars

    constructor {config} {
	FrameWidget::constructor

	set canvas [canvas $this.canvas -scrollregion {0 0 0 0}]
	scrollbar $this.vscroll \
		-relief sunken \
		-command "$this.canvas yview"
	scrollbar $this.hscroll \
		-orient horiz \
		-relief sunken \
		-command "$this.canvas xview"
	$this.canvas config \
		-xscroll "$this.hscroll set" \
		-yscroll "$this.vscroll set"
	
	bind $canvas <Configure> "$this resize %w %h"
	bind $canvas <ButtonPress-2> "$canvas scan mark %x %y"
	bind $canvas <B2-Motion> "$canvas scan dragto %x %y"

	if {!$auto_scrollbars} {
	    pack $this.hscroll -side bottom -fill x
	    pack $this.vscroll -side right -fill y
	}
	pack $canvas -fill both -expand 1

	#  Explicitly handle config's that may have been ignored earlier
	foreach attr $config {
	    config -$attr [set $attr]
	}
    }

    
    # cause a resize event so that the resize callback will be
    # called with the current canvas dimensions

    method resizing {} {
	scan [$canvas bbox all] {%d %d %d %d} x0 y0 x1 y1
	$canvas config -width [expr $x1-$x0] -height [expr $y1-$y0]
    }


    # rotate the named item 90 degrees by swapping the X and Y coords

    method rotate {item} {
	set ids [$canvas find withtag $item]
	foreach id $ids {
	    catch {
		set coords [$canvas coords $id]
		set n [llength $coords]
		set ncoords {}
		for {set i 0} {$i < $n} {incr i 2} {
		    lappend ncoords [lindex $coords [expr $i+1]] [lindex $coords $i]
		}
		eval $canvas coords $id $ncoords
	    }
	}
    }


    # rotate the named item on the X axis

    method rotatex {item} {
	scan [$canvas bbox $item] {%d %d %d %d} x0 y0 x1 y1
	set ids [$canvas find withtag $item]
	foreach id $ids {
	    catch {
		set coords [$canvas coords $id]
		set n [llength $coords]
		set ncoords {}
		for {set i 0} {$i < $n} {incr i 2} {
		    lappend ncoords [expr $x1-[lindex $coords $i]] \
			    [lindex $coords [expr $i+1]] 
		}
		eval $canvas coords $id $ncoords
	    }
	}
    }


    # rotate the named item on the Y axis

    method rotatey {item} {
	scan [$canvas bbox $item] {%d %d %d %d} x0 y0 x1 y1
	set ids [$canvas find withtag $item]
	foreach id $ids {
	    catch {
		set coords [$canvas coords $id]
		set n [llength $coords]
		set ncoords {}
		for {set i 0} {$i < $n} {incr i 2} {
		    lappend ncoords [lindex $coords $i] [expr $y1-[lindex $coords [expr $i+1]]] 
		}
		eval $canvas coords $id $ncoords
	    }
	}
    }

    
    # this method is called with the width and height of the
    # canvas when it is resized

    method resize {w h} {
	scan [$canvas bbox all] "%d %d %d %d" x0 y0 x1 y1

	if {$auto_scale} {
	    set xf [expr $w.0/($x1-$x0)]
	    set yf [expr $h.0/($y1-$y0)]
	    if {$equal_scale} {
		set f [min $xf $yf]
		$canvas scale all $x0 $y0 $f $f
	    } else {
		$canvas scale all $x0 $y0 $xf $yf
	    }
	}

	if {[scan [$canvas bbox all] "%d %d %d %d" x0 y0 x1 y1] != 4} {
	    return
	}

	# center the items
	$canvas config -scrollincrement 1 -scrollregion "$x0 $y0 $x1 $y1"
	if {$auto_center} {
	    $canvas yview [expr ($y1-$y0)/2-$h/2]
	    $canvas xview [expr ($x1-$x0)/2-$w/2]
	} elseif {$first_time} {
	    $canvas yview $y0
	    $canvas xview $x0
	    set first_time 0
	}

	if {$auto_scrollbars} {
	    pack forget $canvas
	    if {$x1 - $x0 > $w} {
		pack $this.hscroll -side bottom -fill x
	    } else {
		pack forget $this.hscroll
	    }
	    if {$y1 - $y0 > $h} {
		pack $this.vscroll -side right -fill y
	    } else {
		pack forget $this.yscroll
	    }
	    pack $canvas -fill both -expand 1
	}
    }


    # -- public variables --

    
    # if true, resizing the canvas also resizes the graphical
    # items in the canvas (text won't resize)
    public auto_scale 0

    # if true, and auto_scale is on, only scale items equally 
    # in the X and Y direction
    public equal_scale 1
    
    # if true, make scrollbars appear and disappear automatically
    # as needed
    public auto_scrollbars 0

    # if true, center the contents of the canvas after a resize
    public auto_center 0

    # pass these options on to the canvas widget
    foreach i {width height bg bd relief} {
	public $i {} [format {
	    if {[winfo exists $this.canvas]} {
		$this.canvas config -%s $%s
	    }
	} $i $i]
    }

    # -- protected variables --

    # canvas widget
    protected canvas

    # used for one time resizing
    protected first_time 1

}
