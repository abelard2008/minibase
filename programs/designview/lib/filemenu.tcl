
proc file_save { } {
    set current [file_getCurrentDBNFile]
    if {$current == {}} {
	file_saveAs
    } else {
	file_writeToFile $current
    }
}

proc file_new { } {
    set currentName [file tail [file_getCurrentFileName]]
    set attrs {}
    set FDs {}
    if {$currentName != {}} {
	set confirm [dialog .dialog "Create New Database" \
		[list Do you want to initialize the new database with the \
		attributes, FDs, and source relations of $currentName?] \
		"" 0 {Yes} {No} {Cancel}]
	if {$confirm == 2} { return }
	if {$confirm == 0} {
	    foreach a [db_getAttributes] {
		lappend attrs [list $a [db_getAttrType $a]]
	    }
	    foreach h [db_getFDHandles] {
		lappend FDs [list [db_getFDlhs $h] [db_getFDrhs $h]]
	    }
	    foreach r [db_getSourceRelations] {
		set R($r,name) [relation_getName $r]
		set R($r,attrs) [relation_getAttributes $r]
		set R($r,key) [relation_getKey $r]
		set R($r,fkey) [relation_getForeignKey $r]
	    }
	}
    }
    create_createDB $attrs $FDs R
}

proc file_saveAs { } {

    if {[file_getCurrentFileName] == {}} {
	one_line_message .error "Error" "No data to write."
	return
    }

    set fname [fileselect "Select an output file:" {} 0 {dbn}]
    if {$fname == {}} { return }
    update

    if [file exists $fname] {
	set overwrite [dialog .fileExists "File Exists" \
		"'[file tail $fname]' already exists. Overwrite?" "" \
		0 No Yes]
	if {$overwrite == 0} { return }
    }

    set w [file_writeToFile $fname]
    if {$w == 0} {
	file_setCurrentFileName $fname
	file_setCurrentDBNFile $fname
	updateTitleBar $fname
    }

}

proc file_close {} {

    set f [file tail [file_getCurrentFileName]]
    if {$f != {}} {
	set confirm [dialog .confirm "Confirm" \
		"Are you sure you want to close $f?" "" \
		0 No Yes]
	if {$confirm == 0} { return 0 }
    }

    db_deleteAllRelations
    sdisplay_deleteAllWindows
    clearMainWindow
    sdisplay_deleteAllWindows
    main_activateButtons 0
    file_clearCurrentFileName
    file_clearCurrentDBNFile
    updateTitleBar {}
    return 1

}

proc file_open {{o2r 0}} {

    global __DBN__o2r

    if ![file_close] {
	return
    }

    if $o2r {
	set opossumFile [fileselect "File Selection" {} 1 DM]
	if {$opossumFile == {}} { return }
	set tmpnam /tmp/o2r-[pid]
	if [catch {exec $__DBN__o2r $opossumFile > $tmpnam} result] {
	    one_line_message .errmsg Error $result
	    return 1
	}
	set fname $tmpnam
    } else {
	set fname [fileselect "File Selection" {} 1 dbn]
	if {$fname == {}} { return }
    }

    if [catch {open $fname r} fileId] {
	one_line_message .errmsg "Error" \
		"Could not open '[file tail $fname]'."
	return 1
    }

    update

    debug "Reading DBN file $fname..."
    parse_file $fileId
    close $fileId
    debug "Done reading DBN file $fname..."

    if $o2r {
	if [catch {exec /bin/rm $fname 2> /dev/null} result] {
	    puts "WARNING: $result"
	}
	file_setCurrentFileName $opossumFile
	file_setCurrentDBNFile {}
	updateTitleBar $opossumFile
    } else {
	file_setCurrentFileName $fname
	file_setCurrentDBNFile $fname
	updateTitleBar $fname
    }

    updateMainWindow
    main_activateButtons

}

proc file_writeToFile { filename } {

    if [catch {open $filename w} f] {
	one_line_message .error "Error" \
		"Error opening '[file tail $filename]'. No data was saved."
	return 1
    }

    puts $f ""

    #
    # Write Database data
    #
    puts $f "% Database"
    puts $f "SourceRelations = [db_getSourceRelations]"
    foreach a [lsort [db_getAttributes]] {
	puts $f "Attribute = $a [db_getAttrType $a]"
    }
    foreach h [lsort [db_getFDHandles]] {
	puts $f "FD = {[db_getFDlhs $h]} {[db_getFDrhs $h]}"
    }

    #
    # Write Relation data
    #
    foreach r [lsort [db_getAllRelations]] {
	if {[getRefCount $r] > 0} {
	    puts $f ""
	    puts $f "% Relation"
	    puts $f "ID = $r"
	    puts $f "Name = [relation_getName $r]"
	    puts $f "Attributes = [relation_getAttributes $r]"
	    set key [relation_getKey $r]
	    if {$key != {}} { puts $f "Key = $key" }
	    foreach fk [relation_getForeignKey $r] {
		puts $f "Foreign Key = [lindex $fk 0] [lindex $fk 1]"
	    }
	    set sessions [db_getSessions $r]
	    if {$sessions != {}} { puts $f "Sessions = $sessions" }
	}
    }

    #
    # Write Session data
    #
    foreach owner [lsort [db_getSessionOwners]] {
	foreach s [lsort [db_getSessions $owner]] {
	    if {[getRefCount $s] > 0} {
		puts $f ""
		puts $f "% Session"
		puts $f "ID = $s"
		puts $f "Name = [session_getName $s]"
		foreach r [lsort [session_getAllRelations $s]] {
		    set kids [session_getChildren $s $r]
		    if {$kids != {}} { puts $f "Decomposition = $r {$kids}" }
		    set d [session_getDefaultDecomposition $s $r]
		    if {$d != {}} {
			puts $f "Linked Session = $r $d"
		    }
		}
	    }
	}
    }

    close $f
    return 0
}

proc file_exit {} {
    set answer [dialog .quit "Quit" \
            "Are you sure you want to quit?" "" 1 {Cancel} {Exit}]
    if { $answer == 1 } {
        exit
    }
}

proc file_setCurrentFileName { fname } {
    global __FILE__current
    set __FILE__current $fname
}

proc file_getCurrentFileName {} {
    global __FILE__current
    if ![info exists __FILE__current] { return {} }
    return $__FILE__current
}

proc file_clearCurrentFileName {} {
    global __FILE__current
    if [info exists __FILE__current] {
	unset __FILE__current
    }
}

proc file_setCurrentDBNFile { fname } {
    global __FILE__dbnFile
    set __FILE__dbnFile $fname
}

proc file_getCurrentDBNFile {} {
    global __FILE__dbnFile
    if ![info exists __FILE__dbnFile] { return {} }
    return $__FILE__dbnFile
}

proc file_clearCurrentDBNFile {} {
    global __FILE__dbnFile
    if [info exists __FILE__dbnFile] {
	unset __FILE__dbnFile
    }
}

