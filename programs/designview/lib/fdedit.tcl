
proc fdedit_editFD { allAttrs lhs rhs {x 150} {y 150} } {

    global __FDEDIT__button

    if [winfo exists .fdedit] { return {} }

    set w [toplevel .fdedit -class FDEdit]
    wm title $w "FD Editor"
    wm geometry $w +$x+$y

    #
    # Create menus
    #
    frame $w.menubar -relief raised -bd 2

    #
    # Help menu
    #
    menubutton $w.menubar.help -text Help \
	    -menu $w.menubar.help.menu -underline 0
    set helpmenu [menu $w.menubar.help.menu -tearoff 0]
    $helpmenu add command -label "FD Editor" -underline 0 \
	    -command { dbn_showHelp fdedit }

    pack $w.menubar.help -side right -padx 10
    pack $w.menubar -side top -expand true -fill x

    set frame0 [frame $w.frame0 -relief ridge -bd 2]
    set label0 [label $frame0.label0 -text "All Attributes" \
	    -bg black -fg white]
    pack $label0 -side top -fill x -expand true

    set nAttrs [llength $allAttrs]

    set list0 [listbox $frame0.list -height 8 -selectmode extended]
    eval $list0 insert end $allAttrs
    if {$nAttrs > 8} {
	set scroll0 [scrollbar $frame0.scroll -orient vertical \
		-width 5 -command "$list0 yview"]
	$list0 config -yscrollcommand "$scroll0 set"
	pack $scroll0 -side right -fill y
    }
    pack $list0 -side top -fill x -expand true
    pack $frame0 -side top -pady 10

    set children_frame [frame $w.children_frame]
    pack $children_frame -side top -padx 10

    #
    # Create frame for lhs
    #
    set lframe [frame $children_frame.lframe -relief ridge -bd 2]
    frame $lframe.top
    frame $lframe.bottom
    set lhs_label [label $lframe.top.label -text "Antecedent"]
    pack $lhs_label -side top
    set lhs_list [listbox $lframe.top.list -height 8 -selectmode extended]
    eval $lhs_list insert end $lhs
    if {$nAttrs > 8} {
	set scroll [scrollbar $lframe.top.scroll -orient vertical \
		-width 5 -command "$lhs_list yview"]
	$lhs_list config -yscrollcommand "$scroll set"
	pack $scroll -side right -fill y
    }

    set lhs_add [button $lframe.bottom.add -text Add \
	    -command "copy_selected_lines $list0 $lhs_list"]
    set lhs_delete [button $lframe.bottom.delete -text Delete \
	    -command "delete_selected_lines $lhs_list"]
    pack $lhs_list -side top
    pack $lhs_add -side left -fill x -expand true
    pack $lhs_delete -side right -fill x -expand true

    #
    # Create frame for rhs
    #
    set rframe [frame $children_frame.rframe -relief ridge -bd 2]
    frame $rframe.top
    frame $rframe.bottom
    set rhs_label [label $rframe.top.label -text "Consequence"]
    pack $rhs_label -side top
    set rhs_list [listbox $rframe.top.list -height 8 -selectmode extended]
    eval $rhs_list insert end $rhs
    if {$nAttrs > 8} {
	set scroll [scrollbar $rframe.top.scroll -orient vertical \
		-width 5 -command "$rhs_list yview"]
	$rhs_list config -yscrollcommand "$scroll set"
	pack $scroll -side right -fill y
    }

    set rhs_add [button $rframe.bottom.add -text Add \
	    -command "copy_selected_lines $list0 $rhs_list"]
    set rhs_delete [button $rframe.bottom.delete -text Delete \
	    -command "delete_selected_lines $rhs_list"]
    pack $rhs_list -side top
    pack $rhs_add -side left -fill x -expand true
    pack $rhs_delete -side right -fill x -expand true

    pack $lframe.top -side top -fill x -expand true
    pack $lframe.bottom -side bottom -fill x -expand true
    pack $lframe -side left -padx 5

    pack $rframe.top -side top -fill x -expand true
    pack $rframe.bottom -side bottom -fill x -expand true
    pack $rframe -side left -padx 5

    set bframe [frame $w.button_frame -relief flat]
    pack $bframe -side bottom
    button $bframe.ok -text OK -width 10 -command {set __FDEDIT__button 1}
    button $bframe.cancel -text Cancel -width 10 \
	    -command {set __FDEDIT__button 0}
    pack $bframe.ok -side left -padx 10 -pady 10
    pack $bframe.cancel -side left -padx 10 -pady 10

    bind $w <Destroy> { set __FDEDIT__button 0 }

    set oldFocus [focus]
    tkwait visibility $w
    grab set $w
    focus $w
    
    while 1 {
	tkwait variable __FDEDIT__button
	if $__FDEDIT__button {
	    #
	    # User clicked on OK...
	    #
	    set ant [$lhs_list get 0 end]
	    set cons [set_difference [$rhs_list get 0 end] $ant]
	    if {$ant == {}} {
		dialog .createfd_error "Error" \
			"No antecedent was defined." \
			"" 0 {OK}
		unset __FDEDIT__button
	    } elseif {$cons == {}} {
		dialog .createfd_error "Error" \
			[list Either no consequence was defined, or \
			all attributes in the consequence are also \
			in the antecedent.] \
			"" 0 {OK}
		unset __FDEDIT__button
	    } else {
		set result [list $ant $cons]
		break
	    }
	} else {
	    #
	    # User clicked on Cancel or closed window...
	    #
	    set result {}
	    break
	}
    }

    if [winfo exists $w] { destroy $w }
    focus $oldFocus
    return $result

}

