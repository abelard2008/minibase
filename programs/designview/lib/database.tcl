
proc db_setActiveRelation { r } {
    global __DB__activeRelation
    set __DB__activeRelation $r
}

proc db_getActiveRelation {} {
    global __DB__activeRelation
    if ![info exists __DB__activeRelation] { return {} }
    return $__DB__activeRelation
}

proc db_getAttributes {} {
    global __DB__attributes
    if ![info exists __DB__attributes] { return {} }
    return $__DB__attributes
}

proc db_addSourceRelation { relation } {

    global __DB__sourceRelations
    global __DB__attributes

    if ![info exists __DB__sourceRelations] {
	set __DB__sourceRelations {}
    }
    if ![info exists __DB__attributes] {
	set __DB__attributes {}
    }
    append_unique __DB__attributes [relation_getAttributes $relation]
    lappend __DB__sourceRelations $relation
    refcount_Incr $relation

}

proc db_getSourceRelations {} {
    global __DB__sourceRelations
    if ![info exists __DB__sourceRelations] {
	return {}
    }
    return $__DB__sourceRelations
}

proc db_isSourceRelation { relation } {
    global __DB__sourceRelations
    if ![info exists __DB__sourceRelations] {
	return {}
    }
    if {[lsearch $__DB__sourceRelations $relation] == -1} {
	return 0
    } else {
	return 1
    }
}

proc db_newSession { relation } {
    global __DB__sessionsOwned
    set session [session_makeSession $relation]
    if ![info exists __DB__sessionsOwned($relation)] {
	set __DB__sessionsOwned($relation) {}
	refcount_Incr $relation
    }
    lappend __DB__sessionsOwned($relation) $session
    refcount_Incr $session
    return $session
}

proc db_getSessions { relation } {
    global __DB__sessionsOwned
    if ![info exists __DB__sessionsOwned($relation)] { return {} }
    return $__DB__sessionsOwned($relation)
}

proc db_getSessionOwners {} {
    global __DB__sessionsOwned
    if ![array exists __DB__sessionsOwned] { return {} }
    return [array names __DB__sessionsOwned]
}

proc db_getDefaultSession { relation } {
    global __DB__sessionsOwned
    if ![info exists __DB__sessionsOwned($relation)] { return {} }
    return [lindex $__DB__sessionsOwned($relation) 0]
}

proc db_setDefaultSession { relation session } {

    global __DB__sessionsOwned
    if ![info exists __DB__sessionsOwned($relation)] { return {} }

    if {[lsearch $__DB__sessionsOwned($relation) $session] == -1} {
        return
    }

    delete_from_list __DB__sessionsOwned($relation) $session
    set __DB__sessionsOwned($relation) \
            [eval list $session $__DB__sessionsOwned($relation)]

}

proc db_deleteSession { relation session } {

    global __DB__sessionsOwned
    if ![info exists __DB__sessionsOwned($relation)] { return }

    delete_from_list __DB__sessionsOwned($relation) $session
    dbnDelete $session

    if {$__DB__sessionsOwned($relation) == {}} {
	dbnDelete $relation
	unset __DB__sessionsOwned($relation)
    }

}

proc db_makeDeepSession {} {

    set rid [relation_makeRelation Database [db_getAttributes]]
    set sid [session_makeSession $rid]
    session_setName $sid "Deep Decomposition"
    session_addChildren $sid $rid [db_getSourceRelations]
    
    set worklist {}
    foreach l [session_getLeaves $sid] {
	set default [db_getDefaultSession $l]
	if {$default != {}} {
	    lappend worklist $default
	}
    }

    while {$worklist != {}} {
	set s [lindex $worklist 0]
	set worklist [lreplace $worklist 0 0]
	set rlist [session_getAllRelations $s]
	if {[llength $rlist] > 1} {
	    foreach r [session_getAllRelations $s] {
		session_addChildren $sid $r [session_getChildren $s $r]
	    }
	    foreach l [session_getLeaves $s] {
		set default [db_getDefaultSession $l]
		if {$default != {}} {
		    lappend worklist $default
		}
	    }
	}
    }

    return $sid

}

