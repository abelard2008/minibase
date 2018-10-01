
proc attrset_makeAttrSet { attrs } {
    global __ATTRSET__oid
    global __ATTRSET__attrList
    global __ATTRSET__relations
    debug "Creating a new AttrSet object. attrs=$attrs"
    set hashstring [attrset_hash $attrs]
    if [info exists __ATTRSET__oid($hashstring)] {
	return $__ATTRSET__oid($hashstring)
    }
    set oid [newOID AttrSet]
    debug "New OID = $oid"
    set __ATTRSET__oid($hashstring) $oid
    set __ATTRSET__attrList($oid) $hashstring
    set __ATTRSET__relations($oid) {}
    return $oid
}

proc attrset_getAttributes { oid } {
    global __ATTRSET__attrList
    if [info exists __ATTRSET__attrList($oid)] {
	return [split $__ATTRSET__attrList($oid) ,]
    }
    return {}
}

proc attrset_getRelations { attrs } {
    global __ATTRSET__relations
    global __ATTRSET__oid
    set hashstring [attrset_hash $attrs]
    if [info exists __ATTRSET__oid($hashstring)] {
	return $__ATTRSET__relations($__ATTRSET__oid($hashstring))
    }
    return {}
}

proc attrset_addRelation { oid relation } {
    global __ATTRSET__relations
    if ![info exists __ATTRSET__relations($oid)] {
	puts "Error (attrset_addRelation): invalid attrSet \"$oid\""
	return
    }
    lappend __ATTRSET__relations($oid) $relation
}

proc attrset_deleteRelation { oid relation } {
    global __ATTRSET__relations
    if ![info exists __ATTRSET__relations($oid)] {
	puts "Error (attrset_addRelation): invalid attrSet \"$oid\""
	return
    }
    delete_from_list __ATTRSET__relations($oid) $relation
}

proc attrset_hash { attrs } {
    return [join [lsort $attrs] ,]
}

proc attrset_delete { oid } {
    global __ATTRSET__oid
    global __ATTRSET__attrList
    global __ATTRSET__relations
    if [info exists __ATTRSET__attrList($oid)] {
	unset __ATTRSET__oid($__ATTRSET__attrList($oid))
	unset __ATTRSET__attrList($oid)
	unset __ATTRSET__relations($oid)
    }
}

