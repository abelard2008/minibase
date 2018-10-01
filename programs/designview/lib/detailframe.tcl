
proc detail_init {} {
    
    global __DETAIL__attrMenu
    global __DETAIL__sessionMenu

    #
    # Create a pop-up menu for the attribute list
    #
    set __DETAIL__attrMenu .detail_attrmenu
    catch {destroy $__DETAIL__attrMenu}
    menu $__DETAIL__attrMenu -tearoff 0
    $__DETAIL__attrMenu add command -label "Attribute Closure" \
	    -command [concat detail_showAttrClosure { $__DETAIL__active }]
    $__DETAIL__attrMenu add command -label "Key?" \
	    -command [concat detail_isKey { $__DETAIL__active }]
    $__DETAIL__attrMenu add command -label "Prime?" \
	    -command [concat detail_isPrime { $__DETAIL__active }]

    #
    # Create a pop-up menu for the session list
    #
    set __DETAIL__sessionMenu .detail_sessionMenu
    catch {destroy $__DETAIL__sessionMenu}
    menu $__DETAIL__sessionMenu -tearoff 0
    $__DETAIL__sessionMenu add command -label "Display" \
	    -command [list busy [concat detail_viewSession \
	    { $__DETAIL__active $__DETAIL__popupX $__DETAIL__popupY }]]
    $__DETAIL__sessionMenu add command -label "Rename" \
	    -command [list busy [concat detail_renameSession \
	    { $__DETAIL__active }]]
    $__DETAIL__sessionMenu add command -label "Delete" \
	    -command [list busy [concat detail_deleteSession \
	    { $__DETAIL__active }]]
    $__DETAIL__sessionMenu add command -label "Set as Default" \
	    -command [list busy [concat detail_setDefaultSession \
	    { $__DETAIL__active }]]
    $__DETAIL__sessionMenu add command -label "Start New Session" \
	    -command [list busy [concat detail_newSession \
	    { $__DETAIL__active $__DETAIL__popupX $__DETAIL__popupY }]]

}

proc detail_makeDetailFrame { parentWidget relation {session {}} } {

    global __DETAIL__frame
    global __DETAIL__attrs
    global __DETAIL__fdproj
    global __DETAIL__sessionList
    global __DETAIL__columns
    global __DETAIL__columnFrame
    global __DETAIL__visible
    global __DETAIL__col
    global __DETAIL__activeRelation
    
    set oid [newOID Detail]
    set f [frame $parentWidget.detail]
    set __DETAIL__frame($oid) $f

    set columns [frame $f.columns]
    set __DETAIL__columnFrame($oid) $columns
    set buttons [frame $f.buttons]

    #
    # Create frame with three columns: attrs, FDs, sessions
    #
    set c1 [frame $columns.c1 -relief raised -bd 2]
    set c2 [frame $columns.c2 -relief raised -bd 2]
    set c3 [frame $columns.c3 -relief raised -bd 2]
    set __DETAIL__columns($oid) [list $c1 $c2 $c3]

    set b1 [frame $columns.b1 -bg black -width 4 \
	    -cursor sb_h_double_arrow]
    set b2 [frame $columns.b2 -bg black -width 4 \
	    -cursor sb_h_double_arrow]

    set h1 [label $c1.head -text " Attributes "]
    set h2 [label $c2.head -text " FD Projection "]
    set h3 [label $c3.head -text " Decomposition Sessions "]

    set __DETAIL__attrs($oid) [ScrolledListbox2 $c1.lframe 1 8 \
	    -setgrid false -highlightthickness 0 \
	    -selectmode extended]
    set __DETAIL__fdproj($oid) [ScrolledListbox2 $c2.lframe 1 8 \
	    -setgrid false -highlightthickness 0]
    set __DETAIL__sessionList($oid) [ScrolledListbox2 $c3.lframe 1 8 \
	    -setgrid false -highlightthickness 0]

    pack $h1 $h2 $h3 -anchor n -fill x -expand false
    pack $c1.lframe $c2.lframe $c3.lframe -anchor nw -side right \
	    -fill both -expand true

    pack $c1 -side left -anchor nw -pady 0 -padx 0 -expand false -fill y
    pack $b1 -side left -anchor nw -pady 0 -padx 0 -fill y
    pack $c2 -side left -anchor nw -pady 0 -padx 0 -expand false -fill y
    pack $b2 -side left -anchor nw -pady 0 -padx 0 -fill y
    pack $c3 -side left -expand true -fill both -anchor nw -pady 0 -padx 0

    pack $columns -side bottom -anchor nw -fill both -expand true

    bind $b1 <Button-1> \
	    [concat set __DETAIL__col $c1 {;} set __DETAIL__x1 %X]
    bind $b1 <ButtonRelease-1> \
	    [concat detail_adjustWidth { $__DETAIL__col } \
	    { [expr %X - $__DETAIL__x1] ; } ]

    bind $b2 <Button-1> \
	    [concat set __DETAIL__col $c2 {;} set __DETAIL__x1 %X]
    bind $b2 <ButtonRelease-1> \
	    [concat detail_adjustWidth { $__DETAIL__col } \
	    { [expr %X - $__DETAIL__x1] ; } ]

    bind $__DETAIL__sessionList($oid) <Double-1> \
	    [list busy [list detail_viewSession $oid %X %Y]]

    bind $__DETAIL__attrs($oid) <Button-2> \
	    [list detail_showAttrMenu $oid %x %y %X %Y]
    bind $__DETAIL__sessionList($oid) <Button-2> \
	    [list detail_showSessionMenu $oid %x %y %X %Y]

    bind $f <Destroy> [list detail_delete $oid]

    set __DETAIL__visible($oid) 1
    set __DETAIL__activeRelation($oid) {}
    
    updateDetailWindow $oid $relation

    detail_propagateOff $oid
    detail_adjustWidth [lindex $__DETAIL__columns($oid) 0] 150
    detail_adjustWidth [lindex $__DETAIL__columns($oid) 1] 150
    
    return $oid
    
}

