
proc dbn_showHelp { topic } {

    global __DBN__helpDir

    set w .help_$topic

    if [winfo exists $w] {
	if [catch {raise_toplevel $w}] { raise $w }
	return
    }

    toplevel $w
    wm title $w "Help for dbnorm"

    set f [frame $w.top]
    pack $f -side top -fill both -expand true

    set t [text $f.t -setgrid true -wrap word \
	    -yscrollcommand "$f.sy set"]
    scrollbar $f.sy -orient vertical -width 10 -command "$f.t yview"

    pack $f.sy -side right -fill y
    pack $f.t -side left -fill both -expand true

    $t insert end [exec cat $__DBN__helpDir/[string tolower $topic].help]

}

