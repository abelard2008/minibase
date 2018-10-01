# util.tcl - utility tcl routines
#
# ------------------------------------------------------------
# Copyright 1993 Allan Brighton.
# 
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted,
# provided that the above copyright notice appear in all copies.  Allan
# Brighton make no representations about the suitability of this software
# for any purpose.  It is provided "as is" without express or implied
# warranty.
# ------------------------------------------------------------


# return the value of the environment variable of the empty
# string if its not defined

#proc utilGetEnv {name} {
#    global env
#    if {[lsearch [array names env] $name] != -1} {
#        return $env($name)
#    }
#    return ""
#}

# return the last component of the directory name
# (/ is a special case)

proc utilDirTail {dir} {
    if {$dir == "/"} {return $dir}
   return [file tail $dir]
}


# return the value of the given tk option for the given widget

proc utilGetConfigValue {w option} {
    return [lindex [$w config $option] 4]
}


# return the second parameter if this is not a color machine
# otherwise return the first

proc utilGetColor {color {mono black}} {
    if {"[tk colormodel .]" != "color"} {return $mono}
    return $color
}

# get the selection or return an empty string

proc utilGetSelection {} {
    set s ""
    catch {set s [selection get]} err
    return $s
}


# error message routine used before window is up
proc utilUsageError {msg} {
    puts stderr "usage: $msg"
    exit 1
}


# Center the window on the screen. 

proc utilCenterWindow {w} {
    wm withdraw $w
    update idletasks
    if {"$w" == "."} {
	set parent "."
    } else {
	set parent [winfo parent $w]
    }
    set x [expr [winfo screenwidth $w]/2 - [winfo reqwidth $w]/2 \
	    - [winfo vrootx $parent]]
    set y [expr [winfo screenheight $w]/2 - [winfo reqheight $w]/2 \
	    - [winfo vrooty $parent]]
    wm geom $w +$x+$y
    wm deiconify $w
}


# set the wait cursor for the window and clear it later

proc utilWaitCursor {w} {
    catch {
	$w config -cursor watch
	update idletasks
	after 1 "$w config -cursor {}"
    }
}


# run the given command with the given args in an xterm

proc utilXtermCmd {cmd args} {
   exec xterm -T "$cmd $args" -e sh -c "$cmd $args; cat > /dev/null" &
}


# set the background color of the root window

proc utilSetBackgroundColor {} {
    set f [frame .ftmp[pid]]
    set color [utilGetConfigValue $f -background]
    . config -background $color
}

