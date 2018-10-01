# CanvasPrint.tcl - Popup dialog for printing the contents of a Tk canvas
#
# Author: Allan Brighton (allan@piano.sta.sub.org) 


# This class extends the PrintDialog class to be able to 
# print the contents of a canvas as postscript

itcl_class CanvasPrint {
    inherit PrintDialog


    # print the contents of the canvas to the open filedescriptor
    
    method print {fd} {
	global $this.color $this.rotate $this.colormap

	# try to fit on a page
	scan [$canvas bbox all] "%d %d %d %d" x1 y1 x2 y2

	set cmd [list $canvas postscript \
		     -colormode [set $this.color] \
		     -width [expr $x2-$x1] \
		     -height [expr $y2-$y1] \
		     -pagewidth 8.5i \
		     -pageheight 11i \
		     -x $x1 \
		     -y $y1 \
		     -rotate [set $this.rotate]]

	if {"[set $this.color]" == "mono"} {
	    # you can add to this array, see canvas(n) man page
	    set $this.colormap(grey) "0.0 0.0 0.0 setrgbcolor"
	    lappend cmd -colormap $this.colormap
	} 

	puts $fd [eval $cmd]
    }



    constructor {config} {
	PrintDialog::constructor

	global $this.color $this.rotate

	pack [frame $this.options -bd 3 -relief groove] \
	    -side top -fill x -expand 1 -in $this.config

	pack [label $this.options.title -text "Postscript Options"] \
	    -side top 
	
	# color options 
	pack [frame $this.color -bd 5] \
	    -side top -fill x -expand 1 -in $this.options
	pack [radiobutton $this.color.color -text "Color" \
	      -variable $this.color \
	      -value color] \
        [radiobutton $this.color.gray -text "Gray-Scale" \
	     -variable $this.color \
	     -value gray] \
        [radiobutton $this.color.mono -text "Black & White" \
	     -variable $this.color \
	     -value mono] \
	-side left -fill x -expand 1
	set $this.color [tk colormodel $this]

	# rotate options 
	pack [frame $this.rotate -bd 5] \
	    -side top -fill x -expand 1 -in $this.options
	pack [radiobutton $this.rotate.yes -text "Landscape" \
	      -variable $this.rotate \
	      -value yes] \
        [radiobutton $this.rotate.no -text "Portrait" \
	     -variable $this.rotate \
	     -value no] \
	-side left -fill x -expand 1
	set $this.rotate yes
    }


    # -- public variables --

    # canvas widget
    public canvas {}

}


