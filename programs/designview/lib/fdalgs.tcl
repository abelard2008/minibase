# algs.tcl tcl implementation of fd algorithms
#
# Andrew Prock, Spring 1996

###########################################################################
###########################################################################
# BEGINNING OF ALGS
###########################################################################
###########################################################################
set LIB ~prock/fd/alg/stable
#source $LIB/fdinterface.tcl

###########################################################################
# Compute the closure of the attributes in (attrs) with respect to
# the FDs in FDset.
#
# attrs:	list attributes to be closed
# FDset:	FDset
#
# returns	closure of the attributes in attrs
###########################################################################
proc attrClosure { FDset attrs } {
    upvar $FDset F

    return [$F attrClosure $attrs]
}

###########################################################################
# Tests if two FDsets are equivalent
###########################################################################
proc equivFD { FDset1 FDset2 } {
    upvar $FDset1 F
    upvar $FDset2 G

    foreach fd [$F get list] {
	set antC [$G attrClosure [$F get lhs $fd]]
	if {[subsetAttr [$F get rhs $fd] $antC]==0} {
	    return 0
	}
    }

    return 1
}

###########################################################################
# Produces a non-redundant, left-hand-side minimum FDset with 
# every right hand side of length one,
#
# FDset:	(input) set of FDs
# CCset:	(output) canonical cover FDset of FDset
###########################################################################
proc canonicalCover { FDset CCset } {
    upvar $FDset F
    upvar $CCset Ccov

    set Tset [FDset]
    set NRTset [FDset]

    # create tmp fdset with consequences of length 1
    foreach fd [$F get list] {
	foreach rhAttr [$F get rhs $fd] {
	    $Tset add [$F get lhs $fd] [list $rhAttr]
	}
    }

    nonredundantCover Tset NRTset
    LminCover NRTset Ccov
}

###########################################################################
# Produces a FDset with the left hand side of every FD minimized
#
# FDset:	input set of FDs
# LMset:	output set of LHS minimum FDs
###########################################################################
proc LminCover { FDset LMset } {
    upvar $FDset F
    upvar $LMset LM

    # copy F into LM
    copyFDset LM F

    # check for redundancy
    foreach fd [$F get list] {
	# fds with left hand sides == 1 *are* min
	if {[llength [$F get lhs $fd]] == 1} {
	    continue
	}

	set z [$F get lhs $fd]
	foreach attr $z {
	    set attrC [$LM attrClosure [listDiff $z $attr]]
	    if [subsetAttr [$F get rhs $fd] $attrC] {
		set z [listDiff $z $attr]
	    }
	}
	$LM set lhs $z $fd
    }
}

###########################################################################
#
# FDset:	(input) set of FDs
# NRset:	(output) set of non-redundant FDs
#
# returns	1 (for success)
###########################################################################
proc nonredundantCover { FDset NRset } {
    upvar $FDset F
    upvar $NRset NR

    # copy F into NR
    copyFDset NR F

    # one by one check if each fd is redundant, if so delete
    for {set i [expr [$F size] -1]} {$i >= 0} {incr i -1} {
	$NR rem $i

	set lhsCl [$NR attrClosure [$F get lhs $i]]
	if {[subsetAttr [$F get rhs $i] $lhsCl]==0} {
	    #$NR add [getFDlhs F $i] [getFDrhs F $i]
	    $NR add [$F get lhs $i] [$F get rhs $i]
	} 
    }
    return 1
}

###########################################################################
# Test a decomposition of a relation for lossless join.
#
# FDset:	(input) set of FDs to test lossless join
# rels:		a Tcl list of lists (which are collections
#		of attributes in the decomposition
#
# returns	TRUE/FALSE
#		-1
###########################################################################
proc testLosslessJoin { FDset rels } {
    upvar $FDset F

    # check for lossless join for TWO set decomposition
    if {[llength $rels]==2} {
	set rel1 [lindex $rels 0]
	set rel2 [lindex $rels 1]

	set isect [listIntersect $rel1 $rel2]
	if {[isKey F $isect $rel1]=="TRUE" || [isKey F $isect $rel2]=="TRUE"} {
	    return TRUE
	}
	return FALSE

    # non-binary lossless join
    } else {
	set U {}
	foreach r $rels {
	    set U [listUnion $U $r]
	}
	set j 0
	set i 0
	foreach X $rels {
	    foreach u $U {
		if {[subsetAttr $u $X]} {
		    set table($i,$u) $u
		} else {
		    set table($i,$u) $j
		    incr j
		}
	    }
	    incr i
	}

	puts "INITIAL TABLE"
	printTable table [llength $X] $U

	# update loop
	set change 1
	while {$change} {
	    set change 0
	    for {set f 0} {$f < [$F size]} {incr f} {
		for {set i 0} {$i < [llength $X]} {incr i} {
		    for {set j [expr $i+1]} {$j < [llength $X]} {incr j} {

			set count 0
			foreach attr [$F get lhs $f] {
			    if {$table($i,$attr)==$table($j,$attrC)} {
				incr count
			    }
			}
			if {$count==[llength [$F get lhs $f]]} {
			    set change [updateTable F $f table $i $j]
			}

		    }
		}
		puts "considered FD [$F get lhs $f] -> [$F get rhs $f]"
		printTable table [llength $X] $U
	    }
	}

	# now check for a row in the table with all "locked" values
	for {set i 0} {$i < [llength $X]} {incr i} {
	    set count 0
	    for {set j 0} {$j < [llength $U]} {incr j} {
		if {$table($i,[lindex $U $j])==$table($i,[lindex $U $j])} {
		    incr count
		}
	    }
	    if {$count == [llength $U]} {
		return TRUE
	    } else {
		return FALSE
	    }
	}
    }

}

