# Andrew Prock, Spring 1996

# Wierdness
#	attributes cannont be named "isa" or "foreign"
#	all attribute names must be unique
#
# Due to miscommunication the parser supporst both the .DATASCHEMA
#	file types AND the files with all the visual info incorporated
#	in them.  (The name of this file is specified by the USER when
#	an "export Data Model" menu action is performed.

###########################################################################
#
###########################################################################
proc parseObjects { file relationships entities } {
    upvar $relationships rel
    upvar $entities ent
#    upvar $isadeps isa

    # parse objects (relations and entities)
    while {[gets $file buf] != -1} {
#	puts "***** $buf"
	
	set buf [string trim $buf]
	switch -- [lindex $buf 0] {
	    "Schema:" {
		#puts [lindex $buf 0]
	    }
	    "Primitive:" {
		switch -- [lindex $buf 3] {
		    "Entity" {
#			puts $buf
			# id into array is Opossum numberic identifier
			set id [lindex $buf 1]
			if ![info exists ent($id)] {
			    set ent($id) [list [parseEntity $file]]
			} else {
			    set ent($id) \
			       [linsert $ent($id) 0 [parseEntity $file]]
			}
#			puts "...$ent($id)"
		    }
		    "Relationship" {
			set id [lindex $buf 1]
			if ![info exists rel($id)] {
			    set rel($id) [list [parseRelationship $file]]
			} else {
			    set rel($id) \
			       [linsert $rel($id) 0 [parseRelationship $file]]
			}
		    }
		    "ChildToIsA" {
			set isa [parseChildToIsA $file]
			lappend ent([lindex $isa 1]) [list [lindex $isa 0] "isa"]
		    }
		}
	    }
	    "Attr:" {
		#puts [lindex $buf 0]
	    }
	    default {
		puts "ERROR?: $buf"
	    }
	}
    }
#    puts "out"
#foreach e [array names ent] {
#    puts $ent($e)
#}
}


###########################################################################
#
###########################################################################
proc parseAttribs { file relationships entities isadeps } {
    upvar $relationships rel
    upvar $entities ent
#    upvar $isadeps isa

    # parse attributes
    while {[gets $file buf] != -1} {
#	puts "***** $buf"
	
	set buf [string trim $buf]
	switch -- [lindex $buf 0] {
	    "Schema:" {
		#puts [lindex $buf 0]
	    }
	    "Primitive:" {
		switch -- [lindex $buf 3] {
		    "Attribute" {
			set list [parseAttribute $file]
			if {[lsearch [array names rel] [lindex $list 0]]==-1} {
			    lappend ent([lindex $list 0]) $list
			} else {
			    lappend rel([lindex $list 0]) $list
			}
		    }
		    "Relates" {
			set list [parseRelates $file]
#			puts "########$list"
			lappend rel([lindex $list 0]) [lindex $list 1]
		    }
		    "Aggregate" {
#			parseAggregate $file
		    }
		    "Type" {
#			parseType $file
		    }
		    "IsA" {
			set isa [parseIsA $file]
#			puts $isa
#			puts "     [lindex $buf 1] isa $isa"
			foreach e [array names ent] {
			    set tmp {}
			    foreach attrib $ent($e) {
				if {[lindex $attrib 0]==[lindex $buf 1]} {
#				    puts "  $e isa ...$isa"
				    set attrib [list $isa "isa"]
				}
				lappend tmp $attrib
			    }
			    set ent($e) $tmp
			}
		    }
		}
	    }
	    "Attr:" {
#		puts [lindex $buf 0]
	    }
	    default {
		puts "ERROR?: [lindex $buf 0]"
	    }
	}
    }

    expandIsaDependancies ent
    expandForeignDependancies rel ent
}

###########################################################################
#
###########################################################################
proc expandForeignDependancies { relationships entities } {
    upvar $relationships rel
    upvar $entities ent

    foreach r [array names rel] {
	while {1} {
	    set p [hasParent $rel($r)]
	    if {$p>0} {
		set rel($r) [expandForeign $rel($r) $p [getParentKey $ent($p)]]
	    } else {
		break
	    }
	}
    }

}

###########################################################################
#
###########################################################################
proc expandForeign { r p k } {
    
#    puts "expan for"
#    puts "$k"
    set len [llength $r]
    for {set i 1} {$i < $len} {incr i} {
	if {[lindex [lindex $r $i] 0]==$p} {
	    foreach ki $k {
		    lappend r [concat $ki $p]
	    }
	    set r [lreplace $r $i $i]
	}
	break
    }
#    puts out
    return $r
}

