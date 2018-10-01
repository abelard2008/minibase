
set __FTREE__gap 8
set __FTREE__errmsg {}

proc ftree_setCanvas { oid c } {
    global __FTREE__canvas
    set __FTREE__canvas($oid) $c
}

proc ftree_getCanvas { oid } {
    global __FTREE__canvas
    return $__FTREE__canvas($oid)
}

proc ftree_makeTree { c f } {

    global __FTREE__parent
    global __FTREE__children
    global __FTREE__child_edges
    global __FTREE__window_id

    set treeID [newOID FTree]
    ftree_setCanvas $treeID $c

    debug "Creating frame tree: canvas=$c, root=$f, treeID=$treeID"

    # Create a window on the canvas
    set i [$c create window 0 0 -anchor n -window $f -tag Root]

    # Store information about this box in the global arrays.
    set __FTREE__parent($treeID,$i) {}
    set __FTREE__children($treeID,$i) {}
    set __FTREE__child_edges($treeID,$i) {}
    set __FTREE__window_id($treeID,$f) $i

    return $treeID
}

proc ftree_addChildren { treeID f child_frames } {

    global __FTREE__parent
    global __FTREE__children
    global __FTREE__child_edges
    global __FTREE__gap
    global __FTREE__window_id

    if {$child_frames == {}} { return }

    debug "FTREE $treeID: Adding children of $f, children=$child_frames"

    set c [ftree_getCanvas $treeID]

    set i $__FTREE__window_id($treeID,$f)
    debug "    canvas id=$i"

    set result [scan [$c bbox $i] "%d %d %d %d" pleft ptop pright pbot]
    set pcenter [expr ($pleft + $pright) / 2]
    debug "    parent bbox = $pleft $ptop $pright $pbot"

    set parent $__FTREE__parent($treeID,$i)
    set children $__FTREE__children($treeID,$i)
    set arcs $__FTREE__child_edges($treeID,$i)
    set descendants [_ftree_getDescendants $treeID $i]
    debug "    Children: $children"
    debug "    Arcs: $arcs"
    debug "    Descendants: $descendants"

    if {$children == {}} {
	set cleft $pleft
	set old_right $pright
    } else {
	scan [eval $c bbox $descendants] "%d %d %d %d" cleft ctop cright cbot
	debug "    old descendant bbox = $cleft $ctop $cright $cbot"
	set cleft [expr $cright + (2 * $__FTREE__gap)]
	set old_right [max $pright $cright]
    }
    set ctop [expr $pbot + (2 * $__FTREE__gap)]
    debug "    new child will have top edge at y=$ctop"

    set cid_list {}
    set arc_list {}
    foreach child $child_frames {
	debug "    Adding child: $child"

	set cwidth [winfo reqwidth $child]
	set ccenter [expr $cleft + ($cwidth / 2)]
	debug "    child coords=($ccenter,$ctop)"
	set cid [$c create window $ccenter $ctop \
		-anchor n -window $child]
	incr cleft [expr $cwidth + (2 * $__FTREE__gap)]

	if {$parent == {}} {
	    set arc [$c create line $pcenter $ptop $ccenter $ctop -tag Root]
	} else {
	    set arc [$c create line $pcenter $ptop $ccenter $ctop]
	}

	set __FTREE__parent($treeID,$cid) $i
	set __FTREE__children($treeID,$cid) {}
	set __FTREE__child_edges($treeID,$cid) {}
	set __FTREE__window_id($treeID,$child) $cid

	lappend cid_list $cid
	lappend arc_list $arc
    }

    debug "    ** Done looping through children **"

    set children [eval list $children $cid_list]
    set arcs [eval list $arcs $arc_list]
    debug "    children=$children, arcs=$arcs"

    set __FTREE__children($treeID,$i) $children
    set __FTREE__child_edges($treeID,$i) $arcs
    
    set new_right \
	    [lindex [eval $c bbox $children] 2]
    if {$new_right > $old_right} {
	_ftree_pushRightSiblings $treeID $i [expr $new_right - $old_right]
	_ftree_centerOverChildren $treeID $i
    }

    debug "    FTREE $treeID: done adding children"
}

