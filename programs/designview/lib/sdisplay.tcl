
set __SDISPLAY__FDProjWin .fdproj
set __SDISPLAY__FDProjList {}
set __SDISPLAY__FDProjVisible 0

proc sdisplay_makeSDisplay { session1 x y {allowSave 1} } {

    global __SDISPLAY__dirty

    if {$session1 == {}} { return }

    debug "Displaying session $session1 at x=$x, y=$y"

    set session2 [session_makeCopy $session1]
    set w .[string tolower $session2]

    set displayOID [newOID SDisplay]
    _sdisplay_setParent $displayOID $session1
    _sdisplay_setSession $displayOID $session2
    _sdisplay_setAllowSave $displayOID $allowSave
    _sdisplay_setDetailVisible $displayOID 0
    set __SDISPLAY__dirty($displayOID) 0

    set root [session_getRoot $session2]
    set rootname [relation_getName $root]

    # Create a toplevel window for the new tree
    toplevel $w -class Toplevel_$displayOID
    wm geometry $w +${x}+${y}
    wm title $w [session_getName $session2]
    bind Toplevel_$displayOID <Destroy> "sdisplay_exitProc $displayOID"
    _sdisplay_setToplevel $displayOID $w

    _sdisplay_packMenubar $displayOID

    set c [ScrolledCanvas $w.cframe 500 500 { 0 0 500 500 }]
    _sdisplay_setCanvas $displayOID $c

    set rootattrs [relation_getAttributes $root]
    set rootframeID [rframe_makeFrame $root $c]
    set rootframename [rframe_getFrameName $rootframeID]
    _sdisplay_setRFrame $displayOID $root $rootframeID
    _sdisplay_setCollapsed $displayOID $rootframeID 0

    if $allowSave {
	bind $rootframeID <Button-1> \
		"+ sdisplay_selectFrame $displayOID $rootframeID"
	bind $rootframeID <Double-1> \
		"+ sdisplay_toggleNode $displayOID"
    }
    
    tkwait visibility $w
    set tree [ftree_makeTree $c $rootframename]
    _sdisplay_setTree $displayOID $tree
    _sdisplay_addChildrenToTree $displayOID $root

    pack $w.cframe -side top -anchor nw -padx 0 -pady 0 \
	    -fill both -expand true

    _sdisplay_packLinkFrame $displayOID

    if $allowSave {
	sdisplay_selectFrame $displayOID $rootframeID
    } else {
	rframe_highlightOff $rootframeID
    }

    sdisplay_viewLeavesOnly $displayOID
    ftree_resetScrollRegion $tree 1

    update
    scan [split [winfo geometry $w] "+x"] "%d %d %d %d" \
	    ww wh wx wy
    wm geometry $w [winfo reqwidth $w]x[winfo reqheight $w]+${wx}+${wy}

    debug "Done displaying session $session1"

    return $displayOID

}

proc _sdisplay_packMenubar { displayOID } {

    set w [sdisplay_getToplevel $displayOID]

    frame $w.menubar -relief raised -bd 2

    if [sdisplay_allowSave $displayOID] {
	#
	# Session menu
	#
	menubutton $w.menubar.session -text Session \
		-menu $w.menubar.session.menu -underline 0
	set sessionmenu [menu $w.menubar.session.menu -tearoff 0]
	$sessionmenu add command -label Save -underline 0 \
		-command [list sdisplay_sessionSave $displayOID]
	$sessionmenu add command -label "Save As" -underline 1 \
		-command [list sdisplay_sessionSaveAs $displayOID]
	$sessionmenu add separator
	$sessionmenu add command -label Exit -underline 1 -command \
		[list sdisplay_exitProc $displayOID]
	pack $w.menubar.session	-side left -padx 10
    }

    #
    # Relation menu
    #
    menubutton $w.menubar.rel -text Relation -menu $w.menubar.rel.menu \
	    -underline 0
    set relmenu [menu $w.menubar.rel.menu -tearoff 0]
    $relmenu add cascade -label "Test/Decompose..." -underline 0 \
	    -menu $relmenu.decomp
    set submenu [menu $relmenu.decomp -tearoff 0]
    $submenu add command -label "Test BCNF" -underline 5 \
	    -command [list busy \
	    [list sdisplay_testNormalForm $displayOID BCNF]]
    $submenu add command -label "Test 3NF" -underline 5 \
	    -command [list busy \
	    [list sdisplay_testNormalForm $displayOID 3NF]]
    $submenu add separator
    $submenu add command -label "Test Lossless Join" -underline 5 \
	    -command [list busy \
	    [list sdisplay_testDecomp $displayOID "Lossless Join"]]
    $submenu add command -label "Test FD Preserving" -underline 5 \
	    -command [list busy \
	    [list sdisplay_testDecomp $displayOID "Dependency Preserving"]]

    if [sdisplay_allowSave $displayOID] {
	$submenu add separator
	$submenu add command -label "Decompose into BCNF" -underline 15 \
		-command [list busy \
		[list sdisplay_generateNormalForm $displayOID BCNF]]
	$submenu add command -label "Decompose into 3NF" -underline 15 \
		-command [list busy \
		[list sdisplay_generateNormalForm $displayOID 3NF]]
    }

    $relmenu add command -label "Link to Another Session" -underline 0 \
	    -command [list sdisplay_setDefaultDecomposition $displayOID]
    $relmenu add command -label "View/Hide Details" -underline 0 \
	    -command [list sdisplay_viewDetails $displayOID]
    $relmenu add command -label "New Session" -underline 0 \
	    -command [list sdisplay_newSession $displayOID]

    if [sdisplay_allowSave $displayOID] {
	$relmenu add separator
	$relmenu add command -label "Undo Decomposition" -underline 0 \
		-command [list busy \
		[list sdisplay_undoDecomposition $displayOID]]
	$relmenu add command -label Delete -underline 1 \
		-command [list busy \
		[list sdisplay_deleteRelation $displayOID]]
	$relmenu add command -label "Rename" -underline 0 \
		-command [list busy \
		[list sdisplay_changeRelationName $displayOID]]
    }

    #
    # View menu
    #
    menubutton $w.menubar.view -text View -menu $w.menubar.view.menu \
	    -underline 0
    set viewmenu [menu $w.menubar.view.menu -tearoff 0]
    $viewmenu add command -label "Collapse Node" -underline 0 \
	    -command [list sdisplay_collapseNode $displayOID]
    $viewmenu add command -label "Expand Node" -underline 0 \
	    -command [list sdisplay_expandNode $displayOID]
    $viewmenu add command -label "Leaves Only" -underline 0 \
	    -command [list sdisplay_viewLeavesOnly $displayOID]
    $viewmenu add command -label "Expand All" -underline 1 \
	    -command [list sdisplay_expandAll $displayOID]
    $viewmenu add command -label "Collapse All" -underline 1 \
	    -command [list sdisplay_collapseAll $displayOID]

    #
    # Help menu
    #
    menubutton $w.menubar.help -text Help -menu $w.menubar.help.menu \
	    -underline 0
    set helpmenu [menu $w.menubar.help.menu -tearoff 0]
    $helpmenu add command -label "Session Window" -underline 0 \
	    -command [list dbn_showHelp session]

    pack $w.menubar.rel $w.menubar.view -side left -padx 10
    pack $w.menubar.help -side right -padx 10
    pack $w.menubar -side top -anchor nw -fill x

}

