
set parse_inProgress 0

proc parse_file { f } {

    global __PARSE__oidMap
    global __PARSE__inProgress
    global __PARSE__lineNo
    global __PARSE__sessionOwner
    global __PARSE__fatal

    if [info exists __PARSE__oidMap] { unset __PARSE__oidMap }
    if [info exists __PARSE__lineNo] { unset __PARSE__lineNo }
    if [info exists __PARSE__sessionOwner] { unset __PARSE__sessionOwner }
    set __PARSE__fatal 0
    set __PARSE__inProgress 1

    parse_createRelations $f
    parse_createSessions $f
    parse_createDatabase $f

    if $__PARSE__fatal {
	db_deleteAllRelations
	set result FAIL
    } else {
	set result SUCCESS
    }

    parse_cleanup

    return $result

}

proc parse_getLine { f varname } {

    global __PARSE__lineNo
    upvar $varname line

    if ![info exists __PARSE__lineNo] { set __PARSE__lineNo 0 }

    while 1 {

	#
	# Clear buffer
	#
	set line {}

	#
	# Ignore comments and blank lines
	#
	parse_skipCommentsAndWhitespace $f

	#
	# Exit loop if EOF
	#
	if [eof $f] { break }

	#
	# Read next input line
	#
	incr __PARSE__lineNo
	gets $f line

	break

    }

}

proc parse_skipCommentsAndWhitespace { f } {
    global __PARSE__lineNo
    skipws $f
    while {[peek $f] == "#"} {
	incr __PARSE__lineNo
	gets $f line
	if [eof $f] { return }
	skipws $f
    }
}

proc parse_seekToNextObject { f {type {}} } {

    debug "Seeking for next object (type=$type)"

    set line {}

    while {[string range [lindex $line 0] 0 0] != "%"} {
	parse_getLine $f line
	if ![llength $line] { return {} }
    }
    
    if {[llength $line] < 2} { return [parse_seekToNextObject $f $type] }

    set objectType [lindex $line 1]

    if {$objectType != {}} {
	if {[string compare $type $objectType]} {
	    return [parse_seekToNextObject $f $type]
	}
    }

    return $type

}

proc parse_seekToStart { f } {
    global __PARSE__lineNo
    debug "Seeking to start of file $f"
    set __PARSE__lineNo 0
    seek $f 0 start
}

proc parse_createDatabase { f } {

    global __PARSE__lineNo
    global __PARSE__fatal

    debug "Creating database object"

    if $__PARSE__fatal { return }

    parse_seekToStart $f

    #
    # Find the database object in the input file
    #
    if {[parse_seekToNextObject $f Database] == {}} {
	puts stderr "ERROR: No database object defined"
	set __PARSE__fatal 1
	return
    }

    #
    # Initialize local variables which store the state of the current
    # relation
    #
    set sourceRelations {}

    #
    # Read every line until eof or a line that begins with "%"
    #
    while 1 {

	parse_skipCommentsAndWhitespace $f
	if {[peek $f] == "%"} { break }

	parse_getLine $f line
	if [eof $f] { break }
	
	#
	# The line should be of the form PROPERTY=VALUE. The following
	# procedure sets local variables (property and value) to values
	# found in the input line. Ignore the current object if the
	# input line cannot be parsed.
	#
	if ![parse_getPropertyAndValue $line property value] {
	    continue
	}

	#
	# Store the object properties in local variables
	#
	switch -- [string toupper $property] {
	    "SOURCERELATIONS" { eval lappend sourceRelations $value }
	    "ATTRIBUTE" {
		if {[llength $value] != 2} {
		    parse_printIgnoreWarning "No type specified"
		} else {
		    #
		    # ** REMOVE SPACES **
		    #
		    db_setAttrType [join [lindex $value 0] _] \
			    [lindex $value 1]
		}
	    }
	    "FD" {
		if {[llength $value] != 2} {
		    parse_printIgnoreWarning "Invalid FD declaration"
		} else {
		    #
		    # ** REMOVE SPACES **
		    #
		    set lhs {}
		    set rhs {}
		    foreach a [lindex $value 0] { lappend lhs [join $a _] }
		    foreach a [lindex $value 1] { lappend rhs [join $a _] }
		    db_addFD $lhs $rhs
		}
	    }
	    default { parse_printIgnoreWarning "Unknown Database property" }
	}
	
    }
    
    #
    # Now the entire object has been read.
    #
    
    if {$sourceRelations == {}} {
	puts stderr [list ERROR: no source relations defined]
	set __PARSE__fatal 1
	return
    }
    
    #
    # Convert the source relation OIDs to internal OIDs. Also add
    # FDs for Key dependencies, if the dependency is not redundant.
    # 
    foreach r $sourceRelations {
	set internalOID [parse_getInternalOID $r]
	if {$internalOID == {}} {
	    puts stderr "WARNING: Undefined source relation: $r"
	} else {
	    db_addSourceRelation $internalOID
	    set k [relation_getKey $internalOID]
	    set attrs [relation_getAttributes $internalOID]
	    if {$k != {}} {
		if {[db_isKey $k $internalOID] == "FALSE"} {
		    db_addFD $k [set_difference $attrs $k]
		}
	    }
	}
    }

    # 
    # Now go ahead and compute FD projection for each source relation.
    # 
    foreach r [db_getSourceRelations] {
	db_computeFDproj [relation_getAttrSet $r]
    }

}

