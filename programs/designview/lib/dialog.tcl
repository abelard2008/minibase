
#
#  This file based on John Ousterhout's Tcl/Tk book and modified
#  locally.
#

set __DLG__width 5i

proc one_line_message {w title text} {
    global __DLG__width
    set old_width $__DLG__width
    set __DLG__width 12i
    dialog $w $title $text "" 0 {OK}
    set __DLG__width $old_width
}

proc dialog {w title text bitmap default args} {

    global __DLG__button
    global __DLG__width

    set __DLG__button {}

    # 1. Create the top-level window and divide it into top
    # and bottom parts.

    toplevel $w -class Dialog

    # modification:
    wm geometry $w +300+200
    
    wm title $w $title
    wm iconname $w Dialog
    frame $w.top -relief raised -bd 1
    pack $w.top -side top -fill both
    frame $w.bot -relief raised -bd 1
    pack $w.bot -side bottom -fill both

    # 2. Fill the top part with the bitmap and message.

    # modification:
    message $w.top.msg -justify center -width $__DLG__width -text $text
    
    pack $w.top.msg -side right -expand 1 -fill both \
	    -padx 3m -pady 3m

    if {$bitmap != ""} {
	label $w.top.bitmap -bitmap $bitmap
	pack $w.top.bitmap -side left -padx 3m -pady 3m
    }
    
    # 3. Create a row of buttons at the bottom of the dialog.
    
    set i 0
    foreach but $args {
	
	button $w.bot.button$i -text $but -command\
		"set __DLG__button $i"
	
	if {$i == $default} {
	    
	    frame $w.bot.default -relief sunken -bd 1

	    # modification:
	    raise $w.bot.button$i

	    if {[llength $args] > 1} {
		pack $w.bot.default -side left \
			-expand 1 -fill x -padx 3m -pady 2m
		pack $w.bot.button$i -in $w.bot.default -side left \
			-expand 1 -fill x -padx 1m -pady 1m
	    } else {
		pack $w.bot.default -side top -padx 3m -pady 2m
		pack $w.bot.button$i -in $w.bot.default -side top \
			-padx 1m -pady 1m
	    }
	    
	} else {
	    
	    if {[llength $args] > 1} {
		pack $w.bot.button$i -side left -expand 1 -fill x \
			-padx 3m -pady 3m -ipadx 2m -ipady 1m
	    } else {
		pack $w.bot.button$i -side top \
			-padx 3m -pady 3m -ipadx 2m -ipady 1m
	    }
	}
	
	incr i
    }
    
    # 4. Set up a binding for <Return>, if there`s a default,
    # set a grab, and claim the focus too.
    
    if {$default >= 0} {
	bind $w <Return> "$w.bot.button$default flash; \
		set __DLG__button $default"
    }
    bind $w <Destroy> "set __DLG__button {}"
    
    set oldFocus [focus]
    tkwait visibility $w
    grab set $w
    focus $w
    
    # 5. Wait for the user to respond, then restore the focus
    # and return the index of the selected button.
    
    tkwait variable __DLG__button
    set return_value $__DLG__button
    if [winfo exists $w] { destroy $w }
    focus $oldFocus
    return $return_value
}
