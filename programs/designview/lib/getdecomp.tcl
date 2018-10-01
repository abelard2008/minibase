
proc dbn_getDecomposition { relation arrayname } {

    global __GETDCMP__button
    global __GETDCMP__parentRelation
    global __GETDCMP__childFrames
    global __GETDCMP__nChildren
    global __GETDCMP__parentList
    global __GETDCMP__toplevel
    global __GETDCMP__activeChild

    upvar $arrayname decomparray

    set rname [relation_getName $relation]
    set attrs [relation_getAttributes $relation]
    set n_attrs [llength $attrs]

    debug "Building decomposition window: rname=$rname, attrs=$attrs"

    set __GETDCMP__nChildren 0
    set __GETDCMP__parentRelation $relation
    set __GETDCMP__activeChild {}

    set w [toplevel .decomp_[newOID GetDecomp] -class DecompWindow]
    wm title $w "Decomposition of $rname"
    wm transient $w
    set __GETDCMP__toplevel $w

    #
    # Create menus
    #
    frame $w.menubar -relief raised -bd 2

    #
    # Decompose menu
    #
    menubutton $w.menubar.decomp -text Decompose \
	    -menu $w.menubar.decomp.menu -underline 0
    set decompmenu [menu $w.menubar.decomp.menu -tearoff 0]
    $decompmenu add command -label "Generate BCNF" -underline 9 \
	    -command "getDecomp_generateNormalForm BCNF"
    $decompmenu add command -label "Generate 3NF" -underline 9 \
	    -command "getDecomp_generateNormalForm 3NF"
    $decompmenu add separator
    $decompmenu add command -label "Add Child" -underline 0 \
	    -command "getDecomp_makeChildFrame"
    $decompmenu add command -label "Delete Child" -underline 0 \
	    -command "getDecomp_deleteChild"

    #
    # Test menu
    #
    menubutton $w.menubar.test -text Test \
	    -menu $w.menubar.test.menu -underline 0
    set testmenu [menu $w.menubar.test.menu -tearoff 0]
    $testmenu add command -label "Test Lossless Join" -underline 5 \
	    -command "getDecomp_testDecomposition {Lossless Join}"
    $testmenu add command -label "Test Dep. Preserving" -underline 5 \
	    -command "getDecomp_testDecomposition {Dependency Preserving}"

    #
    # Help menu
    #
    menubutton $w.menubar.help -text Help \
	    -menu $w.menubar.help.menu -underline 0
    set helpmenu [menu $w.menubar.help.menu -tearoff 0]
    $helpmenu add command -label "Custom Decomposition" -underline 0 \
	    -command { dbn_showHelp customdecomp }

    pack $w.menubar.decomp -side left -padx 10
    pack $w.menubar.test -side left -padx 10
    pack $w.menubar.help -side right -padx 10
    pack $w.menubar -side top -expand true -fill x

    set frame0 [frame $w.frame0 -relief ridge -bd 2]
    set label0 [label $frame0.label0 -text $rname -bg black -fg white]
    pack $label0 -side top -fill x -expand true

    set __GETDCMP__parentList \
	    [listbox $frame0.list -height 8 -selectmode extended]
    eval $__GETDCMP__parentList insert end $attrs
    if {$n_attrs > 8} {
	set scroll0 [scrollbar $frame0.scroll -orient vertical \
		-width 5 -command "$__GETDCMP__parentList yview"]
	$__GETDCMP__parentList config -yscrollcommand "$scroll0 set"
	pack $scroll0 -side right -fill y
    }

    pack $__GETDCMP__parentList -side top -fill x -expand true
    pack $frame0 -side top -pady 10

    set children_frame [frame $w.children]
    pack $children_frame -side top -padx 10

    set __GETDCMP__childFrames {}
    getDecomp_makeChildFrame
    getDecomp_makeChildFrame

    getDecomp_setActiveChild [lindex $__GETDCMP__childFrames 0]

    set bframe [frame $w.button_frame -relief flat]
    pack $bframe -side bottom
    button $bframe.ok -text Done -width 15 -command {set __GETDCMP__button 1}
    button $bframe.cancel -text Cancel -width 15 \
	    -command {set __GETDCMP__button 0}
    pack $bframe.ok -side left -padx 10 -pady 10
    pack $bframe.cancel -side left -padx 10 -pady 10

    bind DecompWindow <Destroy> { set __GETDCMP__button 0 }

    set oldFocus [focus]
    tkwait visibility $w
    grab set $w
    focus $w
    
    while 1 {
	tkwait variable __GETDCMP__button
	set valid 0
	if $__GETDCMP__button {
	    debug "    N children: $__GETDCMP__nChildren"
	    debug "    Frames: $__GETDCMP__childFrames"
	    set valid [getDecomp_buildDecompArray decomparray]
	    if $valid {
		set valid [validate_decomposition decomparray]
	    }
	    if $valid {
		set result OK
		break
	    } else {
		unset __GETDCMP__button
	    }
	} else {
	    set result {}
	    break
	}
    }

    if [winfo exists $w] { destroy $w }
    focus $oldFocus
    return $result
}

