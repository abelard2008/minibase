
#
# User interface for creating a new dbnorm database file. The new database
# can be initialized with attributes, FDs, and source relations by passing
# in arguments as described below, or it can start out empty by passing all
# NULL arguments.
#
# Arguments:
#   attrs: List of <name,type> pairs
#   FDs: List of <lhs,rhs> pairs, lhs and rhs are attribute lists.
#   relationArray: Multi-dimensional array reference parameter.
#       All indeces are of the form (oid,property) where oid uniquely
#       identifies a relation and property is one of: name,attrs,key,fkey.
#       Example: a(R1,name) = Employees
#                a(R1,attrs) = { EmpNo DeptNo DivisionNo EmpName Salary }
#                a(R1,key) = EmpNo
#                a(R1,fkey) = {{DeptNo R2}}
#       The key is a list of attribute names, and fkey (foreign key) is
#       a list of <attr-list,oid> pairs where attr-list is a list of 
#       attributes and oid identifies another source relation. Multiple
#       foreign keys could be defined like this:
#                a(R1,fkey) = {{DeptNo R2} {DivisionNo R3}}
#
proc create_createDB { attrs FDs relationArray } {

    global __CREATE__
    
    upvar $relationArray relations

    if ![info exists __CREATE__(id)] {
	set __CREATE__(id) 0
    }
    
    incr __CREATE__(id)
    set id $__CREATE__(id)

    set bwidth 6
    set listWidth 30
    set listHeight 8
    
    set w .createdb$id
    toplevel $w
    wm title $w "Create a dbnorm Database"
    wm resizable $w 0 0
    wm geometry $w +50+50

    #
    # Frames for toplevel window
    #
    frame $w.menubar -relief raised -bd 1
    set dataframe [frame $w.createdb_data]
    
    #
    # File menu
    #
    menubutton $w.menubar.file -text File \
	    -menu $w.menubar.file.menu
    set filemenu [menu $w.menubar.file.menu -tearoff 0]
    $filemenu add command -label "Save" \
	-command [concat create_fileSave $id]
    $filemenu add command -label "Save As" \
	-command [concat create_fileSaveAs $id]
    $filemenu add separator
    $filemenu add command -label "Exit" \
	-command [concat destroy $w]

    #
    # Help menu
    #
    menubutton $w.menubar.help -text Help \
	    -menu $w.menubar.help.menu
    set helpmenu [menu $w.menubar.help.menu -tearoff 0]
    $helpmenu add command -label "Creating a New DB" \
	    -command {dbn_showHelp createdb}

    #
    # Pack menubar
    #
    pack $w.menubar.file -side left -padx 10
    pack $w.menubar.help -side right -padx 10
    pack $w.menubar -side top -expand true -fill x

    #
    # Data subframes
    #
    frame $dataframe.top -bd 3 -relief ridge
    frame $dataframe.bottom -bd 3 -relief ridge
    set af [frame $dataframe.top.attrs]
    set ff [frame $dataframe.top.fds]
    set rf [frame $dataframe.bottom.relations]
    
    #
    # Create attribute list, and add/delete buttons
    #
    frame $af.top
    frame $af.buttons
    label $af.top.label -text "Database Attributes" -bg black -fg white
    set __CREATE__($id,attrs) [ScrolledListbox $af.list 1 8 \
	    -setgrid true -height $listHeight -width $listWidth \
	    -selectmode extended]
    button $af.buttons.add -text "Add" -width $bwidth \
	    -command [concat create_addAttribute $id]
    button $af.buttons.delete -text "Delete" -width $bwidth \
	    -command [concat create_deleteAttributes $id]
    pack $af.top.label -side top -anchor n -fill x
    pack $af.buttons.add $af.buttons.delete -side left
    pack $af.top -side top -anchor n -fill x
    pack $af.list -side top -anchor nw -fill x
    pack $af.buttons -side bottom
    pack $af -side left -anchor nw
    
    #
    # Create border between attrs and FDs
    #
    frame $dataframe.top.border1 -width 2 -bg gray50
    pack $dataframe.top.border1 -side left -fill y -expand true
    
    #
    # Create FD list, and add/edit/delete buttons
    #
    frame $ff.top
    frame $ff.buttons
    label $ff.top.label -text "Functional Dependencies" -bg black -fg white
    set __CREATE__($id,fds) [ScrolledListbox $ff.list 1 8 \
	    -setgrid true -height $listHeight -selectmode browse \
	    -width $listWidth]
    button $ff.buttons.add -text "Add" -width $bwidth \
	    -command [concat create_addFD $id]
    button $ff.buttons.edit -text "Edit" -width $bwidth \
	    -command [concat create_editFD $id]
    button $ff.buttons.delete -text "Delete" -width $bwidth \
	    -command [concat create_deleteFD $id]
    pack $ff.top.label -side top -anchor n -fill x
    pack $ff.buttons.add $ff.buttons.edit $ff.buttons.delete -side left
    pack $ff.top -side top -anchor n -fill x
    pack $ff.list -side top -anchor nw -fill x
    pack $ff.buttons -side bottom
    pack $ff -side left -anchor nw
    
    #
    # Create Source Relation frame
    #
    frame $rf.top
    frame $rf.left
    frame $rf.right
    frame $rf.keyframe
    frame $rf.border1 -width 2 -height 2 -bg gray50
    frame $rf.border2 -width 2 -height 2 -bg gray50

    set rfn [frame $rf.left.names]
    set rfa [frame $rf.right.attrs]
    set rfk [frame $rf.keyframe.key]
    set rff [frame $rf.keyframe.fkey]
    
    set __CREATE__($id,sourceRelLabel) \
	    [label $rf.top.label -text "Source Relations" \
	    -bg black -fg white]
    pack $rf.top.label -side top -anchor n -fill x

    set __CREATE__($id,relations) [ScrolledListbox $rfn.list 1 8 \
	    -setgrid true -height $listHeight -selectmode single \
	    -width $listWidth]
    frame $rfn.top
    label $rfn.top.label -text "Names:"
    frame $rfn.buttons
    button $rfn.buttons.add -text "Add" -width $bwidth \
	    -command [concat create_addRelation $id]
    button $rfn.buttons.delete -text "Delete" -width $bwidth \
	    -command [concat create_deleteRelation $id]
    pack $rfn.top.label -side top -anchor nw
    pack $rfn.buttons.add $rfn.buttons.delete -side left
    pack $rfn.top -side top -anchor nw
    pack $rfn.list -side top -anchor nw -fill both
    pack $rfn.buttons -side bottom
    pack $rfn -side top -anchor nw -fill x -expand true

    set __CREATE__($id,relAttrs) [ScrolledListbox $rfa.list 1 8 \
	    -setgrid true -height $listHeight -selectmode extended \
	    -width $listWidth]
    frame $rfa.top
    label $rfa.top.label -text "Attributes:"
    pack $rfa.top.label -side top -anchor nw
    frame $rfa.buttons
    frame $rfa.buttons.r1
    button $rfa.buttons.r1.add -text "Add" -width $bwidth \
	    -command [concat create_addRelAttributes $id]
    button $rfa.buttons.r1.delete -text "Delete" -width $bwidth \
	    -command [concat create_deleteRelAttributes $id]
    pack $rfa.buttons.r1.add $rfa.buttons.r1.delete -side left
    pack $rfa.buttons.r1 -side top
    pack $rfa.top $rfa.list -side top -anchor nw -fill x
    pack $rfa.buttons -side bottom
    pack $rfa -side top -anchor nw -fill x -expand true

    set __CREATE__($id,key) [entry $rfk.keytext -relief sunken -bd 2 \
	    -state disabled]
    label $rfk.label -text "Primary Key:"
    button $rfk.button1 -text "Set" -width 3 \
	    -command [concat create_setKey $id]
    button $rfk.button2 -text "Clear" -width 3 \
	    -command [concat create_setKey $id 1]
    pack $rfk.label -side left
    pack $rfk.keytext -side left -fill x -expand true -padx 2
    pack $rfk.button1 $rfk.button2 -side left
    pack $rfk -side top -fill x

    set __CREATE__($id,fkey) [ScrolledListbox $rff.list 1 8 \
	    -setgrid true -height 3 -selectmode browse]
    label $rff.label -text "Foreign Keys:"
    button $rff.button1 -text "Add" -width 3 \
	    -command [concat create_addForeignKey $id]
    button $rff.button2 -text "Delete" -width 3 \
	    -command [concat create_deleteForeignKey $id]
    pack $rff.label -side left
    pack $rff.list -side left -fill x -expand true -padx 2
    pack $rff.button1 $rff.button2 -side left
    pack $rff -side top -fill x
    
    pack $rf.top -side top -anchor n -fill x
    pack $rf.keyframe -side bottom -anchor nw -fill x
    pack $rf.border1 -side bottom -fill x -expand true
    pack $rf.left -side left -anchor nw -fill both -expand true
    pack $rf.border2 -side left -fill y
    pack $rf.right -side left -anchor nw -fill both -expand true
    pack $rf -side top -anchor nw -fill x

    #
    # Pack data frames
    #
    pack $dataframe.top -side top -padx 2 -pady 2
    pack $dataframe.bottom -side top -anchor nw -fill both \
	    -padx 2 -pady 2
    pack $dataframe -side top -anchor nw
    
    #
    # Set state variables
    #
    set __CREATE__($id,toplevel) $w
    set __CREATE__($id,filename) {}
    set __CREATE__($id,activeSource) {}
    set __CREATE__($id,relationOIDs) {}
    set __CREATE__($id,nextOID) 1

    #
    # Insert text into widgets, and set bindings
    #
    bind $w <Destroy> [list create_destroy $id]
    foreach a $attrs {
	$__CREATE__($id,attrs) insert end \
		"[lindex $a 0] \[[lindex $a 1]]"
    }
    foreach fd $FDs {
	create_insertFD $id [lindex $fd 0] [lindex $fd 1]
    }
    set nRelations 0
    foreach key [array names relations *,attrs] {
	set oid [lindex [split $key ,] 0]
	create_insertRelation $id $relations($oid,name) \
		$relations($oid,attrs) $relations($oid,key) \
		$relations($oid,fkey)
	incr nRelations
    }
    if {$nRelations > 0} {
	create_updateSourceRelationFrame $id 0
    }
    set l $__CREATE__($id,relations)
    bindtags $l [list Listbox $l $w all]
    bind $l <Button-1> [concat create_updateSourceRelationFrame $id]
    bind $l <KeyPress> [concat create_updateSourceRelationFrame $id]

}

