
proc getAttr_getAttrAndType {} {

    global __GETATTR__name
    global __GETATTR__type
    global __GETATTR__length
    global __GETATTR__list
    global __GETATTR__result

    set __GETATTR__name {}
    set __GETATTR__type {}
    set __GETATTR__length {}
    set __GETATTR__list {}
    if [info exists __GETATTR__result] { unset __GETATTR__result }

    set w .getAttr
    catch {destroy $w}

    toplevel $w
    wm title $w "New Attribute"
    wm resizable $w 0 0
    
    frame $w.header
    frame $w.name
    frame $w.type
    frame $w.length
    frame $w.buttons
    
    label $w.header.label -text "Enter information for the new attribute:"
    pack $w.header.label -pady 7 -padx 10
    
    label $w.name.label -text "Name:" -anchor w -width 7
    label $w.type.label -text "Type:" -anchor w -width 7
    label $w.length.label -text "Length:" -anchor w -width 7

    entry $w.name.entry -textvariable __GETATTR__name
    pack $w.name.label -side left -anchor nw
    pack $w.name.entry -side left -anchor nw -fill x -expand true

    set l [ScrolledListbox $w.type.listFrame 1 8 -selectmode single \
	    -setgrid true]
    set __GETATTR__list $l
    $l insert end Int Float Char Date Datetime Decimal Interval Money
    pack $w.type.label -side left -anchor nw
    pack $w.type.listFrame -side left -anchor nw -expand true -fill x

    entry $w.length.entry -textvariable __GETATTR__length
    pack $w.length.label -side left -anchor nw
    pack $w.length.entry -side left -anchor nw -fill x -expand true

    button $w.buttons.ok -text OK -width 10 -command getAttr_okCommand
    button $w.buttons.cancel -text "Cancel" -width 10 -command \
	    [concat set __GETATTR__result {{}}]
    pack $w.buttons.ok $w.buttons.cancel -side left -pady 5 -padx 5

    pack $w.header $w.name $w.type $w.length -anchor nw -fill x -expand true
    pack $w.buttons -side bottom -padx 5

    bind $w <Destroy> "set __GETATTR__result {}"

    tkwait visibility $w
    grab set $w
    focus $w
    
    tkwait variable __GETATTR__result
    set result $__GETATTR__result

    catch {destroy $w}
    
    return $result
    
}

proc getAttr_okCommand {} {
    
    global __GETATTR__name
    global __GETATTR__type
    global __GETATTR__length
    global __GETATTR__list
    global __GETATTR__result

    if {$__GETATTR__name == {}} {
	one_line_message .message Error "No name was specified"
	return
    }
    
    set i [$__GETATTR__list curselection]
    if {$i == {}} {
	one_line_message .message Error "No type was specified"
	return
    }
    
    set type [$__GETATTR__list get $i]
    if {$type == "Char" && $__GETATTR__length == {}} {
	one_line_message .message Error "No length was specified"
	return
    }
    
    if {$type == "Char"} {
	set type "Char($__GETATTR__length)"
    }

    set __GETATTR__result [list [join $__GETATTR__name _] $type]

}

