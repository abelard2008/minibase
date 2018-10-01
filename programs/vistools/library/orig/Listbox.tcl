# Listbox.tcl - itcl widget for scrolled lists, 
#                based on the Tk listbox widget. 
#
# Note: the primitive operations of listboxes are not duplicated here,
# but you can access them through $this.listbox
#
# Author: Allan Brighton (allan@piano.sta.sub.org) 


itcl_class Listbox {
    inherit FrameWidget


    # set the contents of the listbox from the list

    method set_contents {list} {
	$listbox delete 0 end
	foreach i $list {
	    $listbox insert end $i
	}
    }


    # return the contents of the listbox as a tcl list

    method get_contents {} {
	set list {}
	set n [$listbox size]
	for {set i 0} {$i < $n} {incr i} {
	    lappend list [$listbox get $i]
	}
	return $list
    }


    # return a list of the selected items

    method get_selected {} {
	set list {}
	foreach i [$listbox curselection] {
	    lappend list [$listbox get $i]
	}
	return $list
    }


    # remove the selected items from the listbox
    # and return them as a list

    method remove_selected {} {
	set list [get_selected]
	set n 0
	foreach i [lsort -decreasing [$listbox curselection]] {
	    $listbox delete $i
	    incr n
	}
	if {$n} {
	    $listbox select from $i
	    $listbox select to $i
	}
	return $list
    }


    # append a line to the list

    method append {line} {
	$listbox insert end $line
    }

    
    # append a list of items to the list

    method append_list {list} {
	foreach i $list {
	    $listbox insert end $i
	}
    }

    
    # move the selected row down 1 row

    method move_down {} {
	set list [$listbox curselection]
	if {[lempty $list]} {
	    return
	}
	set rlist [lsort -decreasing $list]
	foreach i $rlist {
	    set s [$listbox get $i]
	    $listbox delete $i
	    $listbox insert [expr $i+1] $s
	}
	$listbox select from [expr [lindex $list 0]+1]
	$listbox select to [expr [lindex $rlist 0]+1]
    }

    
    # move the selected row up 1 row

    method move_up {} {
	set list [$listbox curselection]
	if {[lempty $list]} {
	    return
	}
	foreach i $list {
	    set s [$listbox get $i]
	    $listbox delete $i
	    $listbox insert [expr $i-1] $s
	}
	$listbox select from [expr [lindex $list 0]-1]
	$listbox select to [expr $i-1]
    }


    # make the list empty

    method clear {} {
	$listbox delete 0 end
    }


    #  create a new object of this class

    constructor {config} {
	FrameWidget::constructor

	if {"$title" != ""} {
	    pack [label $this.title -text $title] \
		    -side top -fill x
	}
	set listbox [listbox $this.listbox \
		-relief sunken \
		-bd 3 \
		-exportselection $exportselection \
		-setgrid 1]

	if {$vscroll} {
	    scrollbar $this.vscroll -relief sunken \
		    -command "$listbox yview"
	    pack $this.vscroll -side right -fill y
	    $listbox config -yscroll "$this.vscroll set"
	}

	if {$hscroll} {
	    scrollbar $this.hscroll -relief sunken \
		    -command "$listbox xview" \
		    -orient horizontal
	    pack $this.hscroll -side bottom -fill x
	    $listbox config -xscroll "$this.hscroll set"
	}

	pack $listbox -side top -fill both -expand 1

	if {$single_select} {
	    tk_listboxSingleSelect $listbox
	}
    }

    
    # -- public member variables --

    # title string for listbox
    public title {}

    # flag: if true, list will have a vertical scrollbar
    public vscroll {1}

    # flag: if true, list will have a horizontal scrollbar
    public hscroll {0}

    # flag: single selection mode
    public single_select {1}

    # flag: export selection ?
    public exportselection {1}

    
    # -- protected members --

    # listbox widget
    protected listbox

    # title label
    protected label

}