###########################################################################
# Note: this algorithm assumes that the isa dependancies are non cyclic.
# more to the point a DAG will not do the trick even though it is a valid
# isa dependancy.  (need to fix this).  (I think this is fixed).
# 
# Each attribute that is derived from an IsA is appended with an extra
# list element which references the entity from which that attribute is
# derived.
###########################################################################
proc expandIsaDependancies { entities } {
    upvar $entities ent
    
#    puts "expand isa d"
    set flag 1
    set i 1
    while {$flag==1} {
	
	# deal with IsA cycles
	incr i
	if {$i > 100} {
	    puts "ERROR: IsA chain over 100???"
	    exit
	}

	set flag 0
	
	foreach e [array names ent] {
#	    puts "---$e"
	    set p [hasParent $ent($e)] 
	    if {$p>0} {
		if ![hasParent $ent($p)] {
		    set pkey [getParentKey $ent($p)]
#		    puts "-$pkey"
		    set ent($e) [expandIsa $ent($e) $p $pkey]
		    set flag 1
		}
	    }
	}
    }
}

###########################################################################
# a is attrib
# p is attrib parent (index thereof)
# k is parents key
###########################################################################
proc expandIsa { a p k } {

#    puts "expand isa"
    set len [llength $a]
    for {set i 1} {$i < $len} {incr i} {
	if {[lindex [lindex $a $i] 0]==$p} {
	    foreach ki $k {
		if ![subsetAttr [list [lindex $ki 1]] [join $a]] {
		    # add parent rel to attr (fkey)
		    lappend a [concat $ki $p]
		} 
	    }
	    set a [lreplace $a $i $i]
	}
	break
    }
    return $a
}

###########################################################################
# takes as an arg an attribute (a)
# returns the label of the parent if it has one 
# otherwise 0
###########################################################################
proc hasParent { a } {
    
    for {set i 1} {$i < [llength $a]} {incr i} {
	set label [lindex [lindex $a $i] 1]
	if {$label=="isa" || $label=="foreign"} {
	    return [lindex [lindex $a $i] 0]
	}
    }
    return 0
}

###########################################################################
# Only works on expanded attributes!
###########################################################################
proc getParentKey { rel } {

#    puts probaa
    set realkey 0
    set k {}
    for {set i 1} {$i < [llength $rel]} {incr i} {
	if {[lindex [lindex $rel $i] 3]==1} {
	    set realkey 1
	    lappend key [lindex $rel $i]
	}
    }

    if {$realkey==0} {
	foreach a [lrange $rel 1 [llength $rel]] {
	    lappend k [lreplace $a 3 3 1]
	}
#	puts probb
	return $k
    } else {
#	puts probc
	return $key
    }
}

###########################################################################
#
###########################################################################
proc parseEntity { file } {

    set name {}
#    puts "=Entity"
    while {[gets $file buf] != -1 && [lindex $buf 0] != "Primitive:"} {
	set lastLine [tell $file]

	switch -- [lindex $buf 1] {
	    "Label.Text" {
		set buf [split $buf ==]
		set name [string trim [lindex $buf 2]]
#		puts " name: $name"
	    }
	    "Name" {  ;# DATASCHEMA file
		set buf [split $buf ==]
		set name [string trim [lindex $buf 2]]
#		puts " name: $name"
	    }
	    default {
		#puts [lindex $buf 1]
	    }
	}
    }
    seek $file $lastLine start

    set properties $name
#    puts "B"
    return $properties
}

###########################################################################
#
###########################################################################
proc parseAttribute { file } {

#    puts "=Attribute"
    set name {}
    set isKey {}
    set type "char(20)"  ;# default type
    while {[gets $file buf] != -1 && [lindex $buf 0] != "Primitive:"} {
	set lastLine [tell $file]

	switch -- [lindex $buf 1] {
	    "Label.Text" {
		set buf [split $buf ==]
		set name [string trim [lindex $buf 2]]
	    }
	    "Name" {             ;# DATASCHEMA file
		set buf [split $buf ==]
		set name [string trim [lindex $buf 2]]
	    }
	    "AssociatedWith" {   ;# both files
		set buf [split $buf ==]
		set id [string trim [lindex $buf 2]]
	    }
	    "KeyLine.Visible" {	 ;# assume KeyLine.Visible denotes attribute as key
		set buf [split $buf ==]
		set isKey [string trim [lindex $buf 2]]
	    }
	    "Key" {	         ;# DATASCHEMA file 
		set buf [split $buf ==]
	        set buf [string trim [lindex $buf 2]]
		if {$buf == "Is-Not-Key"} {
		    set isKey 0
		} else {
		    set isKey 1
		}
	    }
	    "Type" {             ;# both files
		set buf [split $buf ==]
		set type [string trim [lindex $buf 2]]
		if {$type == "char"} {
		    set type "char(20)"
		}
	    }
	    default {
		#puts [lindex $buf 1]
	    }
	}
    }
    seek $file $lastLine start

    set properties [list $id $name $type $isKey]
    return $properties

}

