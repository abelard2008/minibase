
proc updateMainWindow {} {

    global __MAIN__rframes
    global __MAIN__canvas

    set count 0
    set currentRow 1
    set gap 8
    set nextX $gap
    set nextY $gap
    set __MAIN__rframes {}
    set c $__MAIN__canvas

    foreach r [db_getSourceRelations] {

	debug "Updating main window: $r"

	set oid [rframe_makeFrame $r $c]
	lappend __MAIN__rframes $oid
	if {$count == 0} {
	    main_selectFrame $oid
	} else {
	    rframe_highlightOff $oid
	}

	bind $oid <Button-1> [list + main_selectFrame $oid]
	bind $oid <Double-1> \
		[concat + main_newSession {[main_getActiveRelation]} %X %Y]

	set i [$__MAIN__canvas create window $nextX $nextY -anchor nw \
		-window [rframe_getFrameName $oid] -tag row$currentRow ]

	set result [scan [$c bbox row$currentRow] "%d %d %d %d" \
		left top right bot]
	set nextX [expr $right + $gap]
	if {$nextX > 500} {
	    set nextX $gap
	    set nextY [expr $bot + $gap]
	    incr currentRow
	}
	incr count

    }

    scan [$c bbox all] "%d %d %d %d" x1 y1 x2 y2
    $c move all [expr $gap - $x1] [expr $gap - $y1]

    set width [max [expr $x2 + (2 * $gap) - $x1] 200]
    set height [expr $y2 + (2 * $gap) - $y1]
    $c config -scrollregion "0 0 $width $height" \
	    -width $width -height $height

    if [main_detailIsVisible] {
	updateDetailWindow [main_getDetail] [main_getActiveRelation]
    }

}

proc clearMainWindow {} {

    global __MAIN__activeFrame
    global __MAIN__rframes
    global __MAIN__canvas

    if [main_detailIsVisible] {
	clearDetailWindow [main_getDetail]
    }

    if ![info exists __MAIN__rframes] { return }
    foreach f $__MAIN__rframes {
	rframe_delete $f
    }

    if [info exists __MAIN__activeFrame] { unset __MAIN__activeFrame }
    if [info exists __MAIN__rframes] { unset __MAIN__rframes }

    $__MAIN__canvas delete all

}

proc updateTitleBar { {filename {}} } {
    if {$filename == {}} {
	wm title . designview
    } else {
	wm title . [list dbnorm - [file tail $filename]]
    }
}

proc main_getActiveRelation {} {
    global __MAIN__activeFrame
    if ![info exists __MAIN__activeFrame] { return {} }
    return [rframe_getRelation $__MAIN__activeFrame]
}

proc junk {} {

    global __MAIN__relationName
    global __MAIN__sessionList
    global __MAIN__attributeList
    global __MAIN__typeList
    global __MAIN__sessionOIDs
    global __MAIN__fdList
    global __MAIN__fdHandles

    set oldCursor [. cget -cursor]
    . config -cursor watch
    update idletasks

    set r [db_getActiveRelation]
    if {$r == {}} {
	set r [lindex [db_getSourceRelations] 0]
	if {$r == {}} {
	    . config -cursor $oldCursor
	    return
	}
	db_setActiveRelation $r
    }

    set __MAIN__relationName [relation_getName $r]

    #
    # Display the attributes of the active relation
    #
    set key [relation_getKey $r]
    set other [set_difference [relation_getAttributes $r] $key]
    $__MAIN__attributeList delete 0 end
    $__MAIN__typeList delete 0 end
    foreach attr $key {
	$__MAIN__typeList insert end [db_getAttrType $attr]
	$__MAIN__attributeList insert end [append attr " *"]
    }
    foreach attr $other {
	$__MAIN__typeList insert end [db_getAttrType $attr]
	$__MAIN__attributeList insert end $attr
    }

    #
    # Display the decompositions of the active relation
    #
    $__MAIN__sessionList delete 0 end
    set __MAIN__sessionOIDs [db_getSessions $r]
    set snames {}
    foreach session $__MAIN__sessionOIDs {
	lappend snames [session_getName $session]
    }
    eval $__MAIN__sessionList insert end $snames

    #
    # Display the FD projection for the active relation
    #
    set attrSet [relation_getAttrSet $r]
    $__MAIN__fdList delete 0 end
    set __MAIN__fdHandles [db_getFDProjHandles $attrSet]
    set list {}
    foreach handle $__MAIN__fdHandles {
	$__MAIN__fdList insert end [db_getFDprojText $attrSet $handle]
    }

    . config -cursor $oldCursor

}

proc main_getSelectedSession {} {

    global __MAIN__sessionList
    global __MAIN__sessionOIDs

    set index [$__MAIN__sessionList curselection]
    if {$index == {}} {
	one_line_message .error "Error" "You must first select a session."
	return {}
    }

    return [lindex $__MAIN__sessionOIDs $index]

}

