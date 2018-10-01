
proc SelectionBox { prompt choices {title "Make a Selection"} } {

    global __SELECTION__choice
    global __SELECTION__list

    set w .selection
    catch {destroy $w}

    toplevel $w
    wm title $w $title
    wm resizable $w 0 0
    
    label $w.label -text $prompt
    pack $w.label -side top -expand true -fill x -pady 10 -padx 10

    set l [ScrolledListbox $w.listFrame 1 8 -selectmode single \
	    -setgrid true]
    set __SELECTION__list $l
    eval $l insert end $choices
    bind $l <Double-1> [concat \
	    { set __SELECTION__choice [ } $l { curselection ] } ]
    pack $w.listFrame -side top -expand true -fill both

    frame $w.buttons
    
    button $w.buttons.ok -text OK -width 15 -command {
	if {[$__SELECTION__list curselection] != {}} {
	    set __SELECTION__choice [$__SELECTION__list curselection]
	}
    }
    button $w.buttons.cancel -text "Cancel" -width 15 -command \
	    [concat set __SELECTION__choice {{}}]
    pack $w.buttons.ok $w.buttons.cancel -side left -pady 2
    pack $w.buttons -side bottom -padx 5

    bind $w <Destroy> "set __SELECTION__choice {}"

    tkwait visibility $w
    grab set $w
    focus $w
    
    tkwait variable __SELECTION__choice
    set result $__SELECTION__choice

    catch {destroy $w}
    
    return $result
    
}