proc parse_createRelations { f } {

    global __PARSE__lineNo
    global __PARSE__fatal

    debug "Creating relations"

    if $__PARSE__fatal { return }

    parse_seekToStart $f

    #
    # Loop once for every relation object in the input file
    #
    while 1 {

	if {[parse_seekToNextObject $f Relation] == {}} { break }

	#
	# Initialize local variables which store the state of the current
	# relation
	#
	set oid {}
	set name {}
	set attrs {}
	set key {}
	set foreignKey {}
	set sessions {}

	#
	# Read every line until eof or a line that begins with "%"
	#
	while 1 {

	    parse_skipCommentsAndWhitespace $f
	    if {[peek $f] == "%"} { break }

	    parse_getLine $f line
	    if [eof $f] { break }

	    #
	    # The line should be of the form PROPERTY=VALUE. The following
	    # procedure sets local variables (property and value) to values
	    # found in the input line. Ignore the current object if the
	    # input line cannot be parsed.
	    #
	    if ![parse_getPropertyAndValue $line property value] {
		continue
	    }

	    #
	    # Store the object properties in local variables
	    #
	    switch -- [string toupper $property] {
		"ID"         { set oid $value }
		"NAME"       { set name $value }
		"ATTRIBUTES" { eval lappend attrs $value }
		"ATTRIBUTE"  { lappend attrs $value }
		"KEY"        { set key $value }
		"FOREIGNKEY" { lappend foreignKey $value }
		"SESSIONS"   { set sessions $value }
		"FD" {
		    if {[llength $value] != 2} {
			parse_printIgnoreWarning "Invalid FD declaration"
		    } else {
			#
			# ** REMOVE SPACES **
			#
			set lhs {}
			set rhs {}
			foreach a [lindex $value 0] {
			    lappend lhs [join $a _]
			}
			foreach a [lindex $value 1] {
			    lappend rhs [join $a _]
			}
			db_addFD $lhs $rhs
		    }
		}
		default {
		    parse_printIgnoreWarning "Unknown relation property"
		}
	    }

	}

	#
	# Now the entire object has been read.
	#

	#
	# Remove spaces from relation name
	#
	set name [join $name _]

	#
	# Remove spaces from attribute names in attribute list and key
	#
	# ** REMOVE SPACES **
	#
	set temp1 $attrs
	set temp2 $key
	set temp3 $foreignKey
	set attrs {}
	set key {}
	set foreignKey {}
	foreach a $temp1 { lappend attrs [join $a _] }
	foreach a $temp2 { lappend key [join $a _] }
	foreach a $temp3 {
	    lappend foreignKey [list [join [lindex $a 0] _] [lindex $a 1]]
	}

	if {$oid == {}} {
	    puts stderr [list WARNING (line $__PARSE__lineNo): \
		    Previous relation has no OID and will be ignored.]
	    continue
	}
	if {$name == {}} {
	    puts stderr [list WARNING: Relation $oid has no name \
		    and will be ignored.]
	    continue
	}
	if {$attrs == {}} {
	    puts stderr [list WARNING: Relation $oid has an empty \
		    attribute list and will be ignored.]
	    continue
	}

	#
	# Create a relation object, and create the mapping from the file OID
	# to an internal OID.
	#
	set internalOID [relation_makeRelation $name $attrs $key $foreignKey]
	if ![parse_setInternalOID $oid $internalOID] {
	    puts stderr "WARNING: Relation $oid will be ignored."
	    continue
	}

	#
	# Create session objects, and the appropriate OID mappings.
	#
	foreach s $sessions {
	    set sid [db_newSession $internalOID]
	    parse_setInternalOID $s $sid
	    parse_setSessionOwner $s $internalOID
	}

    }

}

