
proc rframe_init {} {

    global __RFRAME__attrMenu
    catch {destroy $__RFRAME__attrMenu}

    set __RFRAME__attrMenu [menu .rframe_attrMenu -tearoff 0]
    $__RFRAME__attrMenu add command -label "Show Types" \
	    -command [list rframe_showAttrTypes]
    $__RFRAME__attrMenu add command -label "Attribute Closure" \
	    -command [list rframe_showAttrClosure]
    $__RFRAME__attrMenu add command -label "Key?" \
	    -command [list rframe_isKey]
    $__RFRAME__attrMenu add command -label "Prime?" \
	    -command [list rframe_isPrime]

}

proc rframe_makeFrame { relation parentWidget } {

    debug "Making relation frame: relation=$relation, parent=$parentWidget"

    set frameid [newOID RFrame]
    set rname [relation_getName $relation]
    set keyAttrs [relation_getKey $relation]
    set otherAttrs [set_difference [relation_getAttributes $relation] \
	    $keyAttrs]

    set borderwidth 2
    set f $parentWidget._$frameid
    frame $f -relief ridge -bd $borderwidth
    bindtags $f [list $frameid Frame $f all]

    global __RFRAME__relation
    global __RFRAME__framename
    set __RFRAME__relation($frameid) $relation
    set __RFRAME__framename($frameid) $f

    # Pack the title label
    label $f.rframe_title -text $rname -bg blue -fg white \
	    -justify center -pady 0 -padx 5
    bindtags $f.rframe_title [list $frameid Label $f.rframe_title all]
    pack $f.rframe_title -fill x

    # Create a frame containing the attribute listbox
    set attr_box [_rframe_makeAttrList $f $keyAttrs $otherAttrs $frameid]

    # Have this frame set its own height and width. If you don't do this,
    # then other procs may ask for its height and width before the display
    # has actually been updated.
    set frame_height [expr [winfo reqheight $f.rframe_title] + \
	    [winfo reqheight $attr_box] + (2 * $borderwidth)]
    set frame_width [expr [max [winfo reqwidth $f.rframe_title] \
	    [winfo reqwidth $attr_box]] + (2 * $borderwidth)]
    $f config -height $frame_height -width $frame_width

    return $frameid

}

proc _rframe_makeAttrList { f keyAttrs otherAttrs frameid } {

    # Check if length of attr list is > 8. If so, the box will have
    # eight lines and a scroll bar.
    set n_attrs [expr [llength $keyAttrs] + [llength $otherAttrs]]
    if {$n_attrs <= 8} {
	set n_lines $n_attrs
	set scroll 0
    } else {
	set n_lines 8
	set scroll 1
    }

    # Create the listbox to display attribute names
    set f2 [frame $f.attr_frame -bd 0]
    bindtags $f2 [list $frameid Frame $f2 all]
    listbox $f2.list -setgrid false -height $n_lines -width 0 -relief flat \
	    -selectmode extended -background white -highlightthickness 0 \
	    -selectborderwidth 0 -selectbackground white
    bindtags $f2.list [list $frameid Listbox $f2.list all]
    foreach k $keyAttrs {
	$f2.list insert end [join [list $k *] ""]
    }
    eval $f2.list insert end $otherAttrs
    pack $f2.list -side left -fill both -expand true -padx 0 -pady 0
    set frame_height [winfo reqheight $f2.list]
    set frame_width [winfo reqwidth $f2.list]

#    bind $f2.list <Button-2> \
#	    [list rframe_showAttrMenu $frameid %W %x %y %X %Y]

    # Create a scrollbar if necessary
    if {$scroll == 1} {
	$f2.list config -yscrollcommand [list $f2.sy set]
	scrollbar $f2.sy -orient vertical -width 5 \
		-command [list $f2.list yview]
	bindtags $f2.sy [list $frameid Scrollbar $f2.sy all]
	pack $f2.sy -side right -fill y
	set frame_height [max [winfo reqheight $f2.list] \
		[winfo reqheight $f2.sy]]
	incr frame_width [winfo reqwidth $f2.sy]
    }

    pack $f2 -side left -fill both -expand true -padx 0 -pady 0

    # Have this frame set its own height and width. If you don't do this,
    # then other procs may ask for its height and width before the display
    # has actually been updated.
    $f2 config -height $frame_height -width $frame_width

    return $f2

}

proc rframe_getCollapsedHeight { oid } {
    global __RFRAME__framename
    if ![info exists __RFRAME__framename($oid)] {
	puts "Error (rframe_getCollapsedHeight): invalid rframe \"$oid\""
	return {}
    }
    set f $__RFRAME__framename($oid)
    return [expr [winfo reqheight $f.rframe_title] + 4]
}

proc rframe_getExpandedHeight { oid } {
    global __RFRAME__framename
    if ![info exists __RFRAME__framename($oid)] {
	puts "Error (rframe_getExpandedHeight): invalid rframe \"$oid\""
	return {}
    }
    set f $__RFRAME__framename($oid)
    return [winfo reqheight $f]
}