###########################################################################
#
###########################################################################
proc parseRelationship { file } {

    set name {}
    while {[gets $file buf] != -1 && [lindex $buf 0] != "Primitive:"} {
	set lastLine [tell $file]

	switch -- [lindex $buf 1] {
	    "Label.Text" {
		set buf [split $buf ==]
		set name [string trim [lindex $buf 2]]
	    }
	    "Name" {		;# DATASCHEMA file
		set buf [split $buf ==]
		set name [string trim [lindex $buf 2]]
	    }
	    default {
		#puts [lindex $buf 1]
	    }
	}
    }
    seek $file $lastLine start

    set properties $name
    return $properties
}

###########################################################################
# ok for both file types
###########################################################################
proc parseRelates { file } {

    set name {}
    while {[gets $file buf] != -1 && [lindex $buf 0] != "Primitive:"} {
	set lastLine [tell $file]

	switch -- [lindex $buf 1] {
	    "Relationship" {
		set buf [split $buf ==]
		set relRelationship [string trim [lindex $buf 2]]
#		puts " name: $name"
	    }
	    "Entity" {
		set buf [split $buf ==]
		set relEntity [string trim [lindex $buf 2]]
#		puts " name: $name"
	    }
	    default {
		#puts [lindex $buf 1]
	    }
	}
    }
    seek $file $lastLine start

    set properties [list $relRelationship [list $relEntity foreign]]

    return $properties
}



###########################################################################
#
###########################################################################
proc parseAggregate { file relations } {
    # 1 lines
}



###########################################################################
# ok for both file types
###########################################################################
proc parseIsA { file } {

#    puts "=IsA"
    while {[gets $file buf] != -1 && [lindex $buf 0] != "Primitive:"} {
	set lastLine [tell $file]

	switch -- [lindex $buf 1] {
	    "ParentEntity" {
		set buf [split $buf ==]
		set parent [string trim [lindex $buf 2]]
#		puts " parent: $parent"
	    }
	    default {
		#puts [lindex $buf 1]
	    }
	}
    }
    seek $file $lastLine start

    set properties [list $parent]
    return $properties

}

###########################################################################
#
###########################################################################
proc parseType { file relations } {
    # 1 lines
}



###########################################################################
# ok for both file types
###########################################################################
proc parseChildToIsA { file } {

#    puts "=ChildToIsA"
    while {[gets $file buf] != -1 && [lindex $buf 0] != "Primitive:"} {
	set lastLine [tell $file]

	switch -- [lindex $buf 1] {
	    "ChildEntity" {
		set buf [split $buf ==]
		set child [string trim [lindex $buf 2]]
#		puts " child: $child"
	    }
	    "IsABox" {
		set buf [split $buf ==]
		set isA [string trim [lindex $buf 2]]
#		puts " isA: $isA"
	    }
	    default {
		#puts [lindex $buf 1]
	    }
	}
    }
    seek $file $lastLine start

    set properties [list $isA $child]
    return $properties
}


###########################################################################
# Nesc. ?
###########################################################################
proc resolveNames { relations entities } {
    upvar $relationships rel
    upvar $entities ent

    foreach index [array names rel] {
	for {set i 1} {$i < [llength $rel($index)] } {incr i} {
	}
    }
    foreach index [array names rel] {
	for {set i 1} {$i < [llength $ent($index)] } {incr i} {
	}
    }
}


###########################################################################
# 1 if every element in list small is in list big
# small is assumed to be smaller or same size as big
###########################################################################
proc subsetAttr { small big } {

    set s 0
    set b 0
    set slen [llength $small]
    while {$b < [llength $big] && $s < [llength $small]} {
        if {[lindex $small $s] == [lindex $big $b]} {
            incr s
            set b -1
            if {$s == $slen} {
                return 1
            }
        }
        incr b
    }
    return 0
}