proc db_deleteSourceRelation { relation } {

    global __DB__sessionsOwned
    global __DB__sourceRelations

    if ![info exists __DB__sourceRelations] { return }

    if {[lsearch $__DB__sourceRelations $relation] == -1} { return }

    if [info exists __DB__sessionsOwned($relation)] {
	foreach s $__DB__sessionsOwned($relation) {
	    db_deleteSession $relation $s
	}
    }

    delete_from_list __DB__sourceRelations $relation
    dbnDelete $relation

}

proc db_deleteAllRelations {} {

    global __DB__sourceRelations
    global __DB__sessionsOwned

    if ![info exists __DB__sourceRelations] { return }
    foreach r $__DB__sourceRelations {
	db_deleteSourceRelation $r
    }

    if [array exists __DB__sessionsOwned] {
	foreach r [array names __DB__sessionsOwned] {
	    foreach s $__DB__sessionsOwned($r) {
		db_deleteSession $r $s
	    }
	}
    }

    #
    # Clear all state variables
    #
    foreach var [info globals __DB__*] {
	global $var
	debug "    un-setting ${var}"
	eval unset ${var}
    }

}

proc db_setAttrType { attr type } {
    global __DB__attrType
    set __DB__attrType($attr) $type
}

proc db_getAttrType { attr } {
    global __DB__attrType
    if ![info exists __DB__attrType($attr)] { return <NoType> }
    return $__DB__attrType($attr)
}

proc db_attrClosure { attrs {relation {}} } {
    if {$relation == {}} {
	upvar #0 __DB__FDset F
    } else {
	set attrSet [relation_getAttrSet $relation]
	db_computeFDproj $attrSet	
	upvar #0 __DB__FDproj($attrSet) F
    }
    return [attrClosure F $attrs]
}

proc db_addFD { lhs rhs } {
    global __DB__dirtyFDset
    upvar #0 __DB__FDset F
    set result [addToFDset F $lhs $rhs]
    set __DB__dirtyFDset 1
    return $result
}

proc db_deleteFD { handle } {
    global __DB__dirtyFDset
    upvar #0 __DB__FDset F
    deleteFD F $handle
    set __DB__dirtyFDset 1
}

proc db_getFDHandles {} {
    upvar #0 __DB__FDset F
    if ![info exists F] { return {} }
    return [getFDlist F]
}

proc db_getFDlhs { handle } {
    upvar #0 __DB__FDset F
    if ![info exists F] { return {} }
    return [getFDlhs F $handle]
}

proc db_getFDrhs { handle } {
    upvar #0 __DB__FDset F
    if ![info exists F] { return {} }
    return [getFDrhs F $handle]
}

proc db_getFDText { handle {usecover 0} } {
    if $usecover {
	upvar #0 __DB__CCover F
    } else {
	upvar #0 __DB__FDset F
    }
    if ![info exists F] { return {} }
    set lhs [getFDlhs F $handle]
    set rhs [getFDrhs F $handle]
    return [concat [join $lhs ,] " -> " [join $rhs ,]]
}

proc db_getFDProjHandles { attrSet } {
    global __DB__FDset
    global __DB__FDproj
    if ![info exists __DB__FDset] { return {} }
    db_computeFDproj $attrSet
    return [getFDlist __DB__FDproj($attrSet)]
}

proc db_computeFDproj { attrSet } {

    global __DB__FDset
    global __DB__CCover
    global __DB__FDproj
    global __DB__dirtyFDset

    if ![info exists __DB__FDset] { return }

    if [info exists __DB__FDproj($attrSet)] {
	if !$__DB__dirtyFDset {
	    return
	}
    }

    debug "Computing FD projection. Attrs = [attrset_getAttributes $attrSet]"
    db_computeCanonicalCover
    projectFD __DB__CCover [db_getAttributes] \
	    [attrset_getAttributes $attrSet] __DB__FDproj($attrSet)
	
}