proc ftree_deleteNode { treeID f } {

    global __FTREE__parent
    global __FTREE__children
    global __FTREE__child_edges
    global __FTREE__window_id
    global __FTREE__gap

    debug "Deleting node $f from tree $treeID"

    set c [ftree_getCanvas $treeID]
    set i [ftree_getFrameID $treeID $f]
    set p $__FTREE__parent($treeID,$i)

    # Can't delete root or a node with children
    if {$p == {}} {
	debug "    Cannot delete root node"
	_ftree_setError "Cannot delete root node"
	return -1
    }
    if {$__FTREE__children($treeID,$i) != {}} {
	debug "    Cannot delete internal nodes"
	_ftree_setError "Cannot delete internal nodes"
	return -1
    }

    set result [scan [$c bbox $i] "%d %d %d %d" cleft ctop cright cbot]
    set ccenter [expr ($cleft + $cright) / 2]
    set cwidth [expr $cright - $cleft]
    debug "    child bbox = $cleft $ctop $cright $cbot"

    set result [scan [$c bbox $p] "%d %d %d %d" pleft ptop pright pbot]
    set pcenter [expr ($pleft + $pright) / 2]
    set pwidth [expr $pright - $pleft]
    debug "    parent bbox = $pleft $ptop $pright $pbot"

    set rightsibs [_ftree_getRightSiblings $treeID $i]

    if {[llength $__FTREE__children($treeID,$p)] > 1} {
	_ftree_pushRightSiblings $treeID $i \
		[expr -1 * ($cwidth + (2 * $__FTREE__gap))]
    }

    $c delete $i
    $c delete Annotation$i

    delete_from_list __FTREE__children($treeID,$p) $i
    set allsibs $__FTREE__children($treeID,$p)
    debug "    new children of parent: $allsibs"

    _ftree_centerOverChildren $treeID $p

    unset __FTREE__parent($treeID,$i)
    unset __FTREE__children($treeID,$i)
    unset __FTREE__child_edges($treeID,$i)
    unset __FTREE__window_id($treeID,$f)

    return 0
}

proc ftree_getFrameID { treeID f } {
    global __FTREE__window_id
    set c [ftree_getCanvas $treeID]
    set result $__FTREE__window_id($treeID,$f)
    return $result
}

proc ftree_setHeight { treeID frame height} {

    global __FTREE__window_id
    global __FTREE__children

    set c [ftree_getCanvas $treeID]
    set id $__FTREE__window_id($treeID,$frame)

    set oldheight [$c itemcget $id -height]
    if {$oldheight == 0} {
	set oldheight [winfo reqheight $frame]
    }

    $c itemconfig $id -height $height

    set children $__FTREE__children($treeID,$id)
    foreach child $children {
	_ftree_moveTree $treeID $child 0 [expr $height - $oldheight]
    }

    _ftree_adjustChildEdges $treeID $id

}

proc ftree_resetScrollRegion { treeID {resizeToplevel 0} } {

    global __FTREE__gap

    debug "Re-setting scroll region for tree $treeID."
    
    set g $__FTREE__gap
    set c [ftree_getCanvas $treeID]

    scan [$c bbox all] "%d %d %d %d" x1 y1 x2 y2
    $c move all [expr $g - $x1] [expr $g - $y1]

    set newwidth [expr $x2 + (2 * $g) - $x1]
    set newheight [expr $y2 + (2 * $g) - $y1]
    set oldwidth [winfo width $c]
    set oldheight [winfo height $c]

    debug "Old: w=$oldwidth, h=$oldheight"
    debug "New: w=$newwidth, h=$newheight"

    $c config -scrollregion "0 0 $newwidth $newheight" \
	    -width $newwidth -height $newheight
    
    # Adjust the maxsize of the toplevel window, if the canvas has grown
#    if {$newwidth > $oldwidth || $newheight > $oldheight} {
#	set width [max $newwidth $oldwidth 200]
#	set height [max $newheight $oldheight 200]
#	$c config -scrollregion "0 0 $width $height" \
#		-width $width -height $height
#
#	update idletasks
#	set w [winfo toplevel $c]
#	wm maxsize $w [winfo reqwidth $w] [winfo reqheight $w]
#	if $resizeToplevel {
#	    wm geometry $w [winfo reqwidth $w]x[winfo reqheight $w]
#	}
#
#    }

}

proc _ftree_getRightSiblings { treeID i } {

    global __FTREE__parent
    global __FTREE__children

    set c [ftree_getCanvas $treeID]
    set p $__FTREE__parent($treeID,$i)
    if {$p == {}} return {}

    set children $__FTREE__children($treeID,$p)
    set index [lsearch -exact $children $i]
    set sibs [lrange $children [expr $index + 1] end]
    return $sibs
}

