
proc input_box { w title text {default {}} } {

    global __INP__button
    global __INP__default
    
    set __INP__default $default
    
    # 1. Create the top-level window and divide it into top
    # and bottom parts.

    toplevel $w -class InputBox
    wm geometry $w +300+200    
    wm title $w $title
    wm iconname $w InputBox
    frame $w.top -relief flat -bd 1
    pack $w.top -side top -fill both -padx 5 -pady 3
    frame $w.bot -relief flat -bd 1
    pack $w.bot -side bottom -fill both

    # 2. Fill the top part with the message and an entry box.

    label $w.top.msg -justify left -text $text    
    entry $w.top.entry -textvariable __INP__default
    pack $w.top.msg -side top -expand 1 -anchor w -padx 3
    pack $w.top.entry -side top -expand 1 -fill both -padx 3 -pady 2

    # 3. Create a row of buttons at the bottom of the dialog.
    
    button $w.bot.ok_button -text OK -command {set __INP__button 1}
    frame $w.bot.default -relief sunken -bd 1
    raise $w.bot.ok_button
    pack $w.bot.default -side left -expand 1 -fill x -padx 3m -pady 2m
    pack $w.bot.ok_button -in $w.bot.default -side left \
	    -expand 1 -fill x -padx 1m -pady 1m

    button $w.bot.cancel_button -text Cancel -command {set __INP__button 0}
    pack $w.bot.cancel_button -side left -expand 1 -fill x \
	    -padx 3m -pady 3m -ipadx 2m -ipady 1m
    
    # 4. Set up a binding for <Return>, if there`s a default,
    # set a grab, and claim the focus too.
    
    bind $w <Return> "$w.bot.ok_button flash; set __INP__button 1"    
    bind $w <Destroy> "set __INP__button 0"
    set oldFocus [focus]
    tkwait visibility $w
    grab set $w
    focus $w.top.entry
    
    # 5. Wait for the user to respond, then restore the focus
    # and return the index of the selected button.
    
    tkwait variable __INP__button
    if $__INP__button {
	set return_value [string trim [$w.top.entry get]]
    } else {
	set return_value {}
    }
    if [winfo exists $w] { destroy $w }
    focus $oldFocus
    return $return_value
}