proc db_FDprojExists { attrSet } {
    global __DB__FDproj
    global __DB__dirtyFDset
    if [info exists __DB__FDproj($attrSet)] {
	if !$__DB__dirtyFDset { return 1 }
    }
    return 0
}

proc db_getFDprojText { attrSet handle } {
    upvar #0 __DB__FDproj($attrSet) F
    if ![info exists F] { return {} }
    set lhs [getFDlhs F $handle]
    set rhs [getFDrhs F $handle]
    return [concat [join $lhs ,] " -> " [join $rhs ,]]
}

proc db_getTextOfAllFDs { {attrSet {}} } {

    if {$attrSet != {}} {
	upvar #0 __DB__FDproj($attrSet) F
	db_computeFDproj $attrSet
	if ![info exists F] { return {} }
    } else {
	upvar #0 __DB__FDset F
    }

    set handles [getFDlist F]
    if {$handles == {}} {
	return {}
    }

    foreach h $handles {
	set lhs [getFDlhs F $h]
	set rhs [getFDrhs F $h]
	if [info exists a($lhs)] {
	    lappend a($lhs) $rhs
	} else {
	    set a($lhs) $rhs
	}
    }

    set result {}
    foreach i [array names a] {
	lappend result [concat [join $i ", "] " -> " [join $a($i) ", "]]
    }

    return $result

}

proc db_computeCanonicalCover {} {

    global __DB__dirtyFDset

    upvar #0 __DB__FDset F
    if ![info exists F] { return }

    upvar #0 __DB__CCover CC

    if ![info exists CC] {
	debug "Computing canonical cover for all FDs"
	canonicalCover F CC
    } else {
	if $__DB__dirtyFDset {
	    debug "Computing canonical cover for all FDs"
	    canonicalCover F CC
	}
    }

    set __DB__dirtyFDset 0
}

#
# Not used!
#
#proc db_displayFDset { minimal x y } {
#
#    set rname [relation_getName [db_getActiveRelation]]
#    set w .fdset_$minimal
#    set handles [db_getFDHandles]
#
#    if {$handles == {}} {
#	one_line_message .errmsg Error \
#		"No FDs are defined."
#	return
#    }
#
#    if $minimal {
#	db_computeCanonicalCover
#	upvar #0 __DB__CCover F
#	set handles [getFDlist F]
#    } else {
#	upvar #0 __DB__FDset F
#    }
#
#    if [winfo exists $w] {
#	set l $w.list_frame.list
#	if [catch {raise_toplevel $w}] { raise $w }
#    } else {
#	toplevel $w
#	if $minimal {
#	    wm title $w "Canonical Cover"
#	} else {
#	    wm title $w "All FDs"
#	}
#	wm geometry $w +$x+$y
#	set l [ScrolledListbox $w.list_frame 1 10 -selectmode single \
#		-setgrid true -width 40]
#	pack $w.list_frame -side top -anchor n
#    }
#
#    $l delete 0 end
#    foreach handle $handles {
#	set lhs [getFDlhs F $handle]
#	set rhs [getFDrhs F $handle]
#	$l insert end [concat [join $lhs ,] " -> " [join $rhs ,]]
#    }
#    $l config -height [min [llength $handles] 30]
#
#    if $minimal { unset F }
#
#}