proc create_destroy { id } {
    global __CREATE__
    foreach n [array names __CREATE__ $id*] {
	unset __CREATE__($n)
    }
}

proc create_addAttribute { id } {
    global __CREATE__
    set attr [getAttr_getAttrAndType]
    if {$attr == {}} { return {} }
    if [create_dupAttrName $id [lindex $attr 0]] {
	one_line_message .message "Error" \
		"Duplicate attribute name: [lindex $attr 0]"
	return
    }
    $__CREATE__($id,attrs) insert end \
	    "[lindex $attr 0] \[[lindex $attr 1]]"
}

proc create_deleteAttributes { id } {
    global __CREATE__
    if {[$__CREATE__($id,attrs) curselection] == {}} {
	return
    }
    set attrPairs [get_selected_lines $__CREATE__($id,attrs)]
    set confirm [dialog .confirm "Delete Attributes" \
	    [join [list "Are you sure you want to delete these attributes?" \
	    "They will be removed from all existing relations and FDs."] \n] \
	    "" 0 {Cancel} {Delete}]
    if {$confirm == 0} { return }
    delete_selected_lines $__CREATE__($id,attrs)
    set attrs {}
    foreach pair $attrPairs {
	lappend attrs [lindex $pair 0]
    }
    create_deleteAttrsFromRelations $id $attrs
    create_deleteAttrsFromFDs $id $attrs
    create_updateSourceRelationFrame $id $__CREATE__($id,activeSource)
}