###########################################################################
# for lossless join
###########################################################################
proc updateTable { FDset f array row1 row2 } {
    upvar $FDset F
    upvar $array ar

    set change 0
    foreach attr [$F get rhs $f] {
	if {$ar($row1,$attr)!=$ar($row2,$attr)} {
	    set change 1
	    if {$ar($row2,$attr)==$attr} {
		set ar($row1,$attr) $attr
	    } else {
		set ar($row2,$attr) $ar($row1,$attr)
	    }
	}
    }
    
    return $change
}

###########################################################################
# for lossless join
###########################################################################
proc printTable { array N U } {
    upvar $array ar
    
    foreach u $U {
	puts -nonewline "$u	"
    }
    puts ""

    foreach u $U {
	puts -nonewline "-----	"
    }
    puts ""

    for {set i 0} {$i < $N} {incr i} {
	foreach u $U { 
	    puts -nonewline "$ar($i,$u)	"
	}
	puts ""
    }
    puts ""
}


###########################################################################
# Tests whether the attributes in (attrs) are a key over the relation
# (rel) with respect to the FDs held in FDset.
#
# FDset:	FDs
# attrs:	possible key
# rel:		relation over which (attrs) may be key
#
# returns	TRUE/FALSE
###########################################################################
proc isKey { FDset attrs rel } {
    upvar $FDset F

    set attrs_cl [$F attrClosure $attrs]
    if { [subsetAttr $rel $attrs_cl]=="1" } {
	return TRUE
    }
    return FALSE
}

###########################################################################
#
# FDset:
# rels:		a Tcl list of lists (which are collections
#		of attributes in the decomposition
#
# returns	TRUE/FALSE
###########################################################################
proc fdPreserving { FDset rels } {
    upvar $FDset F

    set relnum [llength $rels]

    for {set i 0} {$i < [$F size]} {incr i} {

	set Z [$F get lhs $i]
	set change 1

	while { $change=="1" } {
	    set change 0
	    foreach rel $rels {
		set T [rop F $Z $rel]
		if { [llength $T] > [llength $Z] } {
		    set change 1
		    set Z $T
		}
	    }
	}

	if { ![subsetAttr [$F get rhs $i] $Z] } {
	    return FALSE
	}
    }

    return TRUE
}

###########################################################################
# R-operation defined on page 400 of Ullman
###########################################################################
proc rop { FDset attrs rel } {
    upvar $FDset F

    set iclos [$F attrClosure [listIntersect $attrs $rel]]
    set ii [listIntersect $iclos $rel]
    set result [listUnion $attrs [listIntersect $iclos $rel]]

    return $result
}

###########################################################################
#
###########################################################################
proc test3NF { FDset rel } {
    upvar $FDset F

    return [$F test3NF $rel]
    #return "-1 : not implemented yet"
}

###########################################################################
# Determines whether an attribute is prime
###########################################################################
proc isPrime { FDset R A } {
    upvar $FDset F

    return [$F isPrime $A $R]
}

###########################################################################
# FDset is assumed to be canonical
###########################################################################
proc maxset { FDset R A } {
    upvar $FDset F

    set maxA [listDiff $R $A]

	
}

###########################################################################
#
###########################################################################
proc listKeys { FDset attrs } {
    upvar $FDset F

    return "-1 : not implemented yet"
}

###########################################################################
#
###########################################################################
proc decompose3NF { FDset attrs } {
    upvar $FDset F

    canonicalCover F G

    set result {}
    foreach fd [$G get list] {
	set X [$G get lhs $fd]
	puts $X
	foreach fd2 [$G get list] {
	    if {[$G get lhs $fd]==[$G get lhs $fd2]} {
		lappend X [$G get rhs $fd]
	    }
	}
	lappend result $X
    }
    return $result
}

