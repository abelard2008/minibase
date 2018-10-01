# compat.tcl
#
# The definitions below are only needed if you don't link in the 
# BLT and TclX toolkits.

# if blt is not installed, can't use blt_busy
if {[llength [info commands blt_busy]] == 0} {
    proc blt_busy {args} {}
}

# if tclX is not installed, define these here
if {[llength [info commands min]] == 0} {
    proc min {x y} {return [expr {$x < $y ? $x : $y}]}
}

if {[llength [info commands max]] == 0} {
    proc max {x y} {return [expr {$x > $y ? $x : $y}]}
}

if {[llength [info commands lassign]] == 0} {
    proc lassign {list args} {
	set n 0
	foreach i $list {
	    uplevel [list set [lindex $args $n] $i]
	    incr n
	}
    }
}