proc create_insertFD { id lhs rhs {overwriteIndex {}} } {
    global __CREATE__
    if {$overwriteIndex != {}} {
	$__CREATE__($id,fds) delete $overwriteIndex
    } else {
	set overwriteIndex end
    }
    $__CREATE__($id,fds) insert $overwriteIndex \
	    [concat [join $lhs ,] -> [join $rhs ,]]
}

proc create_addFD { id } {
    global __CREATE__
    set attrs {}
    foreach a [$__CREATE__($id,attrs) get 0 end] {
	lappend attrs [lindex $a 0]
    }
    set fd [fdedit_editFD $attrs {} {}]
    if {$fd == {}} {
	return
    }
    create_insertFD $id [lindex $fd 0] [lindex $fd 1]
}

proc create_deleteFD { id } {
    global __CREATE__
    if {[$__CREATE__($id,fds) curselection] == {}} {
	return
    }
    set confirm [dialog .confirm "Delete FDs" \
	    "Are you sure you want to delete the selected FDs?" \
	    "" 0 {Cancel} {Delete}]
    if {$confirm == 0} { return }
    delete_selected_lines $__CREATE__($id,fds)
}

proc create_editFD { id } {
    global __CREATE__
    set l $__CREATE__($id,fds)
    set i [$l curselection]
    if {$i == {}} {
	return
    }
    set fdText [$l get $i]
    set lhs [split [lindex $fdText 0] ,]
    set rhs [split [lindex $fdText 2] ,]
    set allAttrs {}
    foreach a [$__CREATE__($id,attrs) get 0 end] {
	lappend allAttrs [lindex $a 0]
    }
    set newFD [fdedit_editFD $allAttrs $lhs $rhs]
    if {$newFD == {}} {
	return
    }
    create_insertFD $id [lindex $newFD 0] [lindex $newFD 1] $i
}

