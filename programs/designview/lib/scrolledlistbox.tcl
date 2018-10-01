
#
# ScrolledListBox
#
# Creates a frame containing a listbox and (optionally) a vertical
# scrollbar linked to the listbox. The 'scroll' argument determines
# whether or not the scrollbar is created.
#
# This is a slightly modified version of an example from Brent Welch's
# "Practical Programming in Tcl and Tk".
#

proc ScrolledListbox { parent scroll bar_width args } {

    frame $parent

    eval {listbox $parent.list} $args
    pack $parent.list -side left -fill both -expand true

    if {$scroll == 1} {
	$parent.list config -yscrollcommand [list $parent.sy set]
	scrollbar $parent.sy -orient vertical -width $bar_width \
		-command [list $parent.list yview]
	$parent.sy config -highlightthickness \
		[$parent.list cget -highlightthickness]
	pack $parent.sy -side right -fill y
    }

    return $parent.list

}

proc ScrolledListbox2 { parent scroll bar_width args } {

    frame $parent

    eval {listbox $parent.list} $args

    if {$scroll == 1} {
	$parent.list config -yscrollcommand [list $parent.sy set]
	scrollbar $parent.sy -orient vertical -width $bar_width \
		-command [list $parent.list yview]
	$parent.sy config -highlightthickness \
		[$parent.list cget -highlightthickness]
	pack $parent.sy -side right -fill y
    }

    pack $parent.list -side right -fill both -expand true

    return $parent.list

}