proc updateDetailWindow { oid relation } {
    
    global __DETAIL__attrs
    global __DETAIL__fdproj
    global __DETAIL__sessionList
    global __DETAIL__columns
    global __DETAIL__activeRelation
    global __DETAIL__sessionOIDs

    if {$relation == $__DETAIL__activeRelation($oid) || $relation == {}} {
	return
    }

    set __DETAIL__activeRelation($oid) $relation
    
    set lists [list $__DETAIL__attrs($oid) $__DETAIL__fdproj($oid) \
	    $__DETAIL__sessionList($oid)]

    foreach l $lists {
	$l delete 0 end
	$l insert end "Computing..."
    }
    update

    set attrSet [relation_getAttrSet $relation]

    set attrs {}
    foreach a [attrset_getAttributes $attrSet] {
	lappend attrs "$a  \[[db_getAttrType $a]\]"
    }

    set fds [db_getTextOfAllFDs $attrSet]

    foreach l $lists { $l delete 0 end }

    eval $__DETAIL__attrs($oid) insert end $attrs
    eval $__DETAIL__fdproj($oid) insert end $fds
    detail_updateSessionNames $oid

}

proc clearDetailWindow { oid } {

    global __DETAIL__attrs
    global __DETAIL__fdproj
    global __DETAIL__sessionList
    global __DETAIL__columns

    $__DETAIL__attrs($oid) delete 0 end
    $__DETAIL__fdproj($oid) delete 0 end
    $__DETAIL__sessionList($oid) delete 0 end

}

proc detail_updateSessionNames { oid } {
    
    global __DETAIL__sessionList
    global __DETAIL__sessionOIDs

    $__DETAIL__sessionList($oid) delete 0 end
    
    set relation [detail_getActiveRelation $oid]
    set __DETAIL__sessionOIDs($oid) [db_getSessions $relation]

    foreach s $__DETAIL__sessionOIDs($oid) {
	$__DETAIL__sessionList($oid) insert end [session_getName $s]
    }

}

proc detail_getActiveRelation { oid } {
    global __DETAIL__activeRelation
    return $__DETAIL__activeRelation($oid)
}

proc detail_getFrame { oid } {
    global __DETAIL__frame
    return $__DETAIL__frame($oid)
}

proc detail_adjustWidth { f delta } {
    set old [$f cget -width]
    set new [expr $delta + $old]
    debug "adjustWidth $f : $old -> $new"
    $f config -width $new
}

proc detail_adjustHeight { f delta } {
    set old [$f cget -height]
    set new [expr $delta + $old]
    debug "adjustHeight $f : $old -> $new"
    $f config -height $new
}

proc detail_propagateOn { oid } {
    global __DETAIL__columns
    update
    foreach c $__DETAIL__columns($oid) {
	pack propagate $c true
    }
}

proc detail_propagateOff { oid } {
    global __DETAIL__columns
    update
    foreach c $__DETAIL__columns($oid) {
	pack propagate $c false
    }
}

proc detail_viewSession { oid x y } {
    set session [detail_getSelectedSession $oid]
    if {$session == {}} { return }
    sdisplay_makeSDisplay $session $x $y
}