###########################################################################
# See pg 155 of Mannila & Raiha
# requires FDset be in canonical form
# Can I replace R with [unionLHS] ???
###########################################################################
proc projectFD { FDset R X FDprojection } {
    upvar $FDset F
    upvar $FDprojection Fp

    set H [FDset]

    set R [unionAttrs F]
#    puts $R

    copyFDset G F
    set W [listDiff $R $X]
    while { $W != {} } {
        set A [lindex $W 0]
        set W [listDiff $W $A]
	set Ylist {}
	set Zlist {}
	set Blist {}

	# find new dependancy parts (ZY->B)
	for {set i 0} {$i < [$G size]} {incr i} {
	    if { [$G get rhs $i] == $A } {
		lappend Ylist [$G get lhs $i]
	    }
	    
	    if { [subsetAttr $A [$G get lhs $i]] } {
		set newZ [listDiff [$G get lhs $i] $A]
		    lappend Zlist $newZ
		lappend Blist [$G get rhs $i]
	    }
	}

	# create dependancies taking care to remove trivialities
	for {set z 0} {$z < [llength $Zlist]} {incr z} {
	    for {set y 0} {$y < [llength $Ylist]} {incr y} {
		for {set b 0} {$b < [llength $Blist]} {incr b} {
		    set ant [listUnion [lindex $Zlist $z] [lindex $Ylist $y]]
		    set con [listDiff [lindex $Blist $b] $ant]
		    if {$con != {} } {
			$H add $ant $con
		    }
		}
	    }
	}

	# remove FDs with $A in them
	for {set g [$G size]} {$g >= 0} {incr g -1} {
	    if {[subsetAttr $A [$G get lhs $g]] || \
		    [subsetAttr $A [$G get rhs $g]]} {
		$G rem $g
	    }
	}

	# add new dependancies to G
	for {set h 0} {$h < [$H size]} {incr h} {
	    $G add [$H get lhs $h] [$H get rhs $h]
	}
	$H clear
    }

    nonredundantCover G Fp
    $G clear
}

###########################################################################
# See pg 215 of Mannila & Raiha
###########################################################################
proc decomposeBCNF { FDset U } {
    upvar $FDset F

    set Fp [FDset]
    set bcnf [testBCNF F $U]

    if {$bcnf=="TRUE"} {
	return [list $U]
    } else {

	set badf [lindex $bcnf 0]
	set X  [$F get lhs $badf] 
	set Y  [$F get rhs $badf]
	set XY [listUnion $X $Y]
	set XYcomp [listDiff $U [listDiff $Y $X]]
	set Alist [decomposeBCNF Fp $XY]
	$Fp clear
	projectFD F $U $XYcomp Fp
	set Blist [decomposeBCNF Fp $XYcomp]
    }

    set result $Alist
    foreach ri $Blist {
	lappend result $ri
    }
    return $result
}

###########################################################################
# 
# FDset:	FDset to check for BCNF
# rel:		relation to check for BCNF
#
# returns	TRUE or the handle of the FD that violated BCNF
###########################################################################
proc testBCNF { FDset rel } {
    upvar $FDset F

    set result {}
    for {set i 0} {$i < [$F size]} {incr i} {

	if { [llength [$F get rhs $i]]!=1 } {
	    puts "ERROR: FDset must be cannonical"
	    return "-1 : FDset must be cannonical"
	}

	set fdattrs [listUnion [$F get lhs $i] [$F get rhs $i]]
	if { [subsetAttr $fdattrs $rel] } {
	    if { [isKey F [$F get lhs $i] $rel] == "FALSE" } {
		if {[testBCNF F \
			[listUnion [$F get lhs $i] [$F get rhs $i]]]=="TRUE"} {
		    lappend result $i
		}
	    }
	}
    }    

    if {$result=={}} {
	return TRUE
    } else {
	return $result
    }
}

###########################################################################
# returns a list that is the union of all elements in an array
# sorted w/no redundancies
# 
###########################################################################
proc unionLHS { FDset } {
    upvar $FDset F

    foreach el [$F get list] {
	foreach attrib [$F get lhs $el] {
	    set union($attrib) 1
	}
    }

    return [lsort [array names union]]
}

###########################################################################
# returns a list that is the union of all elements in an array
# sorted w/no redundancies
# 
###########################################################################
proc unionAttrs { FDset } {
    upvar $FDset F

    foreach el [$F get list] {
	foreach attrib [$F get lhs $el] {
	    set union($attrib) 1
	}
    }
    foreach el [$F get list] {
	foreach attrib [$F get rhs $el] {
	    set union($attrib) 1
	}
    }

    return [lsort [array names union]]
}

###########################################################################
###########################################################################
# NON FD UTILS
###########################################################################
###########################################################################

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


###########################################################################
#
###########################################################################
proc listIntersect { X Y } {

    set result {}
    foreach a $X {
	if {[lsearch $Y $a]!=-1} {
	    lappend result $a
	}
    }
    return $result
}

###########################################################################
#
###########################################################################
proc listUnion { X Y } {

    set result {}
    foreach a $X {
	set union($a) 1
    }
    foreach a $Y {
	set union($a) 1
    }
    
    return [lsort [array names union]]
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