proc main_viewSession { {session {}} } {

    if {$session == {}} { set session [main_getSelectedSession] }
    if {$session == {}} { return }

    sdisplay_makeSDisplay $session \
	    [expr [winfo rootx .] + 25] \
	    [expr [winfo rooty .] + 25]
 
}

proc main_setDefaultSession {} {

    set session [main_getSelectedSession]
    if {$session == {}} { return }

    set r [main_getActiveRelation]
    if {$r == {}} { return }

    db_setDefaultSession $r $session
    updateMainWindow
}

proc main_newSession { {relation {}} {x {}} {y {}} } {

    if {$relation == {}} {
	set relation [main_getActiveRelation]
    }
    if {$relation == {}} {
	one_line_message .error "Error" "You must first select a relation."
	return
    }

    set session [session_makeSession $relation]
    session_setName $session "New Session"

    if {$x == {}} { set x [expr [winfo rootx .] + 25] }
    if {$y == {}} { set y [expr [winfo rooty .] + 25] }

    sdisplay_makeSDisplay $session $x $y

}

proc main_viewDeepDecomp {} {

    set r [db_getActiveRelation]
    if {$r == {}} {
	one_line_message .error "Error" "You must first open a database."
	return
    }

    set session [db_makeDeepSession]

    sdisplay_makeSDisplay $session \
	    [expr [winfo rootx .] + 25] \
	    [expr [winfo rooty .] + 25] \
	    0

}

proc main_renameSession {} {

    set session [main_getSelectedSession]
    if {$session == {}} { return }

    set newname [input_box .inputbox "Rename a Session" \
	    "Enter a new name for the session:"]
    if {$newname == {}} { return {} }

    session_setName $session $newname
    updateMainWindow

}

proc main_deleteSession {} {

    set session [main_getSelectedSession]
    if {$session == {}} { return }

    set sname [session_getName $session]

    set confirm [dialog .confirm "Delete Session" \
            "Are you sure you want to delete session \"$sname\"?" \
	    "" 0 {Cancel} {Delete}]
    if { $confirm == 0 } { return }

    db_deleteSession [main_getActiveRelation] $session
    updateMainWindow

}

proc main_deleteFD { } {

    global __MAIN__fdList
    global __MAIN__fdHandles

    set r [main_getActiveRelation]
    if {$r == {}} {
	one_line_message .error "Error" "You must first select a relation."
	return
    }

    set index [$__MAIN__fdList curselection]
    if {$index == {}} {
	one_line_message .error "Error" "You must first select a dependency."
	return
    }

    set handle [lindex $__MAIN__fdHandles $index]
    set fdtext [$__MAIN__fdList get $index]

    set confirm [dialog .confirm "Confirm FD Deletion" \
            "Delete the FD \"$fdtext\"?" "" \
	    0 {Cancel} {Delete}]
    if {$confirm == 0} { return }

    db_deleteFD $handle
    $__MAIN__fdList delete [$__MAIN__fdList curselection]
    set __MAIN__fdHandles [lreplace $__MAIN__fdHandles $index $index]
}

#
# addFD and deleteFD should not be used! They were written when the 
# main window displayed all FDs, not a projection.
#
proc main_addFD { } {

    global __MAIN__fdList
    global __MAIN__fdHandles

    set r [main_getActiveRelation]
    if {$r == {}} {
	one_line_message .error "Error" "You must first select a relation."
	return
    }

    set fd [fdedit_editFD {} {} \
	    [expr [winfo rootx .] + 25] [expr [winfo rooty .] + 25]]
    if {$fd == {}} { return }

    set handle [db_addFD [lindex $fd 0] [lindex $fd 1]]
    if {$handle < 0} { return }

    lappend __MAIN__fdHandles $handle
    $__MAIN__fdList insert end [db_getFDText $handle]

}

proc main_editFD { } {
    dialog .file_error "Error" "Not implemented yet." "" 0 {OK}
    return 1

    global __MAIN__source_relation
    global __MAIN__fdList
    global __MAIN__fdHandles

    set r $__MAIN__source_relation
    if {$r == {}} { return }

    set line_no [$__MAIN__fdList curselection]
    if {$line_no == {}} { return }

    set old_handle [lindex $__MAIN__fdHandles $line_no]
    set fd_text [$__MAIN__fdList get $line_no]
    set lhs [split [lindex $fd_text 0] ,]
    set rhs [split [lindex $fd_text 2] ,]

    set fd [fd_edit_window $r [dbn_getAttributes $r] $lhs $rhs]
    if {$fd == {}} { return }

    # Update FDset
    dbn_deleteFD $r $old_handle
    set new_handle [dbn_addFD $r [lindex $fd 0] [lindex $fd 1]]

    # Update display
    $__MAIN__fdList insert $line_no [dbn_getFDText $r $new_handle]
    $__MAIN__fdList delete [expr $line_no + 1]

    # Update list of fd handles
    set __MAIN__fdHandles \
	    [lreplace $__MAIN__fdHandles $line_no $line_no $new_handle]

}

