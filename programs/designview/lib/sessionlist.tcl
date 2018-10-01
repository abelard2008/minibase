
proc sesslist_displayList { relation x y } {

    global __SLIST__done

    set w .sesslist_$relation

    toplevel $w
    wm title $w "Decompositions of [relation_getName $relation]"
    wm resizable $w 0 0
    wm geometry $w +$x+$y

    set l [ScrolledListbox $w.list_frame 1 10 -selectmode single \
	    -setgrid true]
    sesslist_updateList $relation $l
    bind $l <Double-1> \
	    "set __SLIST__done 1; sesslist_displaySession $relation $l $x $y"

    pack $w.list_frame -side left

    set bf [frame $w.bframe]
    button $bf.view -text "View Session" -width 15 -command \
	    "set __SLIST__done 1; sesslist_displaySession $relation $l $x $y"
    button $bf.delete -text "Delete Session" -width 15 -command \
	    "sesslist_deleteSession $relation $l"
    button $bf.default -text "Set as Default" -width 15 -command \
	    "sesslist_setDefault $relation $l"
    button $bf.newsession -text "New Session" -width 15 -command \
	    "set __SLIST__done 1; sesslist_newSession $relation $x $y"
    button $bf.done -text "Dismiss" -width 15 \
	    -command "set __SLIST__done 1"
    pack $bf.view $bf.delete $bf.default $bf.newsession $bf.done -pady 2
    pack $w.bframe -side right -anchor e -padx 20

    bind $w <Destroy> "set __SLIST__done {}"

    tkwait visibility $w
    grab set $w
    focus $w
    
    tkwait variable __SLIST__done

    if [winfo exists $w] { destroy $w }

}

proc sesslist_newSession { relation x y } {
    set session [session_makeSession $relation]
    session_setName $session "New Session"
    sdisplay_makeSDisplay $session $x $y
}

proc sesslist_displaySession { relation listbox x y } {
    set sessions [db_getSessions $relation]
    set index [$listbox curselection]
    if {$index == {}} { return }
    set session [lindex $sessions $index]
    sdisplay_makeSDisplay $session $x $y
}

proc sesslist_deleteSession { relation listbox } {
    set sessions [db_getSessions $relation]
    set index [$listbox curselection]
    if {$index == {}} { return }
    set session [lindex $sessions $index]
    set sname [session_getName $session]

    set confirm [dialog .confirm "Delete Session" \
            "Are you sure you want to delete \"$sname\"?" \
	    "" 0 {Cancel} {Delete}]
    if {$confirm == 0} { return }

    db_deleteSession $relation $session
    sesslist_updateList $relation $listbox

}

proc sesslist_setDefault { relation listbox } {
    set sessions [db_getSessions $relation]
    set index [$listbox curselection]
    if {$index == {}} { return }
    set session [lindex $sessions $index]
    db_setDefaultSession $relation $session
    sesslist_updateList $relation $listbox
}

proc sesslist_updateList { relation listbox } {
    set sessions [db_getSessions $relation]
    set index [$listbox curselection]
    if {$index == {}} { set index 0 }
    $listbox delete 0 end
    set snames {}
    foreach s $sessions {
	lappend snames [session_getName $s]
    }
    eval $listbox insert end $snames
    $listbox selection clear 0 end
    $listbox selection set $index $index
}