proc parse_createSessions { f } {

    global __PARSE__oidMap
    global __PARSE__lineNo
    global __PARSE__sessionOwner
    global __PARSE__fatal

    debug "Creating sessions"

    if $__PARSE__fatal { return }

    #
    # Go to the beginning of the input file
    #
    parse_seekToStart $f

    #
    # This loop will execute once for every session object in the file
    #
    while 1 {

	if {[parse_seekToNextObject $f Session] == {}} { break }

	set name {}
	set oid {}
	if [info exists decomp] { unset decomp }
	if [info exists decomp2] { unset decomp2 }
	if [info exists link] { unset link }
	if [info exists link2] { unset link2 }

	#
	# Read every line until you hit eof or a line that starts with "%"
	#
	while 1 {
	    
	    parse_skipCommentsAndWhitespace $f
	    if {[peek $f] == "%"} { break }

	    parse_getLine $f line
	    if [eof $f] { break }
	    
	    #
	    # The line should be of the form PROPERTY=VALUE. The following
	    # procedure sets local variables (property and value) to values
	    # found in the input line. Ignore the current session if the
	    # input line cannot be parsed.
	    #
	    if ![parse_getPropertyAndValue $line property value] { continue }

	    #
	    # Store the session name and all decompositions
	    #
	    switch -- [string toupper $property] {
		"ID" { set oid $value }
		"NAME" { set name $value }
		"DECOMPOSITION" {
		    if {[llength $value] != 2} {
			parse_printIgnoreWarning "Invalid decomposition"
		    } else {
			set decomp([lindex $value 0]) [lindex $value 1]
		    }
		}
		"LINKEDSESSION" {
		    if {[llength $value] != 2} {
			parse_printIgnoreWarning "Invalid session link"
		    } else {
			set link([lindex $value 0]) [lindex $value 1]
		    }
		}
		default { parse_printIgnoreWarning \
			      "Invalid session property" }
	    }

	}
	
	#
	# Now the entire object has been read. An uninitialized session was
	# created when the sessions owner was parsed, so it's time to add
	# "children" to that session. The tricks are to convert all file OIDs
	# to internal OIDs first, and never to add children to a relation
	# that is not yet part of the session.
	#

	#
	# First make sure that an OID was specified
	#
	if {$oid == {}} {
	    puts stderr [list WARNING (line $__PARSE__lineNo): \
		    Previous session has no OID and will be ignored.]
	    continue
	}

	#
	# Get the internal OIDs for this session and its owner
	# Ignore the current session if there is an error.
	#
	set internalOID [parse_getInternalOID $oid]
	if {$internalOID == {}} {
	    puts stderr "WARNING: Session $oid will be ignored"
	    continue
	}
	set owner [parse_getSessionOwner $oid]
	if {$owner == {}} {
	    puts stderr "WARNING: Session $oid will be ignored"
	    continue
	}

	if {$name != {}} { session_setName $internalOID $name }

	#
	# Now it's time to add decompositions to the session object
	#
	
	#
	# The variable valid will be used to flag errors occurring inside
	# the following loops. No loop should execute if valid == 0.
	#
	set valid 1
	
	if [info exists decomp] {

	    #
	    # Loop once for each decomposition defined in the input file
	    #
	    foreach parent [array names decomp] {

		if !$valid { break }

		#
		# Convert the "parent" OID to an internal OID
		#
		set parent2 [parse_getInternalOID $parent]
		if {$parent2 == {}} {
		    set valid 0
		    continue
		}
		
		#
		# Convert all child OIDs
		#
		set decomp2($parent2) {}
		foreach child $decomp($parent) {
		    if !$valid { break }
		    set child2 [parse_getInternalOID $child]
		    if {$child2 == {}} {
			set valid 0
			continue
		    }
		    lappend decomp2($parent2) $child2
		}
		
	    }
	    
	}

	#
	# Now setup the linked sessions
	#
	if [info exists link] {

	    set valid 1
	    
	    #
	    # Loop once for each decomposition defined in the input file
	    #
	    foreach r [array names link] {

		if !$valid { break }

		#
		# Convert the OID to an internal OID
		#
		set r2 [parse_getInternalOID $r]
		if {$r2 == {}} {
		    set valid 0
		    continue
		}
		
		#
		# Convert session OID
		#
		set s2 [parse_getInternalOID $link($r)]
		if {$s2 == {}} {
		    set valid 0
		    continue
		}
		set link2($r2) $s2
		
	    }
	    
	}

	if !$valid {
	    puts stderr [list WARNING: session $oid \
		    will be ignored. (line $__PARSE__lineNo).]
	    continue
	}

	#
	# Starting with the root relation, add children to the new session.
	# The worklist represents relations that have been added to the
	# session but have no children yet.
	#
	set worklist $owner
	while {$worklist != {}} {
	    set head [lindex $worklist 0]
	    set worklist [lreplace $worklist 0 0]
	    if [info exists decomp2($head)] {
		set children $decomp2($head)
		eval lappend worklist [session_addChildren \
			$internalOID $head $children]
	    }
	    if [info exists link2($head)] {
		session_setDefaultDecomposition $internalOID \
			$head $link2($head)
	    }
	}

    }

}

