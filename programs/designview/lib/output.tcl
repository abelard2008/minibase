# Andrew Prock, Spring 1996
#
# proc outputTables { entities relationships model } 
# proc outputRelationships { entities relationships model }
# proc outEntities { entities model } 
# proc outSQLEntity { relName attrList key } 
# proc outRelFdEntity { relName attrList key } 
# proc outSQLRelationship { relName attrList key fKey ref } 
# proc outRelFdRelationship { relName attrList key fKey ref } 

###########################################################################
#
###########################################################################
proc outputTables { entities relationships model } {
    upvar $entities ent
    upvar $relationships rel

    puts "###$model Output\n"

    if {$model == "RELFD" } {
	outRELFDheader ent rel
    }

    puts "#  Entities"
    outEntities ent $model

    puts "#  Relationships"
    outputRelationships ent rel $model
}

###########################################################################
#
###########################################################################
proc outRELFDheader { entities relationships } {
    upvar $entities ent
    upvar $relationships rel

    puts "% Database"
    puts -nonewline "Source Relations = "
    foreach R [array names ent] {
	puts -nonewline "$R "
#	puts -nonewline "[lindex $ent($R) 0] "
    }
    foreach R [array names rel] {
	puts -nonewline "$R "
#	puts -nonewline "[lindex $rel($R) 0] "
    }
    puts ""
    foreach R [array names ent] {
	foreach a $ent($R) {
	    if {[llength $a]==4} {
		puts "Attribute = [list [lindex $a 1] [lindex $a 2]]"
	    }
	}
    }
    foreach R [array names rel] {
	foreach a $rel($R) {
	    if {[llength $a]==4} {
		puts "Attribute = [list [lindex $a 1] [lindex $a 2]]"
	    }
	}
    }
    puts ""

}

###########################################################################
#
###########################################################################
proc outEntities { entities model } {
    upvar $entities ent

    foreach a [array names ent] {
	set key {}
	set fKey {}
	set attrList {}

	for {set i 1} {$i < [llength $ent($a)] } {incr i} {
	    set attrName [lindex [lindex $ent($a) $i] 1]
	    set type [lindex [lindex $ent($a) $i] 2]
	    lappend attrList [list $attrName $type]
	    set isKey [lindex [lindex $ent($a) $i] 3]
	    if {$isKey == "1"} {
		lappend key $attrName
	    }
	    if {[llength [lindex $ent($a) $i]] > 4} {
		lappend fKey [list [lindex [lindex $ent($a) $i] 1] \
			[lindex $ent([lindex [lindex $ent($a) $i] 4]) 0]]
	    }
	}

	if { $model == "SQL" } {
	    outSQLEntity [lindex $ent($a) 0] $attrList $key $fKey
	} elseif { $model == "RELFD" } {
	    outRelFdEntity [lindex $ent($a) 0] $a $attrList $key $fKey
	}
    }
}

###########################################################################
#
###########################################################################
proc outRelFdEntity { relName id attrList key fkey } {

    set attrs {}
    puts "% Relation"
    puts "Name = [join $relName]"
    puts "ID = $id"
    foreach a $attrList {
	puts "Attribute = [lindex $a 0]"
	lappend attrs [lindex $a 0]
    }
    set rhs [listDiff $attrs $key]
    puts "Key = $key"
    if {$key != {} && $rhs != {}} {
#	if {[llength $key]==1} {
#	    puts -nonewline "FD = $key "
#	} else {
	    puts -nonewline "FD = [list $key] "
#	}
#	if {[llength $rhs]==1} {
#	    puts "$rhs"
#	} else {
	    puts "[list $rhs]"
#	}
    }
    foreach a $fkey {
	#puts "Foreign Key = [lindex $a 0] REF [lindex $a 1]"

	#
	# Andy: Foreign Key
	#
	puts "Foreign Key = [list [lindex $a 0]] [list [lindex $a 1]]"

    }
    puts ""
}