proc detail_newSession { oid x y } {
    set relation [detail_getActiveRelation $oid]
    if {$relation == {}} { return }
    set session [session_makeSession $relation]
    session_setName $session "New Session"
    sdisplay_makeSDisplay $session $x $y
}

proc detail_getSelectedSession { oid } {

    global __DETAIL__sessionList
    global __DETAIL__sessionOIDs

    set index [$__DETAIL__sessionList($oid) curselection]
    if {$index == {}} {
	one_line_message .error "Error" "You must first select a session."
	return {}
    }

    return [lindex $__DETAIL__sessionOIDs($oid) $index]

}

proc detail_getSelectedAttributes { oid } {

    global __DETAIL__attrs

    set indeces [$__DETAIL__attrs($oid) curselection]
    if {$indeces == {}} {
	one_line_message .error "Error" \
		"You must first select a set of attributes."
	return {}
    }

    set result {}
    foreach i $indeces {
	lappend result [lindex [$__DETAIL__attrs($oid) get $i] 0]
    }

    return $result

}

proc detail_showAttrMenu { oid x y rootx rooty } {

    global __DETAIL__attrMenu
    global __DETAIL__attrs
    global __DETAIL__popupX
    global __DETAIL__popupY
    global __DETAIL__active

    set __DETAIL__popupX $rootx
    set __DETAIL__popupY $rooty
    set __DETAIL__active $oid 
    
    set l $__DETAIL__attrs($oid)
    set index [$l nearest $y]
    if ![$l selection includes $index] { return }

    tk_popup $__DETAIL__attrMenu $rootx $rooty

}

proc detail_showSessionMenu { oid x y rootx rooty } {

    global __DETAIL__sessionMenu
    global __DETAIL__sessionList
    global __DETAIL__popupX
    global __DETAIL__popupY
    global __DETAIL__active

    set __DETAIL__popupX $rootx
    set __DETAIL__popupY $rooty
    set __DETAIL__active $oid

    set l $__DETAIL__sessionList($oid)
    set index [$l nearest $y]
    if ![$l selection includes $index] { return }

    tk_popup $__DETAIL__sessionMenu $rootx $rooty

}

proc detail_showAttrClosure { oid } {

    set attrs [detail_getSelectedAttributes $oid]
    if {$attrs == {}} { return }

    set result [db_attrClosure $attrs [detail_getActiveRelation $oid]]
    
    one_line_message .closure "Attribute Closure" \
            [list Closure of [join $attrs ", "] = [join $result ", "]]

}

proc detail_isKey { oid } {

    global __DETAIL__dbVisible

    set relation [detail_getActiveRelation $oid]
    set attrs [detail_getSelectedAttributes $oid]
    if {$attrs == {}} { return }

    set rname [relation_getName $relation]
    set result [db_isKey $attrs $relation]

    if {$result == "TRUE"} {
	one_line_message .iskey "Key?" \
		[concat YES. \{[join $attrs ", "]\} is a key for $rname]
    } else {
	one_line_message .iskey "Key?" \
		[concat NO. \{[join $attrs ", "]\} is NOT a key for $rname]
    }

}

proc detail_isPrime { oid } {

    set relation [detail_getActiveRelation $oid]
    set attrs [detail_getSelectedAttributes $oid]
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

proc detail_renameSession { oid } {
    
    set session [detail_getSelectedSession $oid]
    if {$session == {}} { return }

    set newname [input_box .inputbox "Rename a Session" \
	    "Enter a new name for the session:" [session_getName $session]]
    if {$newname == {}} { return {} }

    session_setName $session $newname
    detail_updateSessionNames $oid

}

proc detail_deleteSession { oid } {

    set session [detail_getSelectedSession $oid]
    if {$session == {}} { return }

    set sname [session_getName $session]

    set confirm [dialog .confirm "Delete Session" \
            "Are you sure you want to delete session \"$sname\"?" \
	    "" 0 {Cancel} {Delete}]
    if { $confirm == 0 } { return }

    db_deleteSession [detail_getActiveRelation $oid] $session
    detail_updateSessionNames $oid

}

proc detail_setDefaultSession { oid } {
    
    set session [detail_getSelectedSession $oid]
    if {$session == {}} { return }

    set r [detail_getActiveRelation $oid]
    if {$r == {}} { return }

    db_setDefaultSession $r $session
    detail_updateSessionNames $oid
    
}

proc detail_delete { oid } {
    
    set f [detail_getFrame $oid]
    catch {destroy $f}
    
    #
    # Clear all state variables
    #
    foreach array [info globals __DETAIL__*] {
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