proc _sdisplay_packLinkFrame { displayOID } {

    global __SDISPLAY__linkButton
    
    set w [sdisplay_getToplevel $displayOID]
    set f [frame $w.link]
    label $f.label -text "Linked Session:"
    button $f.link -text "" -relief flat -bd 2 -state disabled \
	    -command [list sdisplay_displayLinkedSession $displayOID]

    set __SDISPLAY__linkButton($displayOID) $f.link
    
    pack $f.label -side left
    pack $f.link -side left -expand true -fill x
    pack $f -side top -fill x -anchor nw -padx 2

}

proc sdisplay_isCollapsed { oid frame } {
    global __SDISPLAY__collapsed
    if ![info exists __SDISPLAY__collapsed($oid,$frame)] {
	puts "Error (sdisplay_isCollapsed): invalid display \"$oid\""
	return {}
    }
    return $__SDISPLAY__collapsed($oid,$frame)
}

proc sdisplay_allowSave { oid } {
    global __SDISPLAY__allowSave
    if ![info exists __SDISPLAY__allowSave($oid)] {
	puts "Error (sdisplay_allowSave): invalid display \"$oid\""
	return {}
    }
    return $__SDISPLAY__allowSave($oid)
}

proc sdisplay_getSession { oid } {
    global __SDISPLAY__session
    if ![info exists __SDISPLAY__session($oid)] {
	puts "Error (sdisplay_getSession): invalid display \"$oid\""
	return {}
    }
    return $__SDISPLAY__session($oid)
}

proc sdisplay_getDetail { oid } {
    global __SDISPLAY__detail
    if ![info exists __SDISPLAY__detail($oid)] {
	puts "Error (sdisplay_getDetail): invalid display \"$oid\""
	return {}
    }
    return $__SDISPLAY__detail($oid)
}

proc sdisplay_detailIsVisible { oid } {
    global __SDISPLAY__detailVisible
    if ![info exists __SDISPLAY__detailVisible($oid)] {
	puts "Error (sdisplay_detailIsVisible): invalid display \"$oid\""
	return {}
    }
    return $__SDISPLAY__detailVisible($oid)
}

proc sdisplay_getParent { oid } {
    global __SDISPLAY__parent
    if ![info exists __SDISPLAY__parent($oid)] {
	puts "Error (sdisplay_getParent): invalid display \"$oid\""
	return {}
    }
    return $__SDISPLAY__parent($oid)
}

proc sdisplay_getTree { oid } {
    global __SDISPLAY__tree
    if ![info exists __SDISPLAY__tree($oid)] {
	puts "Error (sdisplay_getTree): invalid display \"$oid\""
	return {}
    }
    return $__SDISPLAY__tree($oid)
}

proc sdisplay_getRFrame { oid relation } {
    global __SDISPLAY__framename
    if ![info exists __SDISPLAY__framename($oid,$relation)] {
	puts -nonewline "Error (sdisplay_getRFrame): "
	puts "invalid display \"$oid\" or relation \"$relation\""
	return {}
    }
    return $__SDISPLAY__framename($oid,$relation)
}

proc sdisplay_getToplevel { oid } {
    global __SDISPLAY__toplevel
    if ![info exists __SDISPLAY__toplevel($oid)] {
	puts "Error (sdisplay_getToplevel): invalid display \"$oid\""
	return {}
    }
    return $__SDISPLAY__toplevel($oid)
}

proc sdisplay_getCanvas { oid } {
    global __SDISPLAY__canvas
    if ![info exists __SDISPLAY__canvas($oid)] {
	puts "Error (sdisplay_getCanvas): invalid display \"$oid\""
	return {}
    }
    return $__SDISPLAY__canvas($oid)
}

proc _sdisplay_setCollapsed { oid frame state } {
    global __SDISPLAY__collapsed
    set __SDISPLAY__collapsed($oid,$frame) $state
}

proc _sdisplay_setCollapsed { oid frame state } {
    global __SDISPLAY__collapsed
    set __SDISPLAY__collapsed($oid,$frame) $state
}

proc _sdisplay_setAllowSave { oid allowSave } {
    global __SDISPLAY__allowSave
    set __SDISPLAY__allowSave($oid) $allowSave
}

proc _sdisplay_setDetailVisible { oid visible } {
    global __SDISPLAY__detailVisible
    set __SDISPLAY__detailVisible($oid) $visible
}

