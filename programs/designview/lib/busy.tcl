
#
# From the Tcl/Tk FAQ:
#
# From: -II-  Tk Questions and Answers - How can I:
# 
# A2.C.2. Here is a tip from mgc@cray.com (M. G. Christenson).
# 
# Look at /usr/include/X11/cursorfont.h for a list of available cursors.
# You can use the names in there by removing the 'XC_'.  
# 
# Here's a little proc I use to make my entire application go 'busy'
# while it's doing something. Just call it with the commands you want to
# execute, and the watch cursor will be displayed for the time it takes
# the commands to complete.  Note that any new windows will have their
# normal cursor.
#

proc busy {cmds} {

    global errorInfo

    debug "BUSY commands: $cmds"

    #set busy {.app .root}
    set busy { . }
    set list [winfo children .]
    while {$list != ""} {
        set next {}
        foreach w $list {
            set class [winfo class $w]
            set oldcursor [lindex [$w config -cursor] 4]
            if {[winfo toplevel $w] == $w || $oldcursor != ""} {
                lappend busy [list $w $oldcursor]
            }
            set next [concat $next [winfo children $w]]
        }
        set list $next
    }

    debug "BUSY list: $busy"

    foreach w $busy {
        catch {[lindex $w 0] config -cursor watch}
    }

    update idletasks

    set error [catch {uplevel eval [list $cmds]} result]
    set ei $errorInfo

    foreach w $busy {
        catch {[lindex $w 0] config -cursor [lindex $w 1]}
    }

    if $error {
        error $result $ei
    } else {
        return $result
    }

}