proc create_addRelation { id } {
    global __CREATE__
    set l $__CREATE__($id,relations)
    set r [input_box .inputbox "New Relation" \
	"Enter the new relation name:"]
    if {$r == {}} {
	return {}
    }
    set r [join $r _]
    if {[lsearch [$l get 0 end] $r] != -1} {
	one_line_message .message "Error" "Duplicate relation name: $r"
	return
    }
    create_insertRelation $id $r {} {} {}
    set nRelations [llength $__CREATE__($id,relationOIDs)]
    create_updateSourceRelationFrame $id [expr $nRelations - 1]
}

proc create_deleteRelation { id } {
    
    global __CREATE__
    
    set l $__CREATE__($id,relations)
    set i [$l curselection]
    if {$i == {}} {
	return
    }
    
    set confirm [dialog .confirm "Delete Relations" \
	    "Are you sure you want to delete the selected relation?" \
	    "" 0 {Cancel} {Delete}]
    if {$confirm == 0} { return }
    
    $l delete $i
    set oid [lindex $__CREATE__($id,relationOIDs) $i]
    foreach n [array names __CREATE__ $id,$oid,*] {
	unset __CREATE__($n)
    }
    set __CREATE__($id,relationOIDs) \
	    [lreplace $__CREATE__($id,relationOIDs) $i $i]
    
    if {[$l get 0 0] != {}} {
	create_updateSourceRelationFrame $id 0
    } else {
	create_clearSourceRelationFrame $id
    }

}

proc create_addRelAttributes { id } {
    
    global __CREATE__
    
    set active $__CREATE__($id,activeSource)
    if {$active == {}} {
	return
    }
    set oid [lindex $__CREATE__($id,relationOIDs) $active]
    
    set attrs {}
    foreach i [$__CREATE__($id,attrs) curselection] {
	lappend attrs [lindex [$__CREATE__($id,attrs) get $i] 0]
    }
    if {$attrs == {}} {
	return
    }

    add_lines_to_list $attrs $__CREATE__($id,relAttrs)
    
    set __CREATE__($id,$oid,attrs) [$__CREATE__($id,relAttrs) get 0 end]

}

proc create_deleteRelAttributes { id } {
    global __CREATE__
    if {[$__CREATE__($id,relAttrs) curselection] == {}} {
	return
    }
    set confirm [dialog .confirm "Delete Attributes" \
	    "Are you sure you want to delete the selected attributes?" \
	    "" 0 {Cancel} {Delete}]
    if {$confirm == 0} { return }
    delete_selected_lines $__CREATE__($id,relAttrs)
}

proc create_updateSourceRelationFrame { id {new {}} } {

    global __CREATE__
    
    set old $__CREATE__($id,activeSource)

    if {$new == {}} {
	set new [$__CREATE__($id,relations) curselection]
	if {$new == $old || $new == {}} {
	    return
	}
    }
    
    set __CREATE__($id,activeSource) $new
    set oid [lindex $__CREATE__($id,relationOIDs) $new]
    $__CREATE__($id,sourceRelLabel) config -text \
	    "Source Relation: $__CREATE__($id,$oid,name)"
    $__CREATE__($id,relAttrs) delete 0 end
    eval $__CREATE__($id,relAttrs) insert end $__CREATE__($id,$oid,attrs)
    $__CREATE__($id,key) config -state normal
    $__CREATE__($id,key) delete 0 end
    $__CREATE__($id,key) insert 0 [join $__CREATE__($id,$oid,key) ,]
    $__CREATE__($id,key) config -state disabled
    set fkey $__CREATE__($id,$oid,fkey)
    $__CREATE__($id,fkey) delete 0 end
    if {$fkey != {}} {
	foreach fk $fkey {
	    $__CREATE__($id,fkey) insert end \
		[concat [join [lindex $fk 0] ,] REFERENCES \
		$__CREATE__($id,[lindex $fk 1],name)]
	}
    }

    $__CREATE__($id,relations) selection clear 0 end
    $__CREATE__($id,relations) selection set $new
    
}

