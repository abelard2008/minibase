# tkutil.tcl - utility tcl routines
#
# Author: Allan Brighton (allan@piano.sta.sub.org) 


# return the name of the top level window for w, or "" if it is .

proc utilGetTopLevel {w} {
    set t [winfo toplevel $w]
    if {"$t" == "."} {
	return ""
    }
    return $t
}


# return the value of the given tk option for the given widget

proc utilGetConfigValue {w option} {
    return [lindex [$w config $option] 4]
}


# get the selection or return an empty string

proc utilGetSelection {} {
    set s ""
    catch {set s [selection get]} err
    return $s
}


# if the widget exists, make it visible and reconfigure it
# otherwise create it with the given options

proc utilReUseWidget {type w args} {
    if {[catch {wm deiconify $w}]} {
	eval "$type $w $args"
    } else {
	eval "$w config $args"
    }
}