proc _ftree_pushRightSiblings { treeID i x } {
    global __FTREE__parent
    set c [ftree_getCanvas $treeID]
    debug "About to push right siblings of frame $i on canvas $c by $x"
    set s [_ftree_getRightSiblings $treeID $i]
    foreach sib $s {
	_ftree_moveTree $treeID $sib $x 0
    }
    set p $__FTREE__parent($treeID,$i)
    if {$p != {}} { _ftree_pushRightSiblings $treeID $p $x }
}

proc _ftree_centerOverChildren { treeID i } {

    global __FTREE__parent
    global __FTREE__children
    global __FTREE__child_edges

    set c [ftree_getCanvas $treeID]

    debug "Centering $i over its children"

    set children $__FTREE__children($treeID,$i)

    if {$children != {}} {
	set bbox [eval $c bbox $children]
	set left [lindex $bbox 0]
	set right [lindex $bbox 2]
	debug "    children=$c, bbox=$bbox, left=$left, right=$right"
	set old_center [lindex [$c coords $i] 0]
	set new_center [expr ($left + $right) / 2]
	set shift [expr $new_center - $old_center]
	_ftree_moveNode $treeID $i $shift 0
    }

    _ftree_adjustChildEdges $treeID $i
    set p $__FTREE__parent($treeID,$i)
    if {$p != {}} { _ftree_centerOverChildren $treeID $p }

}

proc _ftree_moveNode { treeID i x y } {
    set c [ftree_getCanvas $treeID]
    debug "Moving node $i on canvas $c by $x, $y"
    $c move $i $x $y
    $c move Annotation$i $x $y
}

proc _ftree_moveTree { treeID i x y } {

    global __FTREE__children

    set c [ftree_getCanvas $treeID]

    debug "Moving tree under frame $i on canvas $c by $x, $y"

    _ftree_moveNode $treeID $i $x $y

    set children $__FTREE__children($treeID,$i)
    if {$children == {}} { return }

    foreach child $children {
	_ftree_moveTree $treeID $child $x $y
    }

    _ftree_adjustChildEdges $treeID $i
}

proc _ftree_getDescendants { treeID i } {
    global __FTREE__children
    set c [ftree_getCanvas $treeID]
    set result $__FTREE__children($treeID,$i)
    foreach child $result {
	set new [_ftree_getDescendants $treeID $child]
	if {$new != {}} {
	    eval lappend result $new
	}
    }
    return $result
}

proc _ftree_adjustChildEdges { treeID i } {

    global __FTREE__child_edges
    global __FTREE__children

    set c [ftree_getCanvas $treeID]

    set kids $__FTREE__children($treeID,$i)
    set arcs $__FTREE__child_edges($treeID,$i)
    set parent_coords [$c coords $i]
    set n_kids [llength $kids]
    set n_arcs [llength $arcs]

    debug "Adjust child edges for frame $i on canvas $c"
    debug "    Parent coords=$parent_coords"
    debug "    Children=$kids"
    debug "    Child edges=$arcs"

    if {$n_kids < $n_arcs} {
	foreach arc [lrange $arcs $n_kids end] {
	    $c delete $arc
	}
	set arcs [lrange $arcs 0 [expr $n_kids - 1]]
	set __FTREE__child_edges($treeID,$i) $arcs
    }

    set count 0
    foreach child $kids {
	set arc_id [lindex $arcs $count]
	eval $c coords $arc_id $parent_coords \
		[$c coords $child]
	debug "    Arc $arc_id coords=[$c coords $arc_id]"
	incr count
    }
}

proc ftree_delete { oid } {
    debug "Deleting FTree $oid"
    foreach g [info globals __FTREE__*] {
	global $g
	if [array exists $g] {
	    foreach n [array names $g $oid] {
		debug "    un-setting ${g}($n)"
		eval unset ${g}($n)
	    }
	    foreach n [array names $g $oid,*] {
		debug "    un-setting ${g}($n)"
		eval unset ${g}($n)
	    }
	}
    }
}

proc _ftree_setError { msg } {
    global __FTREE__errmsg
    set __FTREE__errmsg $msg
}

proc ftree_getError {} {
    global __FTREE__errmsg
    return $__FTREE__errmsg
}