proc rframe_getFrameName { oid } {
    global __RFRAME__framename
    if ![info exists __RFRAME__framename($oid)] {
	puts "Error (rframe_getFrameName): invalid rframe \"$oid\""
	return {}
    }
    return $__RFRAME__framename($oid)
}

proc rframe_getRelation { oid } {
    global __RFRAME__relation
    if ![info exists __RFRAME__relation($oid)] {
	puts "Error (rframe_getFrameName): invalid rframe \"$oid\""
	return {}
    }
    return $__RFRAME__relation($oid)
}

proc rframe_setRelationName { oid newName } {
    set f [rframe_getFrameName $oid]
    $f.rframe_title config -text $newName
}

proc rframe_highlightOn { oid } {
    set f [rframe_getFrameName $oid]
    $f config -relief flat -background black
    $f.rframe_title config -background blue -foreground white
}

proc rframe_highlightOff { oid } {
    set f [rframe_getFrameName $oid]
    $f config -relief ridge -background lightblue
    $f.rframe_title config -background gray80 -foreground gray50
}

proc rframe_getSelectedAttrs { oid } {
    set f [rframe_getFrameName $oid]
    set attrs {}
    foreach a [get_selected_lines $f.attr_frame.list] {
	lappend attrs [string trimright $a " *"]
    }
    return $attrs
}

proc rframe_delete { oid } {

    debug "Deleting RFrame $oid"

    catch {destroy [rframe_getFrameName $oid]}

    # Clear all state variables
    foreach array [info globals __RFRAME__*] {
	global $array
	if [array exists $array] {
	    foreach n [array names $array $oid] {
		debug "    un-setting ${array}($n)"
		eval unset ${array}($n)
	    }
	    foreach n [array names $array $oid,*] {
		debug "    un-setting ${array}($n)"
		eval unset ${array}($n)
	    }
	}
    }

}

proc rframe_showAttrMenu { oid listbox x y rootx rooty } {

    global __RFRAME__attrMenu
    global __RFRAME__activeOID
    global __RFRAME__popupX
    global __RFRAME__popupY

    set __RFRAME__activeOID $oid
    set __RFRAME__popupX $rootx
    set __RFRAME__popupY $rooty

    set index [$listbox nearest $y]
    if ![$listbox selection includes $index] {
	return
    }

    tk_popup $__RFRAME__attrMenu $rootx $rooty

}

proc rframe_showAttrTypes {} {

    global __RFRAME__activeOID
    global __RFRAME__popupX
    global __RFRAME__popupY

    set oid $__RFRAME__activeOID
    set attrs [rframe_getSelectedAttrs $oid]

    set types {}
    foreach a $attrs {
	lappend types [db_getAttrType $a]
    }

    set w .attrTypeWin
    if [winfo exists $w] {
	raise_toplevel $w
	$w.left.list delete 0 end
	$w.right.list delete 0 end
    } else {
	toplevel $w
	wm title $w "Attribute Types"
	wm geometry $w +$__RFRAME__popupX+$__RFRAME__popupY
	frame $w.left
	frame $w.right
	label $w.left.label -text Attribute
	label $w.right.label -text Type
	listbox $w.left.list -setgrid true
	listbox $w.right.list -setgrid true
	pack $w.left.label $w.left.list
	pack $w.right.label $w.right.list
	pack $w.left -side left -anchor nw
	pack $w.right -side right -anchor ne
    }

    eval $w.left.list insert end $attrs
    eval $w.right.list insert end $types

}

proc rframe_showAttrClosure {} {

    global __RFRAME__activeOID

    set oid $__RFRAME__activeOID
    set relation [rframe_getRelation $oid]
    set attrs [rframe_getSelectedAttrs $oid]
    if {$attrs == {}} { return }

    set result [db_attrClosure $attrs $relation]

    one_line_message .closure "Attribute Closure" \
            [list Closure of [join $attrs ", "] = [join $result ", "]]

}

proc rframe_isKey {} {

    global __RFRAME__activeOID

    set oid $__RFRAME__activeOID
    set relation [rframe_getRelation $oid]
    set attrs [rframe_getSelectedAttrs $oid]
    if {$attrs == {}} { return }

    set rname [relation_getName $relation]
    set result [db_isKey $attrs $relation]

    if {$result == "TRUE"} {
	one_line_message .iskey "Key?" [list YES. \
		[join $attrs ", "] is a key for $rname.]
    } else {
	one_line_message .iskey "Key?" [list NO. \
		[join $attrs ", "] is NOT a key for $rname.]
    }

}

proc rframe_isPrime {} {

    global __RFRAME__activeOID

    set oid $__RFRAME__activeOID
    set relation [rframe_getRelation $oid]
    set attrs [rframe_getSelectedAttrs $oid]
    if {$attrs == {}} { return }

    set result [db_isPrime $attrs $relation]

    if {$result == "TRUE"} {
	one_line_message .isprime "Prime?" [list YES. \
		[join $attrs ", "] is prime.]
    } elseif {$result == "FALSE"} {
	one_line_message .isprime "Prime?" [list NO. \
		[join $attrs ", "] is NOT prime.]
    } else {
	one_line_message .isprime "Prime?" "Error: [lrange $result 2 end]"
    }

}

