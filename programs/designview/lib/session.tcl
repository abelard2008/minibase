
proc session_makeSession { relation } {
    global __SESSION__root
    global __SESSION__children
    global __SESSION__parent
    global __SESSION__name
    global __SESSION__defaultDecomp
    debug "Creating a new session object. Root relation is $relation"
    set oid [newOID Session]
    debug "New session OID: $oid"
    dbnSet __SESSION__root($oid) $relation
    set __SESSION__children($oid,$relation) {}
    set __SESSION__parent($oid,$relation) {}
    set __SESSION__defaultDecomp($oid,$relation) {}
    set __SESSION__name($oid) $oid
    return $oid
}

proc session_addChildren { oid relation children } {

    global __SESSION__children
    global __SESSION__parent
    global __SESSION__defaultDecomp

    debug "Adding children to relation $relation in session $oid"
    debug "Children: $children"
    
    if ![info exists __SESSION__children($oid,$relation)] {
	puts "Error (session_addChildren): invalid session \"$oid\""
	return
    }

    foreach child $children {
	if [info exists __SESSION__children($oid,$child)] {
	    one_line_message .message "Error" \
		    [list A cycle was detected while adding children \
		    to relation '[relation_getName $relation]' in session \
		    '[session_getName $oid]'. The child completing the cycle \
		    was relation '[relation_getName $relation]' and this \
		    child was not added to the session]
	} else {
	    lappend __SESSION__children($oid,$relation) $child
	    refcount_Incr $child
	    set __SESSION__children($oid,$child) {}
	    set __SESSION__parent($oid,$child) $relation
	    set __SESSION__defaultDecomp($oid,$child) {}
	}
    }
    
    return $__SESSION__children($oid,$relation)

}

proc session_deleteChildren { oid relation } {

    global __SESSION__children
    global __SESSION__parent
    global __SESSION__defaultDecomp

    if ![info exists __SESSION__children($oid,$relation)] {
	puts "Error (session_deleteChildren): invalid session \"$oid\""
	return
    }

    foreach child $__SESSION__children($oid,$relation) {
	session_deleteChildren $oid $child
	dbnDelete $child
	unset __SESSION__children($oid,$child)
	unset __SESSION__parent($oid,$child)
	unset __SESSION__defaultDecomp($oid,$child)
    }

    set __SESSION__children($oid,$relation) {}

}

proc session_deleteChild { oid parent child } {

    global __SESSION__children
    global __SESSION__parent
    global __SESSION__defaultDecomp

    if ![info exists __SESSION__children($oid,$parent)] {
	puts "Error (session_deleteChild): no children for $oid,$parent
	return
    }

    if {[lsearch -exact $__SESSION__children($oid,$parent) $child] == -1} {
	puts "Error (session_deleteChild): invalid child \"$child\""
	return
    }

    session_deleteChildren $oid $child
    dbnDelete $child
    unset __SESSION__children($oid,$child)
    unset __SESSION__parent($oid,$child)
    unset __SESSION__defaultDecomp($oid,$child)

    delete_from_list __SESSION__children($oid,$parent) $child

}

proc session_makeCopy { session1 } {
    set session2 [session_makeSession [session_getRoot $session1]]
    session_setName $session2 [session_getName $session1]
    session_copyData $session1 $session2
    return $session2
}

proc session_copyData { source dest } {

    debug "Copying session data from $source to $dest..."

    set root1 [session_getRoot $source]
    set root2 [session_getRoot $dest]

    if ![sameOID $root1 $root2] {
	puts "Error (session_copyData): different roots."
	return
    }

    debug "    deleting all descendants of root in $dest"
    session_deleteChildren $dest $root2

    set worklist $root2
    while {$worklist != {}} {
	set head [lindex $worklist 0]
	set worklist [lreplace $worklist 0 0]
	set children [session_getChildren $source $head]
	eval lappend worklist $children
	session_addChildren $dest $head $children
	session_setDefaultDecomposition $dest $head \
		[session_getDefaultDecomposition $source $head]
    }

}