proc create_clearSourceRelationFrame { id } {

    global __CREATE__
    
    set __CREATE__($id,activeSource) {}

    $__CREATE__($id,sourceRelLabel) config -text "Source Relations"

    $__CREATE__($id,relations) delete 0 end
    $__CREATE__($id,relAttrs) delete 0 end
    $__CREATE__($id,key) config -state normal
    $__CREATE__($id,key) delete 0 end
    $__CREATE__($id,key) config -state disabled
    $__CREATE__($id,fkey) delete 0 end
    
}

proc create_setKey { id {clear 0} } {
    global __CREATE__
    set active $__CREATE__($id,activeSource)
    if {$active == {}} {
	return
    }
    set oid [lindex $__CREATE__($id,relationOIDs) $active]
    if $clear {
	set __CREATE__($id,$oid,key) {}
	$__CREATE__($id,key) config -state normal
	$__CREATE__($id,key) delete 0 end
	$__CREATE__($id,key) config -state disabled
	return
    }
    set attrs [get_selected_lines $__CREATE__($id,relAttrs)]
    if {$attrs == {}} {
	return
    }
    set __CREATE__($id,$oid,key) $attrs
    $__CREATE__($id,key) config -state normal
    $__CREATE__($id,key) delete 0 end
    $__CREATE__($id,key) insert 0 [join $attrs ,]
    $__CREATE__($id,key) config -state disabled
}

proc create_addForeignKey { id } {

    global __CREATE__
    
    set active $__CREATE__($id,activeSource)
    if {$active == {}} {
	return
    }
    
    set oid [lindex $__CREATE__($id,relationOIDs) $active]
    
    set attrs [get_selected_lines $__CREATE__($id,relAttrs)]
    if {$attrs == {}} {
	return
    }

    set rnames {}
    foreach r $__CREATE__($id,relationOIDs) {
	lappend rnames $__CREATE__($id,$r,name)
    }
    set choice [SelectionBox \
	    "Which relation does this foreign key reference?" \
	    $rnames "Make a Selection"]
    if {$choice == {}} {
	return
    }
    
    set other [lindex $__CREATE__($id,relationOIDs) $choice]
    set otherName $__CREATE__($id,$other,name)
    set otherAttrs $__CREATE__($id,$other,attrs)

    foreach a $attrs {
	if {[lsearch $otherAttrs $a] == -1} {
	    one_line_message .error "Error" \
		"Attribute '$a' is not in relation '$otherName'"
	    return
	}
    }

    lappend __CREATE__($id,$oid,fkey) [list $attrs $other]
    $__CREATE__($id,fkey) insert end \
	"[join $attrs ,] REFERENCES $otherName"

}

proc create_deleteForeignKey { id } {

    global __CREATE__
    
    set active $__CREATE__($id,activeSource)
    if {$active == {}} {
	return
    }
    
    set i [$__CREATE__($id,fkey) curselection]
    if {$i == {}} {
	return
    }
    set confirm [dialog .confirm "Delete FDs" \
	    "Are you sure you want to delete the selected foreign key?" \
	    "" 0 {Cancel} {Delete}]
    if {$confirm == 0} { return }
    
    set oid [lindex $__CREATE__($id,relationOIDs) $active]
    
    $__CREATE__($id,fkey) delete $i
    set __CREATE__($id,$oid,fkey) [lreplace \
	$__CREATE__($id,$oid,fkey) $i $i]

}

proc create_insertRelation { id rname attrs key fkey } {
    global __CREATE__
    set oid R$__CREATE__($id,nextOID)
    incr __CREATE__($id,nextOID)
    set __CREATE__($id,$oid,name) $rname
    set __CREATE__($id,$oid,attrs) $attrs
    set __CREATE__($id,$oid,key) $key
    set __CREATE__($id,$oid,fkey) $fkey
    $__CREATE__($id,relations) insert end $rname
    lappend __CREATE__($id,relationOIDs) $oid
}

