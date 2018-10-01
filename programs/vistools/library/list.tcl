# list.tcl - list utility routines for tcl
#
# Author: Allan Brighton (allan@piano.sta.sub.org)


# Append the given element to the list if it is not
# already there

proc lunion {plist item} {
    upvar $plist list
    foreach i $list {
        if {"$i" == "$item"} {
	    return
        }
    }
    lappend list $item
}


# Remove any elements matching the given string from the list
# and return the result

proc lremove_item {list item} {
    set l {}
    foreach i $list {
        if {"$i" != "$item"} {
            lappend l $i
        }
    }
    return $l
}


# Remove the given element from the list and return the new list

proc lremove {list i} {
    return "[lrange $list 0 [expr $i-1]] [lrange $list [expr $i+1] end]"
}


# return the list in reverse order

proc lreverse {list} {
    set l {}
    set n [llength $list]
    while {$n > 0} {
	lappend l [lindex $list [incr n -1]]
    }
    return $l
}