proc main_attrClosure {} {

    set relation [main_getActiveRelation]
    if {$relation == {}} { return }

    set attrs [main_getSelectedAttributes]
    if {$attrs == {}} { return }

    set result [db_attrClosure $attrs $relation]

    one_line_message .closure "Attribute Closure" \
            [list Closure of [join $attrs ", "] = [join $result ", "]]

}

proc main_isKey { } {

    set relation [main_getActiveRelation]
    if {$relation == {}} { return }

    set attrs [main_getSelectedAttributes]
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

proc main_isPrime {} {

    set relation [main_getActiveRelation]
    if {$relation == {}} { return }

    set attrs [main_getSelectedAttributes]
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

proc main_getSelectedAttributes {} {
    global __MAIN__attributeList
    if {[main_getActiveRelation] == {}} { return }
    set indeces [$__MAIN__attributeList curselection]
    if {$indeces == {}} {
	one_line_message .error "Error" \
		"You must first select a set of attributes."
    }
    set result {}
    foreach i $indeces {
	lappend result \
		[string trimright [$__MAIN__attributeList get $i] " *"]
    }
    return $result
}

proc main_selectNearest { list y } {
    $list selection set [$list nearest $y] [$list nearest $y]
}

proc main_selectFrame { rframe } {
    set f [main_getActiveFrame]
    if {$f != {}} {
	rframe_highlightOff $f
    }
    rframe_highlightOn $rframe
    main_setActiveFrame $rframe
    if [main_detailIsVisible] {
	updateDetailWindow [main_getDetail] [rframe_getRelation $rframe]
    }
}

proc main_setActiveFrame { f } {
    global __MAIN__activeFrame
    set __MAIN__activeFrame $f
}

proc main_getActiveFrame {} {
    global __MAIN__activeFrame
    if ![info exists __MAIN__activeFrame] {
	return {}
    }
    return $__MAIN__activeFrame
}

proc main_activateButtons {{activate 1}} {

    global __MAIN__buttons
    global __MAIN__relationMenu
    global __MAIN__fileMenu
    global __MAIN__inactiveFileCommands

    if $activate {
	set state normal
    } else {
	set state disabled
    }

    foreach b [concat $__MAIN__buttons $__MAIN__relationMenu] {
	$b config -state $state
    }

    foreach c [concat $__MAIN__inactiveFileCommands] {
	$__MAIN__fileMenu.menu entryconfig $c -state $state
    }

}

proc main_updateRelationNames {} {
    #
    # I'm sure there's an efficient way to do this, but I'm just going to 
    # destroy everything then re-create it to avoid moving objects around
    # on the canvas when the length of a relation name changes
    #
    clearMainWindow
    updateMainWindow
}

proc main_viewDetails {} {

    global __MAIN__canvas
    global __MAIN__detailButton
    global __MAIN__buttonFrame
    global __MAIN__detailFrame
    global __MAIN__canvasFrame
    
    debug "Viewing relation details in main window"
    
    set relation [main_getActiveRelation]
    if {$relation == {}} {
	return
    }
    
    set w .
    set c $__MAIN__canvas
    
    update
    scan [split [winfo geometry $w] "+x"] "%d %d %d %d" \
	    oldWidth oldHeight wx wy
    
    if [main_detailIsVisible] {

	$__MAIN__detailButton config -text "View Details"
	destroy .border1
	detail_delete [main_getDetail]
	main_setDetailVisible 0
	update
	set newWidth [min [winfo reqwidth $w] $oldWidth]
	set newHeight [min [winfo reqheight $w] $oldHeight]
	
    } else {
	
	$__MAIN__detailButton config -text "Hide Details"
	set d [detail_makeDetailFrame $w $relation]
	main_setDetail $d
	set __MAIN__detailFrame [detail_getFrame $d]
	main_setDetailVisible 1
    
	frame .border1 -bg black -height 4 -cursor sb_v_double_arrow
	
	bind .border1 <Button-1> \
		[concat set __MAIN__y1 %Y]
	bind .border1 <ButtonRelease-1> \
		[concat resizeWidget $c height \
		{ [expr %Y - $__MAIN__y1] ; } ]

	pack $__MAIN__detailFrame -side bottom -fill both -expand true
	pack .border1 -side bottom -fill x

	update
	set newWidth [max $oldWidth [winfo reqwidth $__MAIN__detailFrame]]
	set newHeight [expr $oldHeight + \
		[winfo reqheight $__MAIN__detailFrame] + 4]

    }

    wm geometry $w ${newWidth}x${newHeight}+${wx}+${wy}

}

proc main_getDetail {} {
    global __MAIN__detail
    if ![info exists __MAIN__detail] {
	return {}
    }
    return $__MAIN__detail
}

proc main_setDetail { d } {
    global __MAIN__detail
    set __MAIN__detail $d
}

proc main_setDetailVisible { visible } {
    global __MAIN__detailVisible
    set __MAIN__detailVisible $visible
}

proc main_detailIsVisible {} {
    global __MAIN__detailVisible
    if ![info exists __MAIN__detailVisible] {
	return {}
    }
    return $__MAIN__detailVisible
}