proc getDecomp_buildDecompArray { arrayname } {

    global __GETDCMP__childFrames
    upvar $arrayname decomparray

    if [info exists decomparray] { unset decomparray }

    foreach frame $__GETDCMP__childFrames {

	set name [$frame.top.name get]
	set attrlist [$frame.top.list get 0 end]

	if [info exists decomparray($name)] {
	    one_line_message .errmsg Error \
		    "Duplicate relation name: $name"
	    return 0
	}
	if {$name == {}} {
	    one_line_message .errmsg Error \
		    "A relation name is undefined."
	    return 0
	}
	if ![valid_relation_name $name] {
	    one_line_message .errmsg Error \
		    "Invalid relation name: $name"
	    return 0
	}
	if {$attrlist == {}} {
	    one_line_message .errmsg Error \
		    "A relation has an empty attribute set."
	    return 0
	}
	set decomparray($name) $attrlist
    }

    return 1

}

proc validate_decomposition { arrayname } {

    global __GETDCMP__parentRelation
    upvar $arrayname decomparray

    set parent $__GETDCMP__parentRelation
    set parent_attrs [relation_getAttributes $parent]

    debug "Validating decomposition"
    debug "    decomparray=[array get decomparray]"
    debug "    parent attrs=$parent_attrs"

    #
    # Make sure no new relations have identical attribute sets
    #
    #     Not implemented yet.
    #

    #
    # Make sure that the union of all attribute sets is equal to the 
    # attribute set for the original relation
    #
    set attr_union {}
    set missing {}
    foreach n [array names decomparray] {
	eval lappend attr_union $decomparray($n)
    }
    debug "    attr_union=$attr_union"
    foreach a $parent_attrs {
	if {[lsearch -exact $attr_union $a] == -1} {
	    lappend missing $a
	}
    }
    debug "    missing=$missing"
    if {$missing != {}} {
	one_line_message .errmsg "Decomposition Error" \
		"Some attributes are not part of the decomposition: \
		\n    [join $missing ,]"
	return 0
    }

    #
    # Ask user to confirm the decomp if it is not lossless join or 
    # dependency preserving
    #
    set confirm [db_confirmDecomposition \
	    $__GETDCMP__parentRelation decomparray]
    if {[lindex $confirm 0] == "-1"} {
	one_line_message .message "Decomposition Error" \
		"Error: [lrange $confirm 2 end]"
	return 0
    }
    if {$confirm != "TRUE"} {
	return 0
    }

    return 1

}

proc getDecomp_setActiveChild { frame } {

    global __GETDCMP__activeChild
    global __GETDCMP__childFrames

    set old $__GETDCMP__activeChild

    debug "getDecomp_setActive: old=$old"
    debug "                      new=$frame"

    if ![winfo exists $frame] { return }

    if {$old != {}} {
	if [winfo exists $old] {
	    $old config -background #d9d9d9 -relief ridge
	}
    }

    $frame config -background black -relief flat
    set __GETDCMP__activeChild $frame

}

proc getDecomp_deleteChild {} {

    global __GETDCMP__activeChild
    global __GETDCMP__nChildren
    global __GETDCMP__childFrames

    set frame $__GETDCMP__activeChild
    if {$frame == {}} {
	one_line_message .errmsg Error "You must select a relation first"
	return
    }
    if {$__GETDCMP__nChildren <= 2} {
	one_line_message .errmsg Error "You cannot delete any more children."
	return
    }

    if ![winfo exists $frame] { return }

    destroy $frame
    delete_from_list __GETDCMP__childFrames $frame
    getDecomp_setActiveChild [lindex $__GETDCMP__childFrames 0]
    incr __GETDCMP__nChildren -1
}