proc _sdisplay_setSession { oid session } {
    global __SDISPLAY__session
    dbnSet __SDISPLAY__session($oid) $session
}

proc _sdisplay_setParent { oid session } {
    global __SDISPLAY__parent
    dbnSet __SDISPLAY__parent($oid) $session
}

proc _sdisplay_setTree { oid tree } {
    global __SDISPLAY__tree
    set __SDISPLAY__tree($oid) $tree
}

proc _sdisplay_setRFrame { oid relation frame } {
    global __SDISPLAY__framename
    set __SDISPLAY__framename($oid,$relation) $frame
}

proc _sdisplay_setToplevel { oid win } {
    global __SDISPLAY__toplevel
    set __SDISPLAY__toplevel($oid) $win
}

proc _sdisplay_setCanvas { oid c } {
    global __SDISPLAY__canvas
    set __SDISPLAY__canvas($oid) $c
}

proc _sdisplay_setDetail { oid d } {
    global __SDISPLAY__detail
    set __SDISPLAY__detail($oid) $d
}

proc sdisplay_displayLinkedSession { oid } {

    global __SDISPLAY__selected
    if ![info exists __SDISPLAY__selected($oid)] {
	return
    }
    
    set rframe $__SDISPLAY__selected($oid)
    set relation [rframe_getRelation $rframe]
    set session [sdisplay_getSession $oid]
    set link [session_getDefaultDecomposition $session $relation]
    if {$link != {}} {
	set w [sdisplay_getToplevel $oid]
	set x [expr [winfo rootx $w] + 25]
	set y [expr [winfo rooty $w] + 25]
	sdisplay_makeSDisplay $link $x $y
    }

}

proc sdisplay_selectFrame { oid rframe } {

    global __SDISPLAY__selected
    global __SDISPLAY__linkButton
    
    if [info exists __SDISPLAY__selected($oid)] {
	rframe_highlightOff $__SDISPLAY__selected($oid)
    }
    
    rframe_highlightOn $rframe
    set __SDISPLAY__selected($oid) $rframe
    
    set relation [rframe_getRelation $rframe]
    set session [sdisplay_getSession $oid]
    set link [session_getDefaultDecomposition $session $relation]
    if {$link != {}} {
	$__SDISPLAY__linkButton($oid) config -state normal \
		-relief raised -text [session_getName $link]
    } else {
	$__SDISPLAY__linkButton($oid) config -state disabled \
		-relief flat -text ""
    }
    
    if [sdisplay_detailIsVisible $oid] {
	updateDetailWindow [sdisplay_getDetail $oid] $relation
    }
    
}

proc sdisplay_getSelectedFrame { oid } {
    global __SDISPLAY__selected
    if ![info exists __SDISPLAY__selected($oid)] { return {} }
    return $__SDISPLAY__selected($oid)
}

proc sdisplay_getActiveRelation { oid } {
    set f [sdisplay_getSelectedFrame $oid]
    if {$f == {}} {
	return {}
    }
    return [rframe_getRelation $f]
}

proc _sdisplay_addChildrenToTree { sessionDisplay relation } {

    debug "Adding children of $relation to display $sessionDisplay"

    set rframe [sdisplay_getRFrame $sessionDisplay $relation]
    set session [sdisplay_getSession $sessionDisplay]
    set kids [session_getChildren $session $relation]

    if {$kids == {}} {
	debug "    No children"
	return
    }
    debug "    Children=$kids"

    set parentframe [rframe_getFrameName $rframe]
    set canvas [sdisplay_getCanvas $sessionDisplay]
    set tree [sdisplay_getTree $sessionDisplay]

    set child_frames {}
    foreach child $kids {
	set rframe [rframe_makeFrame $child $canvas]
	rframe_highlightOff $rframe
	bind $rframe <Button-1> \
		"+ sdisplay_selectFrame $sessionDisplay $rframe"
	bind $rframe <Double-1> \
		"+ sdisplay_toggleNode $sessionDisplay"
	lappend child_frames [rframe_getFrameName $rframe]
	_sdisplay_setRFrame $sessionDisplay $child $rframe
	_sdisplay_setCollapsed $sessionDisplay $rframe 0
    }
    ftree_addChildren $tree $parentframe $child_frames

    foreach child $kids {
	_sdisplay_addChildrenToTree $sessionDisplay $child
    }
}

proc sdisplay_deleteAllWindows {} {
    global __SDISPLAY__toplevel
    foreach oid [array names __SDISPLAY__toplevel] {
	destroy $__SDISPLAY__toplevel($oid)
    }
}

