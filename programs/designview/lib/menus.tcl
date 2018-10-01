
proc menu_setup { } {

    global __MAIN__menuFrame
    global __MAIN__fileMenu
    global __MAIN__inactiveFileCommands
    global __MAIN__relationMenu
    global __MAIN__helpMenu

    set __MAIN__menuFrame [frame .mainmenus -relief raised -bd 2]
    set f $__MAIN__menuFrame
    pack $f -side top -fill x -anchor nw -pady 0

    #
    # File Menu
    #
    set __MAIN__fileMenu [menubutton $f.file -text File \
	    -menu $f.file.menu -underline 0]
    set filemenu [menu $f.file.menu -tearoff 0]
    $filemenu add command -label Open -underline 0 \
	    -command [list busy file_open]
    $filemenu add command -label {Close} -underline 0 -state disabled \
	    -command [list busy file_close]
    $filemenu add command -label {New} -underline 0 \
	    -command [list busy file_new]
    $filemenu add separator
    $filemenu add command -label Save -underline 0 -state disabled \
	    -command [list busy file_save]
    $filemenu add command -label {Save As} -underline 1 -state disabled \
	    -command [list busy file_saveAs]
    $filemenu add command -label {Show SQL} -underline 6 -state disabled \
	    -command [list busy sql_display]
    $filemenu add separator
    $filemenu add command -label {Import Opossum Schema} -underline 0 \
	    -command [list busy [list file_open 1]]
    $filemenu add separator
    $filemenu add command -label {Exit} -underline 0 \
	    -command [list file_exit]
    pack $f.file -side left -padx 10

    set __MAIN__inactiveFileCommands [list Close Save {Save As} {Show SQL}]

    #
    # Relation Menu
    #
    set __MAIN__relationMenu [menubutton $f.relation -text Relation \
	    -menu $f.relation.menu -underline 0 -state disabled]
    set relationmenu [menu $f.relation.menu -tearoff 0]
    $relationmenu add command -label {View/Hide Details} -underline 0 \
	    -command [list busy main_viewDetails]
    $relationmenu add command -label {Start New Session} -underline 0 \
	    -command [list busy main_newSession]
    pack $f.relation -side left -padx 10

    #
    # Help Menu
    #
    set __MAIN__helpMenu [menubutton $f.help -text Help \
	    -menu $f.help.menu -underline 0]
    set helpmenu [menu $f.help.menu -tearoff 0]
    $helpmenu add command -label {Introduction} -underline 0 \
	    -command [list busy {dbn_showHelp intro}]
    $helpmenu add command -label {Main Window} -underline 0 \
	    -command [list busy {dbn_showHelp main}]
    $helpmenu add command -label {Session Window} -underline 0 \
	    -command [list busy {dbn_showHelp session}]
    $helpmenu add command -label {Input File Format} -underline 1 \
	    -command [list busy {dbn_showHelp inputfile}]
    $helpmenu add command -label {Using Opossum} -underline 0 \
	    -command [list busy {dbn_showHelp opossum}]
    pack $f.help -side right -padx 10

}