proc parse_printIgnoreWarning { text } {
    global __PARSE__lineNo
    puts stderr \
	    "WARNING (line $__PARSE__lineNo): $text. Line will be ignored."
}

proc parse_getPropertyAndValue { line propertyvar valuevar } {

    upvar $propertyvar property
    upvar $valuevar value

    #
    # Believe it or not, the following regular expression
    # matches a line of the form ATTRIBUTE = VALUE, where
    # whitespace is insignificant and VALUE is optional. 
    # After the pattern match, the characters before "=" 
    # are stored in attr, and the characters after "=" are 
    # stored in val.
    #
    set valid [regexp "^\[ \t\]*(\[^=\]+)\[ \t\]*=\[ \t\]*(.*)" \
	    $line match property value]
    
    if !$valid {
	if {$line != {}} {
	    parse_printIgnoreWarning "Invalid property specification"
	}
	return 0
    }

    set property [join $property {}]
    return 1

}

proc parse_getSessionOwner { session } {
    global __PARSE__sessionOwner
    global __PARSE__lineNo
    if ![info exists __PARSE__sessionOwner($session)] {
	puts stderr [list WARNING: Session $session is not owned by \
		any relation (line $__PARSE__lineNo).]
	return {}
    }
    return $__PARSE__sessionOwner($session)
}

proc parse_setSessionOwner { session relation } {
    global __PARSE__sessionOwner
    global __PARSE__lineNo
    if [info exists __PARSE__sessionOwner($session)] {
	puts stderr [list WARNING: Session $session is owned by more \
		than one relation (line $__PARSE__lineNo).]
	return 0
    }
    set __PARSE__sessionOwner($session) $relation
    return 1
}

proc parse_getInternalOID { fileOID } {
    global __PARSE__oidMap
    global __PARSE__lineNo
    if ![info exists __PARSE__oidMap($fileOID)] {
	puts stderr [list WARNING: invalid object reference: $fileOID \
		(line $__PARSE__lineNo)]
	return {}
    }
    return $__PARSE__oidMap($fileOID)
}

proc parse_setInternalOID { fileOID internalOID } {

    global __PARSE__oidMap
    global __PARSE__lineNo

    debug "OID mapping: $fileOID -> $internalOID"

    if [info exists __PARSE__oidMap($fileOID)] {
	puts stderr [list WARNING: duplicate OID detected: $fileOID \
		(line $__PARSE__lineNo)]
	return 0
    }

    set __PARSE__oidMap($fileOID) $internalOID
    return 1
}

proc parse_cleanup {} {

    debug "Deleting all parse module global variables"

    foreach v [info globals __PARSE__*] {
	global $v
	debug "    un-setting $v"
	unset $v
    }

    set __PARSE__inProgress 0

}

proc parse_inProgress {} {
    global __PARSE__inProgress
    if ![info exists __PARSE__inProgress] {
	return 0
    }
    return $__PARSE__inProgress
}