proc create_writeToFile { id filename } {
    
    global __CREATE__
    
    if [catch {open $filename w} f] {
	one_line_message .error "Error" \
		"Error opening '[file tail $filename]'. No data was saved."
	return 0
    }

    puts $f ""

    puts $f "% Database"

    #
    # List of source relations
    #
    puts $f "SourceRelations = $__CREATE__($id,relationOIDs)"
    
    #
    # Attributes
    #
    foreach attrPair [$__CREATE__($id,attrs) get 0 end] {
	puts $f [list Attribute = [lindex $attrPair 0] \
		[string trim [lindex $attrPair 1] "\[]"]]
    }

    #
    # FDs
    #
    foreach fd [$__CREATE__($id,fds) get 0 end] {
	set lhs [split [lindex $fd 0] ,]
	set rhs [split [lindex $fd 2] ,]
	puts $f "FD = [list $lhs] [list $rhs]"
    }
    
    #
    # Relations
    #
    foreach oid $__CREATE__($id,relationOIDs) {
	puts $f "\n% Relation"
	puts $f "ID = $oid"
	puts $f "Name = $__CREATE__($id,$oid,name)"
	puts $f "Attributes = $__CREATE__($id,$oid,attrs)"
	puts $f "Key = $__CREATE__($id,$oid,key)"
	foreach fk $__CREATE__($id,$oid,fkey) {
	    puts $f "Foreign Key = [list [lindex $fk 0]] [lindex $fk 1]"
	}
    }

    close $f
    return 1

}

proc create_fileSave { id } {
    global __CREATE__
    set f $__CREATE__($id,filename) 
    if {$f == {}} {
	create_fileSaveAs $id
	return
    }
    create_writeToFile $id $f
}

proc create_fileSaveAs { id } {
    
    global __CREATE__
    
    set f [fileselect "File Selection" {} 0 dbn]
    if {$f == {}} {
	return
    }
    
    if [file exists $f] {
	set overwrite [dialog .fileExists "File Exists" \
		"'[file tail $f]' already exists. Overwrite?" "" \
		0 No Yes]
	if {$overwrite == 0} {
	    return
	}
    }

    set __CREATE__($id,filename) $f
    create_fileSave $id
}

proc create_deleteAttrsFromRelations { id attrs } {
    global __CREATE__
    foreach oid $__CREATE__($id,relationOIDs) {
	set __CREATE__($id,$oid,attrs) \
		[set_difference $__CREATE__($id,$oid,attrs) $attrs]
	set __CREATE__($id,$oid,key) \
		[set_difference $__CREATE__($id,$oid,key) $attrs]
	set fkAttrs [set_difference \
		[lindex $__CREATE__($id,$oid,fkey) 0] $attrs]
	if {$fkAttrs == {}} {
	    set __CREATE__($id,$oid,fkey) {}
	} else {
	    set __CREATE__($id,$oid,fkey) [list $fkAttrs \
		    [lindex $__CREATE__($id,$oid,fkey) 0]]
	}
    }
}

proc create_deleteAttrsFromFDs { id attrs } {
    global __CREATE__
    set l $__CREATE__($id,fds)
    set allFDs [$l get 0 end]
    $l delete 0 end
    foreach fd $allFDs {
	set lhs [split [lindex $fd 0] ,]
	set rhs [split [lindex $fd 2] ,]
	set newlhs [set_difference $lhs $attrs]
	set newrhs [set_difference $rhs $attrs]
	if {$newlhs != {} && $newrhs != {}} {
	    create_insertFD $id $newlhs $newrhs
	}
    }
}

proc create_dupAttrName { id attr } {
    global __CREATE__
    set attr [string toupper $attr]
    foreach attrPair [$__CREATE__($id,attrs) get 0 end] {
	if {$attr == [string toupper [lindex $attrPair 0]]} {
	    return 1
	}
    }
    return 0
}

proc create_test {} {
    
    set attrs [list \
	    {A Int} {B Int} {C Int} {D Int} {E Int} {F Int} {G Int} \
	    {H Int} {I Int} {J Int} {K Int} {L Int} {M Int} {N Int} \
	    {O Int} {P Int} {Q Int} {R Int}]

    
    set FDs [list \
	    {{A B} {C D E F}} \
	    {{D} {G H I}} \
	    {{G} {H J K L}} \
	    {{B} {Q R}}]
    
    set R(R1,name) Relation1
    set R(R1,attrs) {A B C D E F G}
    set R(R1,key) {A B}
    set R(R1,fkey) {{A R2} {{B Q} R3}}

    set R(R2,name) Relation2
    set R(R2,attrs) {A H J K L}
    set R(R2,key) {A}
    set R(R2,fkey) {{{B Q} R3}}
    
    set R(R3,name) Relation3
    set R(R3,attrs) {B Q R}
    set R(R3,key) {B}
    set R(R3,fkey) {}
    
    create_createDB $attrs F R

}

