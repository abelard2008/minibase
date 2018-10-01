
proc debug { s } {
    global __DBN__debug
    if ![info exists __DBN__debug] { return }
    if $__DBN__debug { puts $s }
}

proc debug_on { } {
    global __DBN__debug
    set __DBN__debug 1
}

proc debug_off { } {
    global __DBN__debug
    set __DBN__debug 0
}

proc not_implemented { } {
    one_line_message .msg "Error" "Not implemented."
}

proc max { args } {
    set max [lindex $args 0]
    foreach a $args {
	if { $a > $max } {
	    set max $a
	}
    }
    return $max
}

proc min { args } {
    set min [lindex $args 0]
    foreach a $args {
	if { $a < $min } {
	    set min $a
	}
    }
    return $min
}

proc clear_canvas { c } {
    foreach child [winfo children $c] { destroy $child }
    $c delete all
}

#
# Print values of global variables to stdout
#
proc show_globals { {prefix __} } {

    if {$prefix == {}} {
	set names [lsort [info globals *]]
    } else {
	set names [lsort [info globals $prefix*]]
    }

    foreach v $names {
	global $v
	if ![array exists $v] { puts "$v = [set $v]" }
    }
    foreach v $names {
	global $v
	if [array exists $v] {
	    puts "Array $v"
	    if [string compare $v __DB__FDset] {
		foreach n [lsort [array names $v]] {
		    puts "    $n = [set ${v}($n)]"
		}
	    }
	}
    }
}

#
# Display a file-select dialog, then load the code in the selected file.
#
proc reload_file { } {
    set fname [fileselect "File Selection" {} 1 tcl]
    if {$fname == {}} { return }
    source $fname
}

#
# Lists: append elements of list2 to list1 (list1 is passed by name)
#
proc append_unique { listvar list2 } {
    upvar $listvar list1
    foreach elt $list2 {
        if {[lsearch $list1 $elt] == -1} { lappend list1 $elt }
    }
}

proc delete_from_list { listvar item } {
    upvar $listvar list
    set index [lsearch $list $item]
    while {$index != -1} {
	set list [lreplace $list $index $index]
	set index [lsearch $list $item]
    }
}

#
# Lists: compute "intersection" of the two lists
#
proc set_intersection { list1 list2 } {
    set result {}
    foreach elt $list2 {
        if {[lsearch $list1 $elt] != -1} { 
	    if {[lsearch $result $elt] == -1} { 
		lappend result $elt
	    }
	}
    }
    return $result
}

#
# Lists: compute "difference" of list1 - list2
#
proc set_difference { list1 list2 } {
    set result {}
    foreach elt $list1 {
	set a($elt) 1
    }
    foreach elt $list2 {
	if [info exists a($elt)] {
	    unset a($elt)
	}
    }
    return [array names a]
}

#
# Lists: compute "union" of list1 and list2
#
proc set_union { list1 list2 } {
    foreach elt $list1 { set a($elt) 1 }
    foreach elt $list2 { set a($elt) 1 }
    return [array names a]
}

#
# Listboxes: get text of selected lines
#
proc get_selected_lines { list } {
    set indeces [$list curselection]
    set result {}
    foreach i $indeces {
	lappend result [$list get $i]
    }
    return $result
}

#
# Listboxes: copy selected lines from list1 to list2
#
proc copy_selected_lines { list1 list2 } {
    set index_list [$list1 curselection]
    set selected_lines {}
    foreach i $index_list {
        lappend selected_lines [$list1 get $i]
    }
    add_lines_to_list $selected_lines $list2
}

#
# Listboxes: add some lines (only ones that are not already there)
#
proc add_lines_to_list { lines list } {
    set old [$list get 0 end]
    foreach l $lines {
	if {[lsearch -exact $old $l] == -1} {
	    $list insert end $l
	}
    }
}

#
# Listboxes: delete all highlighted lines
#
proc delete_selected_lines { list } {
    foreach i [lsort -integer -decreasing [$list curselection]] {
        $list delete $i
    }
}

proc valid_relation_name { r } {
    # Error if name does not begin with a letter
    if ![regexp {^[a-zA-Z]} $r] { return 0 }
    # Error if any characters other than letters, numbers, underscores
    if [regexp {[^a-zA-Z0-9_]} $r] { return 0 }
    return 1
}

proc unique_relation_name { r } {
    set r [string toupper $r]
    foreach name [dbn_getRelationNames] {
	if ![string compare [string toupper $name] $r] {
	    return 0
	}
    }
    return 1
}

#
# peek at the next available character on file descriptor f
#
proc peek { f } {
    set c [read $f 1]
    if {$c != {}} { seek $f -1 current }
    return $c
}

#
# skip over incoming whitespace characters on file descriptor f
#
proc skipws { f } {
    while {[isspace [read $f 1]]} {
	# do nothing
    }
    if ![eof $f] {
	seek $f -1 current
    }
}

proc isspace { c } {
    return [regexp "^\[ \t\n\r\f\v]\$" $c]
}

proc writeTextToFile { textwidget {filename {}} {overwrite 0} } {

    if ![winfo exists $textwidget] {
	one_line_message .message "Error" \
		"ERROR: Text widget does not exist: $textwidget"
	return 0
    }

    if {$filename == {}} {
	set filename [fileselect "Select an output file:" {} 0 {}]
	if {$filename == {}} {
	    return 0
	}
    }

    if {[file exists $filename] && !$overwrite} {
	set overwrite [dialog .fileExists "File Exists" \
		"'[file tail $filename]' already exists. Overwrite?" "" \
		0 No Yes]
	if {$overwrite == 0} {
	    return 0
	}
    }

    if [catch {open $filename w} fileId] {
	one_line_message .message "Error" \
		"Could not open '[file tail $filename]'."
	return 0
    }

    set result [puts -nonewline $fileId [$textwidget get 1.0 end]]
    close $fileId

    if {$result != {}} {
	one_line_message .message "Error" $result
	return 0
    }

    return 1

}

proc resizeWidget { w widthOrHeight delta } {
    set old [$w cget -${widthOrHeight}]
    set new [expr $old + $delta]
    debug "Resizing widget: w=$w, old=$old, new=$new"
    $w config -${widthOrHeight} ${new}p
}

