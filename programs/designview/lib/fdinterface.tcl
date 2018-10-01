# fdinterface.tcl tcl implementation of fd algorithms
#
# Andrew Prock, Spring 1996

###########################################################################
###########################################################################
# INTERFACE TO FDs 
###########################################################################
###########################################################################

###########################################################################
# Interface used to ad a functional dependance of the form
# lhs -> rhs
# to the set of functional dependancies FDset
###########################################################################
proc addToFDset { FDset lhs rhs } {
    upvar $FDset F

    if ![info exists F] {
	set F [FDset]
    }
    return [$F add $lhs $rhs]
}


###########################################################################
# Interface returns a list of all the handles into the FD set.
# Handles are used to specify a particular FD, for getting
# or for deleting.
###########################################################################
proc getFDlist { FDset } {
    upvar $FDset F

    return [$F get list]
}


###########################################################################
# Interface to get an fd with specified handle (see getFDlist)
###########################################################################
proc getFD { FDset handle } {
    upvar $FDset F

    return [$F get $handle]
}


###########################################################################
# Interface that deletes FD from a set
###########################################################################
proc deleteFD { FDset handle } {
    upvar $FDset F

    $F rem $handle
}


###########################################################################
# new format
###########################################################################
proc getFDlhs { FDset handle } {
    upvar $FDset F

    return [$F get lhs $handle]
}


###########################################################################
#
###########################################################################
proc copyFDset { FDset1 FDset2 } {
    upvar $FDset1 F
    upvar $FDset2 G

#    puts "ee"
    if ![info exists F] {
	set F [FDset]
    }
#    puts "ee"

    $F clear
#    puts "ee"
    foreach fd [getFDlist G] {
	$F add [$G get lhs $fd] [$G get rhs $fd]
    }
#    puts "1e"
}


###########################################################################
# new format
###########################################################################
proc getFDrhs { FDset handle } {
    upvar $FDset F

    return [$F get rhs $handle]

#    return [lindex $F($handle) 1]
}


###########################################################################
# prints the FD
#
# new format
###########################################################################
proc printFDset { FDset } {
    upvar $FDset F

    $F print
    puts ""

#    foreach a [lsort [getFDlist F]] {
#	puts "$a: [getFDlhs F $a] -> [getFDrhs F $a]"
#    }
#    puts ""
}

