
proc dbnDelete { oid } {

    global __REFCOUNT__

    debug "dbnDelete \"$oid\""

    if ![info exists __REFCOUNT__($oid)] {
	debug "    no prior references"
	return
    }
    
    incr __REFCOUNT__($oid) -1
    if {$__REFCOUNT__($oid) > 0} {
	debug "    $__REFCOUNT__($oid) reference(s) still exist"
	return
    }
    
    debug "    final reference being deleted..."

    unset __REFCOUNT__($oid)
    
    set t [oid2type $oid]
    if {$t == {}} { return }
    switch -exact -- $t {
	Session { session_delete $oid }
	Relation { relation_delete $oid }
	AttrSet { attrset_delete $oid }
	Decomp { decomp_delete $oid }
	default { puts "Error in dbnDelete: invalid type \"$t\"" }
    }

    debug "Done deleting $oid"

}

proc dbnSet { varname oid } {
    upvar $varname var
    debug "dbnSet: variable=$varname, oid=$oid"
    if [info exists var] { dbnDelete $var }
    set var $oid
    if {$oid != {}} { refcount_Incr $oid }
}

proc newOID { type } {
    global __REFCOUNT__
    global __LASTOID__
    if ![info exists __LASTOID__($type)] { set __LASTOID__($type) 0 }
    incr __LASTOID__($type)
    set newoid $type$__LASTOID__($type)
    return $newoid
}

proc refcount_Incr { oid } {
    global __REFCOUNT__
    debug "Incrementing reference count for oid \"$oid\""
    if {$oid == {}} { return }
    if ![info exists __REFCOUNT__($oid)] {
	set __REFCOUNT__($oid) 1
    } else {
	incr __REFCOUNT__($oid)
    }
}

proc getRefCount { oid } {
    global __REFCOUNT__
    if [info exists __REFCOUNT__($oid)] {
	return $__REFCOUNT__($oid)
    } else {
	return 0
    }
}

proc oid2type { oid } {
    if {[scan $oid {%[a-zA-Z]} result] == 0} { return {} }
    return $result
}

proc sameOID { oid1 oid2 } {
    if {$oid1 == {}} {
	puts "Error (sameOID): oid1 is null"
	return 0
    }
    if {$oid2 == {}} {
	puts "Error (sameOID): oid2 is null"
	return 0
    }
    if [string compare $oid1 $oid2] { return 0 }
    return 1
}