proc db_testNormalForm { relation normalform } {

    global __DB__FDset
    global __DB__FDproj

    debug "Testing $normalform for relation $relation"

    if ![info exists __DB__FDset] {
	return "-1 : No FDs defined."
    }

    set attrSet [relation_getAttrSet $relation]
    if {$attrSet == {}} {
	return "-1 : Unknown relation: \"$relation\""
    }

    set attrs [attrset_getAttributes $attrSet]
    set rname [relation_getName $relation]

    db_computeFDproj $attrSet
    
    switch -exact -- $normalform {
	BCNF {
	    set result [testBCNF __DB__FDproj($attrSet) $attrs]
	}
	3NF {
	    set result [test3NF __DB__FDproj($attrSet) $attrs]
	}
	default {
	    return "-1 : Unknown normal form: $normalform"
	}
    }

    if {$result == "TRUE"} { return TRUE }
    if {[lindex $result 0] == -1} { return $result }

    set fdlist {}
    foreach handle $result {
	lappend fdlist [db_getFDprojText $attrSet $handle]
    }

    return $fdlist
}

proc db_generateNormalForm { relation normalform } {

    global __DB__FDset
    global __DB__FDproj

    debug "DB: Generating a $normalform decomposition of $relation"

    if ![info exists __DB__FDset] {
	return "-1 : No FDs defined."
    }

    set attrSet [relation_getAttrSet $relation]
    if {$attrSet == {}} {
	return "-1 : Unknown relation: \"$relation\""
    }
    set attrs [attrset_getAttributes $attrSet]

    debug "DB: attrSet=$attrSet, attrs=$attrs"

    db_computeFDproj $attrSet

    switch -exact -- $normalform {
	BCNF {
	    set result [decomposeBCNF __DB__FDproj($attrSet) $attrs]
	}
	3NF {
	    set result [decompose3NF __DB__FDproj($attrSet) $attrs]
	}
	default {
	    set result "-1 : Unknown normal form: $normalform"
	}
    }
    
    debug "DB: result = [join $result "\n             "]"
    return $result

}

proc db_isPrime { attrs {relation {}} } {

    if {$relation == {}} {
	upvar #0 __DB__FDset F
	set allAttrs [db_getAttributes]
    } else {
	set attrSet [relation_getAttrSet $relation]
	set allAttrs [attrset_getAttributes $attrSet]
	db_computeFDproj $attrSet	
	upvar #0 __DB__FDproj($attrSet) F
    }

    return [isPrime F $allAttrs $attrs]

}

proc db_isKey { attrs {relation {}} } {
    upvar #0 __DB__FDset F
    if ![info exists F] { return FALSE }
    if {$relation == {}} {
	set allAttrs [db_getAttributes]
    } else {
	set allAttrs [relation_getAttributes $relation]
    }
    return [isKey F $attrs $allAttrs]
}

proc db_testDecomposition { relation arrayname testname } {

    global __DB__FDproj
    global __DB__FDset
    upvar $arrayname decomparray

    debug "Testing $testname..."

    set attrSet [relation_getAttrSet $relation]
    if {$attrSet == {}} {
	return "-1 : Unknown relation: \"$relation\""
    }

    set attrs [attrset_getAttributes $attrSet]

    if ![info exists __DB__FDset] {
	return "-1 : No FDs defined."
    }

    set relationlist {}
    foreach r [array names decomparray] {
	debug "    r=$r, attrs=$decomparray($r)"
	lappend relationlist $decomparray($r)
    }

    switch -exact -- $testname {
        "Lossless Join" {
	    db_computeFDproj $attrSet
	    if {[llength $relationlist] > 2} {
		one_line_message .message Warning \
			"Warning: Non-binary lossless join test bypassed."
		return TRUE
	    }
	    set result [testLosslessJoin \
		    __DB__FDproj($attrSet) $relationlist]
	}
        "Dependency Preserving" {
	    db_computeFDproj $attrSet
	    set result [fdPreserving __DB__FDproj($attrSet) $relationlist]
	}
        default {
	    set result "-1 : Unknown decomposition test: $testname"
	}
    }

    return $result

}

proc db_getAllRelations {} {

    set worklist [db_getSourceRelations]

    while {$worklist != {}} {
	
	set curr [lindex $worklist 0]
	set worklist [lreplace $worklist 0 0]
	if [info exists flag($curr)] {
	    continue
	}
	
	set flag($curr) 1
	foreach s [db_getSessions $curr] {
	    eval lappend worklist [session_getAllRelations $s]
	}

    }

    return [array names flag]

}

