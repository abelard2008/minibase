
proc sql_display {} {

    global __SQL__filename
    global __SQL__textWidget

    set w .sql
    catch {destroy $w}
    toplevel $w
    wm title $w "SQL Commands for [file tail [file_getCurrentFileName]]"
    bind $w <Destroy> { sql_destroy }

    set f [frame $w.text]
    set t [text $f.t -setgrid true -wrap word \
	    -yscrollcommand "$f.sy set"]
    scrollbar $f.sy -orient vertical -width 10 -command "$f.t yview"
    pack $f.sy -side right -fill y
    pack $f.t -side left -anchor nw -fill both -expand true

    set f [frame $w.buttons]
    set save [button $f.b1 -text "Save" -command sql_save]
    set saveAs [button $f.b2 -text "Save As" -command sql_saveAs]
    set close [button $f.b3 -text "Close" -command [concat destroy $w]]
    pack $save $saveAs $close -side left -padx 5 -pady 5

    pack $w.buttons -side bottom
    pack $w.text -side bottom -fill both -expand true

    foreach r [db_getBaseRelations] {
	$t insert end "\n"
	$t insert end [relation_getSQL $r 1]
    }
    
    $t config -state disabled

    set __SQL__filename {}
    set __SQL__textWidget $t

}

proc sql_save {} {

    global __SQL__filename
    global __SQL__textWidget

    if {$__SQL__filename == {}} {
	sql_saveAs
	return
    }

    writeTextToFile $__SQL__textWidget $__SQL__filename 1

}

proc sql_saveAs {} {

    global __SQL__filename
    global __SQL__textWidget

    set __SQL__filename [fileselect "Select an output file:" {} 0 {sql}]
    if {$__SQL__filename == {}} {
	return
    }

    if [file exists $__SQL__filename] {
	set overwrite [dialog .fileExists "File Exists" \
		"'[file tail $__SQL__filename]' already exists. Overwrite?" \
		"" 0 No Yes]
	if {$overwrite == 0} {
	    return
	}
    }

    writeTextToFile $__SQL__textWidget $__SQL__filename 1

}

proc sql_destroy {} {
    global __SQL__filename
    global __SQL__textWidget
    catch { unset __SQL__filename }
    catch { unset __SQL__textWidget }
}

