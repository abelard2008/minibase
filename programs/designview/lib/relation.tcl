
proc relation_makeRelation { name attrs {key {}} {foreignKey {}} } {

    global __RELATION__name
    global __RELATION__attrSet
    global __RELATION__key
    global __RELATION__foreignKey
    
    set confirm [relation_checkForIdenticalRelations $attrs $name]
    if {[lindex $confirm 0] == "UseExistingRelation"} {
	return [lindex $confirm 1]
    }

    set oid [newOID Relation]
    set attrSet [attrset_makeAttrSet $attrs]
    attrset_addRelation $attrSet $oid

    set __RELATION__name($oid) $name
    set __RELATION__key($oid) $key
    set __RELATION__foreignKey($oid) $foreignKey
    dbnSet __RELATION__attrSet($oid) $attrSet

    return $oid
}

proc relation_getName { oid } {
    global __RELATION__name
    if ![info exists __RELATION__name($oid)] {
	puts "Error (relation_getName): invalid relation \"$oid\""
	return {}
    }
    return $__RELATION__name($oid)
}

proc relation_name2oid { rname } {
    global __RELATION__name
    set result {}
    foreach oid [array names __RELATION__name] {
	if {"$__RELATION__name($oid)" == "$rname"} {
	    lappend result $oid
	}
    }
    return $result
}

proc relation_setName { oid newname } {
    global __RELATION__name
    set __RELATION__name($oid) $newname
}

proc relation_getAttributes { oid } {
    global __RELATION__attrSet
    if ![info exists __RELATION__attrSet($oid)] {
	puts "Error (relation_getAttributes): invalid relation \"$oid\""
	return {}
    }
    return [attrset_getAttributes $__RELATION__attrSet($oid)]
}

proc relation_getKey { oid } {
    global __RELATION__key
    if ![info exists __RELATION__key($oid)] {
	puts "Error (relation_getKey): invalid relation \"$oid\""
	return {}
    }
    return $__RELATION__key($oid)
}

proc relation_setKey { oid keyAttrs } {
    global __RELATION__key
    set __RELATION__key($oid) $keyAttrs
}

proc relation_getForeignKey { oid } {
    global __RELATION__foreignKey
    if ![info exists __RELATION__foreignKey($oid)] {
	puts "Error (relation_getForeignKey): invalid relation \"$oid\""
	return {}
    }
    return $__RELATION__foreignKey($oid)
}

proc relation_setForeignKey { oid fkey } {
    global __RELATION__foreignKey
    set __RELATION__foreignKey($oid) $fkey
}

proc relation_getAttrSet { oid } {
    global __RELATION__attrSet
    if ![info exists __RELATION__attrSet($oid)] {
	puts "Error (relation_getAttributes): invalid relation \"$oid\""
	return {}
    }
    return $__RELATION__attrSet($oid)
}

proc relation_getSQL { oid {deep 0} } {

    set sql "CREATE TABLE [relation_getName $oid] (\n"

    set attrs [relation_getAttributes $oid]
    set nAttrs [llength $attrs]
    set key [relation_getKey $oid]
    set fkey [relation_getForeignKey $oid]

    set count 0

    foreach a $attrs {
	if !$count {
	    set sql "${sql}    $a [db_getAttrType $a]"
	} else {
	    set sql "${sql},\n    $a [db_getAttrType $a]"
	}
	incr count
    }

    if {$key != {}} {
	set sql "${sql},\n    PRIMARY KEY ([join $key {, }])"
    }

    foreach fk $fkey {

	set attrs [lindex $fk 0]
	set other [lindex $fk 1]
	set attrText [join $attrs ", "]

	# Temporary fix: o2r puts names in FK, not OIDs
	set other [lindex [relation_name2oid $other] 0]
	if {$other == {}} { continue }

	if $deep {
	    set other [db_getForeignKeyReferences $attrs $other]
	}

	foreach r $other {
	    set sql "${sql},\n    FOREIGN KEY ($attrText)"
	    set sql "${sql} REFERENCES [relation_getName $r] ($attrText)"
	}

    }

    set sql "${sql}\n);\n"

    return $sql

}

proc relation_delete { oid } {

    global __RELATION__attrSet

    debug "Deleting relation $oid"

    #
    # Decrement reference count for the attribute set
    #
    if [info exists __RELATION__attrSet($oid)] {
	attrset_deleteRelation $__RELATION__attrSet($oid) $oid
	dbnDelete $__RELATION__attrSet($oid)
    }

    #
    # Clear all state variables
    #
    foreach array [info globals __RELATION__*] {
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

proc relation_checkForIdenticalRelations { attrs newName } {
    
    global __RELATION__choice
    
    set otherOIDs [attrset_getRelations $attrs]
    if {$otherOIDs == {}} { return CreateNewRelation }
    
    set confirm [dialog .confirm "Identical Relation Detected" \
	    [list While creating the '$newName' relation, existing relations \
	    with identical attribute sets were found. Do you want to \
	    replace '$newName' with one of the existing relations, or \
	    create a new relation named '$newName'?] \
	    "" 0 {Replace With an Existing Relation} {Create a New Relation}]
    if {$confirm == 1} { return CreateNewRelation }
    
    if {[llength $otherOIDs] == 1} {
	return [list UseExistingRelation [lindex $otherOIDs 0]]
    }
    
    set otherNames {}
    foreach oid $otherOIDs {
	lappend otherNames [relation_getName $oid]
    }
    
    set choice {}
    while {$choice == {}} {
	set choice [SelectionBox \
		[list Select a relation to replace '$newName':] $otherNames]
	if {$choice == {}} {
	    one_line_message .message "Error" \
		    [list No relation was selected.]
	}
	
    }
	
    return [list UseExistingRelation [lindex $otherOIDs $choice]]
    
}