proc db_getBaseRelations {} {

    set result {}

    set worklist {}
    foreach r [db_getSourceRelations] {
	lappend worklist [list $r {}]
    }

    while {$worklist != {}} {
	
	set currRelation [lindex [lindex $worklist 0] 0]
	set currSession [lindex [lindex $worklist 0] 1]
	
	set worklist [lreplace $worklist 0 0]
	if {[info exists visited($currRelation)]} {
	    continue
	}
	set visited($currRelation) 1

	if {$currSession == {}} {
	    set s [db_getDefaultSession $currRelation]
	} else {
	    set s [session_getDefaultDecomposition \
		    $currSession $currRelation]
	    if {$s == {}} {
		set s [db_getDefaultSession $currRelation]
	    }
	}
	
	if {$s == {}} {
	    lappend result $currRelation
	} else {
	    foreach leaf [session_getLeaves $s] {
		lappend worklist [list $leaf $s]
	    }
	}

    }

    return $result

}

proc db_confirmDecomposition { relation arrayname } {
    
    upvar $arrayname decomparray
    
    #
    # Make sure all attributes in the parent's primary key exist together
    # in at least one of the child relations.
    #
    set valid 0
    set parentKey [relation_getKey $relation]
    foreach name [array names decomparray] {
	if {[set_difference $parentKey $decomparray($name)] == {}} {
	    set valid 1
	    break
	}
    }
    if {$valid == 0} {
	set confirm [dialog .confirm "Split Key" \
		[list This decomposition will cause the parent \
		relation's primary key to be split, and could result \
		in the inability to enforce foreign key \
		constraints. Continue?] "" \
		0 {No} {Continue}]
	if !$confirm { return FALSE }
    }

    #
    # Make sure no child contains all of the parent's attributes
    #
    set valid 1
    set violator {}
    set parentAttrs [relation_getAttributes $relation]
    foreach name [array names decomparray] {
	if {[set_difference $parentAttrs $decomparray($name)] == {}} {
	    set valid 0
	    set violator $name
	    break
	}
    }
    if !$valid {
	one_line_message .message "Invalid Decomposition" \
		[list Invalid Decomposition: relation $violator \
		contains all of the parent relation's attributes.]
	return FALSE
    }

    #
    # See if this is a lossless join decomposition
    #
    set valid [db_testDecomposition $relation decomparray \
	    "Lossless Join"]
    if {$valid == "FALSE"} {
	set confirm [dialog .confirm "Not Lossless Join" \
		[list This is not a lossless join \
		decomposition. Continue?] "" \
		0 {No} {Continue}]
	if !$confirm { return FALSE }
    } elseif {$valid != "TRUE"} {
	return $valid
    }

    #
    # See if this is a dependency preserving decomposition
    #
    set valid [db_testDecomposition $relation decomparray \
	    "Dependency Preserving"]
    if {$valid == "FALSE"} {
	set confirm [dialog .confirm "Not Dependency Preserving" \
		[list This is not a dependency preserving \
		decomposition. Continue?] \
		"" 0 {No} {Continue}]
	if !$confirm { return FALSE }
    } elseif {$valid != "TRUE"} {
	return $valid
    }

    return TRUE

}

proc db_getForeignKeyReferences { attrs relation } {

    set allAttrs [relation_getAttributes $relation]
    if {[set_difference $attrs $allAttrs] != {}} {
	return {}
    }

    set session [db_getDefaultSession $relation]
    if {$session == {}} {
	return $relation
    }

    set result {}
    foreach l [session_getLeaves $session] {
	set newRefs [db_getForeignKeyReferences $attrs $l]
	if {$newRefs != {}} {
	    lappend result $newRefs
	}
    }

    return $result

}

