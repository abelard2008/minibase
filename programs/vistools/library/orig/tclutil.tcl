# util.tcl - general utility procs
#
# Author: Allan Brighton (allan@piano.sta.sub.org)


# return the name of a file in (or under) the user's home directory
# (if HOME is defined) with the name $name1, or, if name2 is specified, 
# use name1 as the directory under $HOME and name2 as the filename.
# The directory is created if it does not exist.

proc utilGetConfigFilename {name1 {name2 ""}} {
    global env

    if {[info exists env(HOME)]} {
	set dir $env(HOME)
    } else {
	set dir .
    }

    if {"$name2" == ""} {
	return $dir/$name1
    } else {
	set dir $dir/$name1
	if {[file isfile $dir]} {
	    exec rm -f $dir
	}
	if {![file isdir $dir]} {
	    exec mkdir $dir
	}
	return $dir/$name2
    }
}


# process the argument list setting the local variable x
# for each option -x to the given value. The local
# var must already exist in the caller's stack frame

proc utilGetArgs {arglist} {
    set n [llength $arglist]
    for {set i 0} {$i < $n} {incr i} {
	set opt [lindex $arglist $i]
	set var [string range $opt 1 end]
	set arg [lindex $arglist [incr i]]
	if {[uplevel [list info exists $var]]} {
	    uplevel [list set $var $arg]
	} else {
	    error "invalid option: \"$opt\""
	}
    }
}


# return the value of the environment variable or the default
# string if its not defined

proc utilGetEnv {name {def ""}} {
    global env
    if {[info exists env($name)]} {
        return $env($name)
    }
    return $def
}


# return the last component of the directory name
# (/ is a special case)

proc utilDirTail {dir} {
    if {$dir == "/"} {return $dir}
   return [file tail $dir]
}


# add some formats to the tclX convertclock command
# (yy/mm/dd, yy-mm-dd)
# - it is easier to make the change here than in the yacc code in tclX...

proc utilConvertClock {date args} {
    if {[scan $date {%d%*[\-/]%d%*[\-/]%d} yy mm dd] == 3 && $yy > 12 && $mm <= 12} {
	set date $mm/$dd/$yy
    }
    return [eval "convertclock $date $args"]
}