proc sdisplay_delete { oid } {

    debug "Deleting SDisplay $oid"

    set session [sdisplay_getSession $oid]
    set parent [sdisplay_getParent $oid]

    # Delete all the relation frames and the frame tree
    foreach r [session_getAllRelations $session] {
	rframe_delete [sdisplay_getRFrame $oid $r]
    }
    ftree_delete [sdisplay_getTree $oid]

    # Decrement the reference count for the session and the parent session
    dbnDelete $session
    dbnDelete $parent

    # Clear all state variables
    foreach array [info globals __SDISPLAY__*] {
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

proc sdisplay_newSession { oid } {
    set f [sdisplay_getSelectedFrame $oid]
    if {$f == {}} {
	one_line_message .errmsg Error "You must select a relation first"
	return
    }
    set relation [rframe_getRelation $f]
    set session [session_makeSession $relation]
    session_setName $session "New Session"
    sdisplay_makeSDisplay $session \
	    [expr 25 + [winfo rootx [sdisplay_getToplevel $oid]]] \
	    [expr 25 + [winfo rooty [sdisplay_getToplevel $oid]]]
}

proc sdisplay_deleteRelation { oid } {
    
    global __SDISPLAY__dirty

    debug "Deleting a relation from the session display..."

    set f [sdisplay_getSelectedFrame $oid]
    if {$f == {}} {
	one_line_message .errmsg Error "You must select a relation first"
	return
    }

    set deadrelation [rframe_getRelation $f]
    set rname [relation_getName $deadrelation]
    set session [sdisplay_getSession $oid]
    set parent [session_getParent $session $deadrelation]
    set children [session_getChildren $session $deadrelation]
    debug "    relation=$deadrelation, rname=$rname"
    debug "    session=$session, parent=$parent"

    # Can't delete root or a node with children
    if {$parent == {}} {
	one_line_message .errmsg Error "You cannot delete the root node"
	return
    }
    if {$children != {}} {
	one_line_message .errmsg Error "You cannot delete an internal node"
	return
    }

    set answer [dialog .confirm "Delete Relation" \
            "Are you sure you want to delete the $rname relation?" \
	    "" 0 {Cancel} {Delete}]
    if {$answer == 0} { return }

    sdisplay_selectFrame $oid [sdisplay_getRFrame $oid $parent]

    set tree [sdisplay_getTree $oid]
    set result [ftree_deleteNode $tree [rframe_getFrameName $f]]
    if $result {
	one_line_message .errmsg Error "Error: [ftree_getError]"
	return
    }
    
    ftree_resetScrollRegion $tree 0
    session_deleteChild $session $parent $deadrelation
    set __SDISPLAY__dirty($oid) 1
    
    debug "Done deleting the relation"

}

proc sdisplay_undoDecomposition { oid {parentrelation {}} } {

    global __SDISPLAY__dirty

    debug "Undoing a decomposition..."

    if {$parentrelation == {}} {
	
	set parentframe [sdisplay_getSelectedFrame $oid]
	if {$parentframe == {}} {
	    one_line_message .errmsg Error "You must select a relation first"
	    return
	}
	
	set response [dialog .confirm "Undo Decomposition" \
		"Delete all derived relations?" \
		"" 0 {Cancel} {Delete}]
	if {$response == 0} { return }
	
	set parentrelation [rframe_getRelation $parentframe]
	
    } else {
	
	set parentframe [sdisplay_getRFrame $oid $parentrelation]
	if {$parentframe == {}} { return }
	
    }
    
    set parentname [relation_getName $parentrelation]
    set session [sdisplay_getSession $oid]
    set childrelations [session_getChildren $session $parentrelation]
    set tree [sdisplay_getTree $oid]
    
    foreach child $childrelations {
	set grandkids [session_getChildren $session $child]
	if {$grandkids != {}} {
	    sdisplay_undoDecomposition $oid $child
	}
	set childframe [sdisplay_getRFrame $oid $child]
	set result [ftree_deleteNode $tree [rframe_getFrameName $childframe]]
	if $result {
	    one_line_message .errmsg Error "Error: [ftree_getError]"
	    return
	}
    }
    
    ftree_resetScrollRegion $tree 0
    session_deleteChildren $session $parentrelation
    set __SDISPLAY__dirty($oid) 1

    debug "Done deleting the relation"

}

proc sdisplay_changeRelationName { oid } {

    global __SDISPLAY__dirty

    debug "Changing a relation name"

    set frame [sdisplay_getSelectedFrame $oid]
    if {$frame == {}} {
	one_line_message .errmsg Error "You must select a relation first"
	return
    }
	
    set relation [rframe_getRelation $frame]
    set w [sdisplay_getToplevel $oid]
    set newname [input_box $w.newname \
	    "New Relation Name" "Enter the new relation name:" \
	    [relation_getName $relation]]

    if {$newname == {}} { return }

    rframe_setRelationName $frame $newname
    relation_setName $relation $newname
    
    if [db_isSourceRelation $relation] {
	main_updateRelationNames
    }

    set __SDISPLAY__dirty($oid) 1
    
    debug "Done changing the relation name"

}

proc sdisplay_testDecomp { oid testname } {

    debug "Testing a decomposition for $testname"

    set frame [sdisplay_getSelectedFrame $oid]
    if {$frame == {}} {
	one_line_message .errmsg Error "You must select a relation first"
	return
    }
	
    set relation [rframe_getRelation $frame]
    set rname [relation_getName $relation]
    set session [sdisplay_getSession $oid]
    set children [session_getChildren $session $relation]
    set tree [sdisplay_getTree $oid]
    
    if {$children == {}} {
	one_line_message .errmsg Error \
		"This relation has not been decomposed"
	return
    }

    foreach child $children {
	set decomparray($child) [relation_getAttributes $child]
    }

    set valid [db_testDecomposition $relation decomparray $testname]

    if {[lindex $valid 0] == "TRUE"} {
	one_line_message .message "$testname Test" \
		"YES! This is a $testname decomposition."
	return
    }

    if {[lindex $valid 0] == "FALSE"} {
	one_line_message .message "$testname Test" \
		"NO! This is NOT a $testname decomposition."
	return
    }

    if {[lindex $valid 0] == -1} {
	one_line_message .message "$testname Test" \
		"Error: [lrange $valid 2 end]"
    } else {
	one_line_message .message "$testname Test" "Unknown error: $result"
    }

    debug "Done testing the decomposition"

}

proc sdisplay_customDecompose { oid } {

    global __SDISPLAY__dirty

    debug "Decomposing a relation from the session display..."

    set parentframe [sdisplay_getSelectedFrame $oid]
    if {$parentframe == {}} {
	one_line_message .errmsg Error "You must select a relation first"
	return
    }

    set relation [rframe_getRelation $parentframe]
    set rname [relation_getName $relation]
    set session [sdisplay_getSession $oid]
    debug "    relation=$relation, rname=$rname, session=$session"

    # Can't decompose a node with children
    if {[session_getChildren $session $relation] != {}} {
	one_line_message .errmsg Error "You cannot decompose an internal node"
	return
    }

    set result [dbn_getDecomposition $relation decomparray]
    if {$result == {}} { return }

    # Create new relation objects
    set childNames [lsort [array names decomparray]]
    set childAttrSets {}
    foreach r $childNames {
	lappend childAttrSets $decomparray($r)
    }
    set relationOIDs [sdisplay_makeChildRelations \
	    $relation $childAttrSets $childNames]

    # Notify the Session object of the new decomposition
    session_addChildren $session $relation $relationOIDs

    # Add the relation frames to the tree
    _sdisplay_addChildrenToTree $oid $relation
    ftree_resetScrollRegion [sdisplay_getTree $oid] 0

    set __SDISPLAY__dirty($oid) 1
    
    debug "Done decomposing"

}

proc sdisplay_violationDecompose { oid relation lhs rhs } {

    global __SDISPLAY__dirty

    debug "Decomposing to remove violation $lhs -> $rhs"

    if {$lhs == {}} {
	one_line_message .errmsg Error \
		"Invalid decomposition: {$lhs} -> {$rhs}"
	return
    }
    if {$rhs == {}} {
	one_line_message .errmsg Error \
		"Invalid decomposition: {$lhs} -> {$rhs}"
	return
    }

    set parentframe [sdisplay_getSelectedFrame $oid]
    set rname [relation_getName $relation]
    set session [sdisplay_getSession $oid]
    debug "    relation=$relation, rname=$rname, session=$session"

    # Can't decompose a node with children
    if {[session_getChildren $session $relation] != {}} {
	one_line_message .errmsg Error "You cannot decompose an internal node"
	return
    }

    set allAttrs [relation_getAttributes $relation]
    set attrset1 [concat $lhs $rhs]
    set attrset2 [concat $lhs [set_difference $allAttrs $attrset1]]
    set key1 $lhs
    set key2 [relation_getKey $relation]
    if {[set_intersection $key2 $rhs] != {}} {
	set key2 [set_union $key2 $lhs]
    }

    debug "    attrset1=$attrset1, key1=$key1"
    debug "    attrset2=$attrset2, key2=$key2"

    array set decomparray [list 1 $attrset1 2 $attrset2]
    set confirm [db_confirmDecomposition $relation decomparray]
    if {[lindex $confirm 0] == "-1"} {
	one_line_message .message "Decomposition Error" \
		"Error: [lrange $confirm 2 end]"
	return
    }
    if {$confirm != "TRUE"} {
	return
    }

    #
    # Copy Foreign Key declarations to the child relations
    #
    set childFKey1 {}
    set childFKey2 {}
    set parentFKey [relation_getForeignKey $relation]
    foreach fk $parentFKey {
	if {[set_difference [lindex $fk 0] $attrset1] == {}} {
	    lappend childFKey1 $fk
	}
	if {[set_difference [lindex $fk 0] $attrset2] == {}} {
	    lappend childFKey2 $fk
	}
    }

    #
    # Notify the Session object of the new decomposition
    #
    session_addChildren $session $relation [list \
	    [relation_makeRelation ${rname}1 $attrset1 $key1 $childFKey1] \
	    [relation_makeRelation ${rname}2 $attrset2 $key2 $childFKey2]]

    #
    # Add the relation frames to the tree
    #
    _sdisplay_addChildrenToTree $oid $relation
    ftree_resetScrollRegion [sdisplay_getTree $oid] 0

    set __SDISPLAY__dirty($oid) 1
    
    debug "Done decomposing to remove violation"

}

proc sdisplay_viewDecomp { oid } {

    debug "Viewing decompositions for display $oid"

    set frame [sdisplay_getSelectedFrame $oid]
    if {$frame == {}} {
	one_line_message .errmsg Error "You must select a relation first"
	return
    }

    set relation [rframe_getRelation $frame]
    set rname [relation_getName $relation]
    set session [sdisplay_getSession $oid]
    debug "    relation=$relation, rname=$rname, session=$session"

    sesslist_displayList $relation \
	    [expr 25 + [winfo rootx [sdisplay_getToplevel $oid]]] \
	    [expr 25 + [winfo rooty [sdisplay_getToplevel $oid]]]

    debug "Done viewing decompositions"

}

proc sdisplay_viewDerivHist { oid } {

    one_line_message .errmsg Error "Not implemented yet"
    return

    debug "Viewing derivation history for display $oid"

    set frame [sdisplay_getSelectedFrame $oid]
    if {$frame == {}} {
	one_line_message .errmsg Error "You must select a relation first"
	return
    }

    set relation [rframe_getRelation $frame]
    set session [sdisplay_getSession $oid]
    set parentSession [relation_getParentSession $relation]
    debug "    relation=$relation, session=$session"
    debug "    parentSession=$parentSession"

    if {[session_getParent $session $relation] != {}} {
	one_line_message .errmsg Error \
		"You can only view a derivation history for the root node"
	return
    }

    if {$parentSession == {}} {
	one_line_message .errmsg "Derivation History" \
		"No history exists for [relation_getName $relation]"
	return
    }

    sdisplay_makeSDisplay $parentSession \
	    [expr 25 + [winfo rootx [sdisplay_getToplevel $oid]]] \
	    [expr 25 + [winfo rooty [sdisplay_getToplevel $oid]]]

    debug "Done viewing derivation history"

}

proc sdisplay_viewDetails { oid } {

    debug "Viewing relation details for display $oid"
    
    set frame [sdisplay_getSelectedFrame $oid]
    if {$frame == {}} {
	one_line_message .errmsg Error "You must select a relation first"
	return
    }
    
    set relation [rframe_getRelation $frame]
    debug "    relation=$relation"

    set w [sdisplay_getToplevel $oid]
    set c [sdisplay_getCanvas $oid]
    
    update
    scan [split [winfo geometry $w] "+x"] "%d %d %d %d" \
	    oldWidth oldHeight wx wy

    if [sdisplay_detailIsVisible $oid] {

	destroy $w.border1
	detail_delete [sdisplay_getDetail $oid]
	_sdisplay_setDetailVisible $oid 0
	pack $w.cframe -expand true

	update
	set newWidth [min [winfo reqwidth $w] $oldWidth]
	set newHeight [min [winfo reqheight $w] $oldHeight]
	
    } else {
	
	pack $w.cframe -expand false

	frame $w.border1 -bg black -height 4 -cursor sb_v_double_arrow
	pack $w.border1 -side top -fill x
	
	bind $w.border1 <Button-1> \
		[concat set __SDISPLAY__y1 %Y]
	bind $w.border1 <ButtonRelease-1> \
		[concat resizeWidget $c height \
		{ [expr %Y - $__SDISPLAY__y1] ; } ]

	_sdisplay_setDetail $oid [detail_makeDetailFrame \
		[sdisplay_getToplevel $oid] $relation \
		[sdisplay_getSession $oid]]
	pack [detail_getFrame [sdisplay_getDetail $oid]] \
		-side bottom -fill both -expand true
	_sdisplay_setDetailVisible $oid 1
    
	update
	set newWidth [max $oldWidth [winfo reqwidth $w.detail]]
	set newHeight [expr $oldHeight + [winfo reqheight $w.detail] + 4]

    }

    wm geometry $w ${newWidth}x${newHeight}+${wx}+${wy}

}

proc sdisplay_setDefaultDecomposition { oid } {

    global __SDISPLAY__linkButton
    global __SDISPLAY__dirty

    debug "Setting default decomp for display $oid"

    set frame [sdisplay_getSelectedFrame $oid]
    if {$frame == {}} {
	one_line_message .errmsg Error "You must select a relation first"
	return
    }

    set relation [rframe_getRelation $frame]
    set session [sdisplay_getSession $oid]
    set sessions [db_getSessions $relation]
    debug "    relation=$relation, session=$session"
    
    if {$sessions == {}} {
	one_line_message .message "Error" \
		"No other sessions exist for this relation."
	return
    }

    set snames {}
    foreach s $sessions {
	lappend snames [session_getName $s]
    }
    lappend snames "<No Decomposition>"
    
    set choice [SelectionBox \
	    [list Select the session that relation \
	    '[relation_getName $relation]' should be linked to:] \
	    $snames "Default Decomposition"]
    if {$choice == {}} {
	return 
    } elseif {$choice == [llength $sessions]} {
	set default {}
    } else {
	set default [lindex $sessions $choice]
    }
    
    session_setDefaultDecomposition $session $relation $default

    if {$default != {}} {
	$__SDISPLAY__linkButton($oid) config -state normal \
		-relief raised -text [session_getName $default]
    } else {
	$__SDISPLAY__linkButton($oid) config -text "" -state disabled \
		-relief flat
    }

    set __SDISPLAY__dirty($oid) 1
    
}

proc sdisplay_viewFDProjection { oid } {

    global __SDISPLAY__FDProjWin
    global __SDISPLAY__FDProjList
    global __SDISPLAY__FDProjVisible

    debug "Viewing FD projection for display $oid"

    set frame [sdisplay_getSelectedFrame $oid]
    if {$frame == {}} {
	one_line_message .errmsg Error "You must select a relation first"
	return
    }

    set relation [rframe_getRelation $frame]
    set attrSet [relation_getAttrSet $relation]
    debug "    relation=$relation, attrSet=$attrSet"

    set w $__SDISPLAY__FDProjWin

    if ![winfo exists $w] {
	toplevel $w
	wm geometry $w \
		+[expr 25 + [winfo rootx $w]]+[expr 25 + [winfo rootx $w]]

	set __SDISPLAY__FDProjList \
		[ScrolledListbox $__SDISPLAY__FDProjWin.listframe 1 8 \
		-selectmode single -setgrid true -height 8 -width 50]
	pack $w.listframe -side top -fill both -expand true
	bind $w <Destroy> {
	    set __SDISPLAY__FDProjVisible 0
	}
	set __SDISPLAY__FDProjVisible 1
    }

    wm title $w \
	    "FD Projection for [relation_getName $relation]"

    $__SDISPLAY__FDProjList delete 0 end
    $__SDISPLAY__FDProjList insert end "Computing..."
    update
    set fds [db_getTextOfAllFDs $attrSet]
    $__SDISPLAY__FDProjList delete 0 end

    if {$fds == {}} {
	$__SDISPLAY__FDProjList insert end "<No FDs>"
    } else {
	eval $__SDISPLAY__FDProjList insert end $fds
    }

    debug "Done displaying FD projection"

}

proc sdisplay_testNormalForm { oid normalform } {

    debug "Starting $normalform test..."

    set frame [sdisplay_getSelectedFrame $oid]
    if {$frame == {}} {
	one_line_message .errmsg Error "You must select a relation first"
	return
    }

    set relation [rframe_getRelation $frame]
    set rname [relation_getName $relation]
    set w [sdisplay_getToplevel $oid]
    debug "    relation=$relation, rname=$rname"

    set result [db_testNormalForm $relation $normalform]

    if {[lindex $result 0] == "TRUE"} {
	one_line_message .message "$normalform Test" \
		"YES! $rname is in $normalform."
    } elseif {[lindex $result 0] == -1} {
	one_line_message .message "$normalform Test" \
		"Error: [lrange $result 2 end]"
    } else {

	set response [sdisplay_showViolations $oid $relation \
		$result $normalform]

	switch -- [lindex $response 0] {
	    "VIOLATION" {
		set index [lindex $response 1]
		if {$index != {}} {
		    set fdtext [lindex $result $index]
		    set lhs [split [lindex $fdtext 0] ,]
		    set rhs [split [lindex $fdtext 2] ,]
		    sdisplay_violationDecompose $oid $relation $lhs $rhs
		}
	    }
	    "CUSTOM" {
		sdisplay_customDecompose $oid
	    }
	}

    }

    debug "Done with $normalform test."

}

proc sdisplay_showViolations { oid relation violations normalform } {

    global __SDISPLAY__button

    set w [sdisplay_getToplevel $oid]
    set tl [toplevel $w.violations]
    wm title $tl "$normalform Test"
    wm geometry $tl +[expr 25+[winfo rootx $w]]+[expr 25+[winfo rooty $w]]

    set top [frame $tl.top]
    label $top.label -text "Violations of $normalform:"
    set list [ScrolledListbox $top.listframe 1 10 -setgrid true -width 50]
    eval $list insert end $violations
    pack $top.label -side top -anchor nw
    pack $top.listframe -side top -fill both -expand true

    set bottom [frame $tl.bottom]
    button $bottom.dismiss -text Dismiss -command [list \
	    set __SDISPLAY__button {}]
    button $bottom.decomp -text Decompose -command [list \
	    set __SDISPLAY__button VIOLATION]
    button $bottom.custom -text "Custom Decomposition" -command [list \
	    set __SDISPLAY__button CUSTOM]
    button $bottom.help -text "Help" \
	    -command [list dbn_showHelp violations]
    frame $bottom.default -relief sunken -bd 1
    raise $bottom.dismiss
    pack $bottom.default -side left \
	    -expand 1 -fill x -padx 3m -pady 2m
    pack $bottom.dismiss -in $bottom.default -side left \
	    -expand 1 -fill x -padx 1m -pady 1m
    pack $bottom.decomp -side left -expand 1 -fill x \
	    -padx 3m -pady 3m -ipadx 2m -ipady 1m
    pack $bottom.custom -side left -expand 1 -fill x \
	    -padx 3m -pady 3m -ipadx 2m -ipady 1m
    pack $bottom.help -side left -expand 1 -fill x \
	    -padx 3m -pady 3m -ipadx 2m -ipady 1m
	
    pack $top -side top -fill both -expand true
    pack $bottom -side bottom -fill x -expand true

    bind $tl <Return> "$bottom.dismiss flash; \
	    set __SDISPLAY__button {}"
    bind $tl <Destroy> "set __SDISPLAY__button {}"
    
    set oldFocus [focus]
    tkwait visibility $tl
    grab set $tl
    focus $tl

    while 1 {

	tkwait variable __SDISPLAY__button
	set result $__SDISPLAY__button

	if {$result == "VIOLATION"} {

	    if {[$list curselection] == {}} {
		one_line_message .errmsg Error \
			"You must first select a violation"
		continue
	    }

	    if ![sdisplay_allowSave $oid] {
		set confirm [dialog .confirm "Start New Session?" \
			[list You cannot decompose in this window. \
			Start a new decomposition session?] \
			"" 0 {Cancel} {New Session}]
		if $confirm {
		    set ss [session_makeSession $relation]
		    session_setName $ss "New Session"
		    set newdisplay [sdisplay_makeSDisplay $ss \
			    [expr [winfo rootx $tl] + 25] \
			    [expr [winfo rooty $tl] + 25]]

		    set fdtext [$list get [$list curselection]]
		    sdisplay_violationDecompose $newdisplay $relation \
			    [split [lindex $fdtext 0] ,] \
			    [split [lindex $fdtext 2] ,]

		}
		continue
	    }

	    lappend result [$list curselection]

	}

	break

    }

    if [winfo exists $tl] { destroy $tl }

    catch focus $oldFocus
    return $result
 
}

proc sdisplay_generateNormalForm { oid normalform } {

    global __SDISPLAY__dirty

    debug "Generating a $normalform decomposition..."

    set frame [sdisplay_getSelectedFrame $oid]
    if {$frame == {}} {
	one_line_message .errmsg Error "You must select a relation first"
	return
    }

    set relation [rframe_getRelation $frame]
    set rname [relation_getName $relation]
    set session [sdisplay_getSession $oid]
    set w [sdisplay_getToplevel $oid]
    debug "    relation=$relation, rname=$rname, session=$session"

    #
    # Can't decompose a node with children
    #
    if {[session_getChildren $session $relation] != {}} {
	one_line_message .errmsg Error "You cannot decompose an internal node"
	return
    }

    #
    # Generate a decomposition (a list of attribute lists)
    #
    set result [db_generateNormalForm $relation $normalform]

    #
    # Check for errors
    #
    if {[lindex $result 0] == -1} {
	one_line_message .message "$normalform Test" \
		"Error: [lrange $result 2 end]"
	return
    }

    #
    # Stop if the decomposition consists of a single attribute list,
    # indicating that the original relation is in the correct normal form.
    #
    if {[llength $result] == 1} {
	one_line_message .message "$normalform Test" \
		"This relation is already in $normalform"
	return
    }

    #
    # Make user confirm a decomp that is not lossless join or FD preserving
    #
    set arrayList {}
    set i 1
    foreach r $result {
	lappend arrayList DummyName$i
	lappend arrayList $r
	incr i
    }
    array set decomparray $arrayList
    set confirm [db_confirmDecomposition $relation decomparray]
    if {[lindex $confirm 0] == "-1"} {
	one_line_message .message "Decomposition Error" \
		"Error: [lrange $confirm 2 end]"
	return
    }
    if {$confirm != "TRUE"} {
	return
    }


    #
    # Create new relation objects
    #
    set relationOIDs [sdisplay_makeChildRelations $relation $result]

    #
    # Notify the Session object of the new decomposition
    #
    session_addChildren $session $relation $relationOIDs

    #
    # Add the relation frames to the tree
    #
    _sdisplay_addChildrenToTree $oid $relation
    ftree_resetScrollRegion [sdisplay_getTree $oid] 0

    set __SDISPLAY__dirty($oid) 1
    
    debug "Done generating $normalform decomposition"
    
}

proc sdisplay_sessionSave { oid } {

    global __SDISPLAY__dirty

    set copy [sdisplay_getSession $oid]
    set original [sdisplay_getParent $oid]

    #
    # If this is a new session (one that is not yet owned by a 
    # relation), then ask user to name the session. Otherwise
    # just copy the session to the parent session.
    #
    if {[getRefCount $original] == 1} {
	sdisplay_sessionSaveAs $oid
    } else {
	session_copyData $copy $original
	file_save
	set __SDISPLAY__dirty($oid) 0
    }

}

proc sdisplay_sessionSaveAs { oid } {

    global __SDISPLAY__dirty

    set name [input_box .inputbox "New Session Name" \
	    "Enter a name for the new session:"]
    if {$name == {}} { return {} }

    set session [sdisplay_getSession $oid]
    set relation [session_getRoot $session]
    set newSession [db_newSession $relation]
    session_copyData $session $newSession
    session_setName $newSession $name
    _sdisplay_setParent $oid $newSession
    
    wm title [sdisplay_getToplevel $oid] $name
    set detailRelation [sdisplay_getActiveRelation $oid]
    if {$detailRelation != {} && [sameOID $relation $detailRelation] && \
	[sdisplay_detailIsVisible $oid]} {
	updateDetailWindow [sdisplay_getDetail $oid] $relation
    }
    
    file_save
    set __SDISPLAY__dirty($oid) 0
    
}

proc sdisplay_toggleNode { oid } {
    set collapsed [sdisplay_isCollapsed $oid [sdisplay_getSelectedFrame $oid]]
    if {$collapsed == {}} { return }
    if $collapsed {
	sdisplay_expandNode $oid
    } else {
	sdisplay_collapseNode $oid
    }
}

proc sdisplay_collapseNode { oid } {
    set frame [sdisplay_getSelectedFrame $oid]
    if {$frame == {}} {
	one_line_message .errmsg Error "You must select a relation first"
	return
    }
    set framename [rframe_getFrameName $frame]
    set tree [sdisplay_getTree $oid]
    ftree_setHeight $tree $framename [rframe_getCollapsedHeight $frame]
    _sdisplay_setCollapsed $oid $frame 1
}

proc sdisplay_expandNode { oid } {
    set frame [sdisplay_getSelectedFrame $oid]
    if {$frame == {}} {
	one_line_message .errmsg Error "You must select a relation first"
	return
    }
    set framename [rframe_getFrameName $frame]
    set tree [sdisplay_getTree $oid]
    ftree_setHeight $tree $framename [rframe_getExpandedHeight $frame]
    ftree_resetScrollRegion $tree 0
    _sdisplay_setCollapsed $oid $frame 0
}

proc sdisplay_viewLeavesOnly { oid } {
    set session [sdisplay_getSession $oid]
    set tree [sdisplay_getTree $oid]
    foreach r [session_getAllRelations $session] {
	set rframe [sdisplay_getRFrame $oid $r]
	set framename [rframe_getFrameName $rframe]
	if {[session_getChildren $session $r] == {}} {
	    set newheight [rframe_getExpandedHeight $rframe]
	    _sdisplay_setCollapsed $oid $rframe 0
	} else {
	    set newheight [rframe_getCollapsedHeight $rframe]
	    _sdisplay_setCollapsed $oid $rframe 1
	}
	ftree_setHeight $tree $framename $newheight
    }
}

proc sdisplay_expandAll { oid } {
    set session [sdisplay_getSession $oid]
    set tree [sdisplay_getTree $oid]
    foreach r [session_getAllRelations $session] {
	set rframe [sdisplay_getRFrame $oid $r]
	set framename [rframe_getFrameName $rframe]
	ftree_setHeight $tree $framename \
		[rframe_getExpandedHeight $rframe]
	_sdisplay_setCollapsed $oid $rframe 0
    }
    ftree_resetScrollRegion $tree 1
}

proc sdisplay_collapseAll { oid } {
    set session [sdisplay_getSession $oid]
    set tree [sdisplay_getTree $oid]
    foreach r [session_getAllRelations $session] {
	set rframe [sdisplay_getRFrame $oid $r]
	set framename [rframe_getFrameName $rframe]
	ftree_setHeight $tree $framename \
		[rframe_getCollapsedHeight $rframe]
	_sdisplay_setCollapsed $oid $rframe 1
    }
}

proc sdisplay_makeChildRelations { parent childAttrSets {childNames {}} } {
    
    set childOIDs {}
    set parentName [relation_getName $parent]
    set parentKey [relation_getKey $parent]
    set parentFKey [relation_getForeignKey $parent]
    set i 1
    
    foreach attrset $childAttrSets {

	set childKey {}
	set childFKey {}
	set childName ${parentName}$i

	if {$childNames != {}} {
	    set childName [lindex $childNames [expr $i - 1]]
	}

	if {[set_difference $parentKey $attrset] == {}} {
	    set childKey $parentKey
	}

	foreach fk $parentFKey {
	    if {[set_difference [lindex $fk 0] $attrset] == {}} {
		lappend childFKey $fk
	    }
	}

	lappend childOIDs [relation_makeRelation \
		$childName $attrset $childKey $childFKey]

	incr i

    }

    return $childOIDs

}

proc sdisplay_exitProc { oid } {
    
    global __SDISPLAY__dirty
    
    if ![info exists __SDISPLAY__dirty($oid)] {
	return
    }
    
    if $__SDISPLAY__dirty($oid) {
	set confirm [dialog .confirm "Save Changes?" \
		"Save changes before closing?" \
		"" 0 {Yes} {No} {Cancel}]
	if {$confirm == 2} { return }
	if {$confirm == 0} { sdisplay_sessionSave $oid }
    }

    set w [sdisplay_getToplevel $oid]
    sdisplay_delete $oid
    destroy $w

}