###########################################################################
#
###########################################################################
proc outSQLEntity { relName attrList key fkey } {

    set commandList {}
    puts -nonewline "CREATE TABLE [join [join $relName] _] \(\n\t"

    if { $attrList != {} } {
	foreach a $attrList {
	    set command [list [join [lindex $a 0] _] [lindex $a 1] ]
	    lappend commandList [join $command "\t"]
	}
    }

    if { $key != {} } {
	foreach a $key {
	    set command [list [list PRIMARY KEY] [join $a _] ]
	    lappend commandList [join $command "\t"]
	}
    }

    if { $fkey != {} } {
	foreach a $fkey {
	    set command [list [list FOREIGN KEY] [join [lindex $a 0] _] \
		    REFERENCES [join [lindex $a 1]]]
	    lappend commandList [join $command "\t"]
	}
    }

    puts [join $commandList ",\n\t"]
    puts "\)"
    puts ""
}

###########################################################################
#
###########################################################################
proc outputRelationships { entities relationships model } {
    upvar $entities ent
    upvar $relationships rel

    foreach r [array names rel] {
	
	# step through attribute list
	set key {}
	set fkey {}
	set attrList {}

	for {set i 1} {$i < [llength $rel($r)] } {incr i} {
	    set attrName [lindex [lindex $rel($r) $i] 1]
	    set type [lindex [lindex $rel($r) $i] 2]
#	    lappend attrList $attrName
	    lappend attrList [list $attrName $type]

	    set isKey [lindex [lindex $rel($r) $i] 3]
	    if {$isKey == "1"} {
		lappend key $attrName
	    }
	    if {[llength [lindex $rel($r) $i]] > 4} {
		lappend fkey [list [lindex [lindex $rel($r) $i] 1] \
			[lindex $ent([lindex [lindex $rel($r) $i] 4]) 0]]
	    }
	}

	if { $model == "SQL" } {
	    outSQLRelationship [lindex $rel($r) 0] $attrList $key $fkey
	} elseif { $model == "RELFD" } {
	    outRelFdEntity [lindex $rel($r) 0] $r $attrList $key $fkey 
	}

    }
}

############################################################################
## Still needed? (used)
############################################################################
#proc outRelFdRelationship { relName id attrList key fKey } {
#
#    puts "% Relation"
#    puts "Name = [join $relName]"
#    puts "ID = $id"
#    foreach a [concat $attrList] {
#	if {$a != {}} {
#	    puts "Attribute = $a"
#	}
#    }
#    puts "Key = $key"
#    puts "FD = [list $key] "
#    if {$fKey!={} } {
#	foreach a $fKey {
#	    puts "Foreign Key = [lindex $a 0] REF [lindex $a 1]"
#	}
#    }
#    puts ""
#}

###########################################################################
#
###########################################################################
proc outSQLRelationship { relName attrList key fkey } {
    
    puts -nonewline "CREATE TABLE [join [join $relName] _] \(\n\t"

    if { $attrList != {} } {
	foreach a $attrList {
	    set command [list [join [lindex $a 0] _] [lindex $a 1] ]
	    lappend commandList [join $command "\t"]
	}
    }

    if { $key != {} } {
	foreach a [concat $key] {
	    set command [list [list PRIMARY KEY] [join $a _] ]
	    lappend commandList [join $command "\t"]
	}
    }

    if { $fkey != {} } {
	foreach a $fkey {
	    set command [list [list FOREIGN KEY] [join [lindex $a 0] _] \
		    REFERENCES [join [lindex $a 1]]]
	    lappend commandList [join $command "\t"]
	}
    }

    puts [join $commandList ",\n\t"]
    puts "\)"
    puts ""
}

###########################################################################
# computes the set difference of x - y
###########################################################################
proc listDiff { x y } {

    set result {}
    foreach a $x {
        if {[lsearch $y $a]==-1} {
            lappend result $a
        }
    }
    return $result
}