proc getDecomp_makeChildFrame {} {

    global __GETDCMP__parentRelation
    global __GETDCMP__childFrames
    global __GETDCMP__nChildren
    global __GETDCMP__parentList
    global __GETDCMP__activeChild
    global __GETDCMP__toplevel

    set rname [relation_getName $__GETDCMP__parentRelation]
    set attrs [relation_getAttributes $__GETDCMP__parentRelation]
    set nAttrs [llength $attrs]
    set w $__GETDCMP__toplevel

    incr __GETDCMP__nChildren
    set i $__GETDCMP__nChildren

    while {[winfo exists $w.children.frame$i]} { incr i }

    set f [frame $w.children.frame$i -relief ridge -bd 3]
    frame $f.top
    frame $f.bottom
    set name [entry $f.top.name -justify center -highlightthickness 0]
    bind $name <Button-1> "getDecomp_setActiveChild $f"
    $name insert 0 $rname$i
    pack $name -side top -fill x -expand true -padx 2 -pady 2
    set list [listbox $f.top.list -height 8 -selectmode extended]
    bind $list <Button-1> "getDecomp_setActiveChild $f"
    if {$nAttrs > 8} {
	set scroll [scrollbar $f.top.scroll -orient vertical \
		-width 5 -command "$list yview"]
	$list config -yscrollcommand "$scroll set"
	pack $scroll -side right -fill y
    }
    pack $list -side top
    
    set add [button $f.bottom.add -text Add \
	    -command "copy_selected_lines $__GETDCMP__parentList $list"]
    set delete [button $f.bottom.delete -text Delete \
	    -command "delete_selected_lines $list"]
    pack $add -side left -fill x -expand true
    pack $delete -side right -fill x -expand true
    
    pack $f.top -side top -fill x -expand true
    pack $f.bottom -side bottom -fill x -expand true
    
    lappend __GETDCMP__childFrames $f
    getDecomp_setActiveChild $f
    pack $f -side left -padx 5

}

proc getDecomp_setAttributes { childnumber attrs } {

    global __GETDCMP__childFrames

    set frame [lindex $__GETDCMP__childFrames $childnumber]
    if {$frame == {}} { return }

    set list $frame.top.list
    $list delete 0 end
    eval $list insert end $attrs
}

proc getDecomp_setChildName { childnumber name } {

    global __GETDCMP__childFrames

    set frame [lindex $__GETDCMP__childFrames $childnumber]
    if {$frame == {}} { return }

    set entry $frame.top.name
    $entry delete 0 end
    $entry insert 0 $name
}

proc getDecomp_testDecomposition { testname } {

    global __GETDCMP__parentRelation

    set valid [getDecomp_buildDecompArray decomparray]
    if !$valid { return }

    set relation $__GETDCMP__parentRelation

    set result [db_testDecomposition $relation decomparray $testname]
    if {[lindex $result 0] == "TRUE"} {
	one_line_message .message "$testname Test" \
		"YES! This is a $testname decomposition."
	return
    }

    if {[lindex $result 0] == "FALSE"} {
	one_line_message .message "$testname Test" \
		"NO! This is NOT a $testname decomposition."
	return
    }

    if {[lindex $result 0] == -1} {
	one_line_message .message "$testname Test" \
		"Error: [lrange $result 2 end]"
    } else {
	one_line_message .message "$testname Test" "Unknown error: $result"
    }

}

proc getDecomp_generateNormalForm { normalform } {

    global __GETDCMP__parentRelation
    global __GETDCMP__activeChild
    global __GETDCMP__nChildren
    global __GETDCMP__childFrames

    set relation $__GETDCMP__parentRelation
    set rname [relation_getName $relation]

    set result [db_generateNormalForm $relation $normalform]
    if {[lindex $result 0] == -1} {
	one_line_message .message "$normalform Test" \
		"Error: [lrange $result 2 end]"
	return
    }

    set i 0
    foreach attrset $result {
	if {$i > [expr $__GETDCMP__nChildren - 1]} {
	    getDecomp_makeChildFrame
	}
	getDecomp_setChildName $i ${rname}[expr $i + 1]
	getDecomp_setAttributes $i $attrset
	incr i
    }

}