proc session_setDefaultDecomposition { oid relation otherSession } {
    
    global __SESSION__defaultDecomp
    
    debug "Session $oid: setting default decomp of $relation to $otherSession"

    if ![info exists __SESSION__defaultDecomp($oid,$relation)] {
	puts "Error (session_setDefaultDecomp): $relation not member of $oid"
	return 0
    }

    if { $otherSession != {} && \
	    ![sameOID $relation [session_getRoot $otherSession]] } {
	
	set rname [relation_getName $relation]
	set sname [session_getName $otherSession]

	one_line_message .message "Error" \
		[list ERROR: Could not set the default decomposition of \
		relation '$rname' to session '$sname' because '$rname' \
		is not the root relation of '$sname'.]
	
	return 0
    
    }
    
    set __SESSION__defaultDecomp($oid,$relation) $otherSession
    return 1

}

proc session_getDefaultDecomposition { oid relation } {
    
    global __SESSION__defaultDecomp
    
    debug "Session $oid: looking up default decomp of $relation"

    if ![info exists __SESSION__defaultDecomp($oid,$relation)] {
	puts "Error (session_getDefaultDecomp): $relation not member of $oid"
	return {}
    }
    
    set result $__SESSION__defaultDecomp($oid,$relation)

    if {$result != {} && ![session_exists $result]} {
	set __SESSION__defaultDecomp($oid,$relation) {}
	return {}
    }

    return $result

}

proc session_exists { oid } {
    global __SESSION__root
    return [info exists __SESSION__root($oid)]
}

proc session_getChildren { oid relation } {
    global __SESSION__children
    debug "Session $oid: looking up children of $relation"
    if ![info exists __SESSION__children($oid,$relation)] {
	puts "Error (session_getChildren): no children for $oid,$relation"
	return
    }
    debug "    children=$__SESSION__children($oid,$relation)"
    return $__SESSION__children($oid,$relation)
}

proc session_getRoot { oid } {
    global __SESSION__root
    if ![info exists __SESSION__root($oid)] {
	puts "Error (session_getRoot): invalid session \"$oid\""
	return {}
    }
    return $__SESSION__root($oid)
}

proc session_getName { oid } {
    global __SESSION__name
    if ![info exists __SESSION__name($oid)] {
	puts "Error (session_getName): invalid session \"$oid\""
	return {}
    }
    return $__SESSION__name($oid)
}

proc session_setName { oid name } {
    global __SESSION__name
    if ![info exists __SESSION__name($oid)] {
	puts "Error (session_getName): invalid session \"$oid\""
	return {}
    }
    set __SESSION__name($oid) $name
}

proc session_getAllRelations { oid } {
    set root [session_getRoot $oid]
    return [concat $root [session_getAllDescendants $oid $root]]
}

proc session_getLeaves { oid } {
    set root [session_getRoot $oid]
    set leaves [session_getLeafDescendants $oid $root]
    if {$leaves == {}} {
	return $root
    } else {
	return $leaves
    }
}

proc session_getAllDescendants { oid relation } {
    set children [session_getChildren $oid $relation]
    if {$children == {}} { return {} }
    set result $children
    foreach child $children {
	eval lappend result [session_getAllDescendants $oid $child]
    }
    return $result
}

proc session_getLeafDescendants { oid relation } {
    set result {}
    foreach child [session_getChildren $oid $relation] {
	set leaves [session_getLeafDescendants $oid $child]
	if {$leaves == {}} {
	    lappend result $child
	} else {
	    eval lappend result [session_getLeafDescendants $oid $child]
	}
    }
    return $result
}

proc session_getParent { session relation } {
    global __SESSION__parent
    if ![info exists __SESSION__parent($session,$relation)] {
	puts "Error (session_getParent): no parent for $session,$relation"
	return
    }
    return $__SESSION__parent($session,$relation)
}

proc session_getAllSessions {} {
    global __SESSION__root
    if ![array exists __SESSION__root] { return {} }
    return [array names __SESSION__root]
}

proc session_delete { oid } {

    debug "Deleting session $oid"

    set root [session_getRoot $oid]
    session_deleteChildren $oid $root
    dbnDelete $root

    # Clear all state variables
    foreach array [info globals __SESSION__*] {
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

