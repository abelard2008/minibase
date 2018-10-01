# Dialog.tcl - itcl widget for creating dialog windows
#              (modified from tk_dialog)
#
# A dialog window here is a window with a message at top and a row 
# of buttons at bottom. 
# Through inheritance, you can add widgets inbetween...
#
# Usage: Dialog .d -text "message" [options]
# (see public variables below for options)
# 
# Author: Allan Brighton (allan@piano.sta.sub.org)


itcl_class Dialog {
    inherit TopLevelWidget


    # create the dialog

    constructor {config} {
	TopLevelWidget::constructor
	global $this.choice

	wm title $this $title
	wm iconname $this Dialog
	wm minsize $this 10 10 

	# the top frame has one frame for the message and bitmap
	# and another for extensions defined in derived classes
	pack \
		[frame $this.top -relief raised -bd 2] \
		[frame $this.top.def -bd 3] \
		[frame $this.top.ext -bd 3] \
		-side top -fill both -expand 1

	if {[string length $text] >= $scroll_threshold} {
	    pack [Canvas $this.msg \
		    -width 5i -height 2i \
		    -relief flat -bd 12 \
		    -bg [utilGetConfigValue $this.top -bg]] \
		    -in $this.top.def -side right \
		    -expand 1 -fill both
	    $this.msg frameconfig -bd 3 -relief sunken
	    $this.msg.canvas create text 0 0 \
		    -text $text -font $font
	} else {
	    pack [message $this.msg -width 3i -text $text -font $font] \
		    -in $this.top.def -side right -expand 1 \
		    -fill both -padx 5m -pady 5m
	}

	if {$bitmap != ""} {
	    pack [label $this.bitmap -bitmap $bitmap] \
		    -in $this.top.def -side left -padx 5m -pady 5m
	}

	# Create a row of buttons at the bottom of the dialog.

	pack [frame $this.bot -relief raised -bd 2] \
		-side bottom -fill x
	set i 0
	foreach but $buttons {
	    button $this.button$i -text $but -command "set $this.choice $i"
	    if {$i == $default} {
		frame $this.default -relief sunken -bd 1
		raise $this.button$i $this.default
		pack $this.default -in $this.bot -side left -expand 1 -padx 3m -pady 2m
		pack $this.button$i -in $this.default -padx 2m -pady 2m \
			-ipadx 2m -ipady 1m
		bind $this <Return> "$this.button$i flash; set $this.choice $i"
	    } else {
		pack $this.button$i -in $this.bot -side left -expand 1 \
			-padx 3m -pady 3m -ipadx 2m -ipady 1m
	    }
	    incr i
	}
    }

    
    # Display the window and return the user's selection

    method activate {} {
	global $this.choice

	# Set a grab and claim the focus.

	set oldFocus [focus]
	grab $this
	focus $this

	# Wait for the user to respond, then restore the focus

	tkwait variable $this.choice
	set result [virtual set_result]
	unset $this.choice
	$this delete
	focus $oldFocus
	return $result
    }


    # this method may be redefined in a derived class to change the value
    # that is returned in $variable (default is the number of the button selected)

    method set_result {} {
	global $this.choice
	return [set $this.choice]
	
    }


    # -- public variables --

    # Title to display in dialog's decorative frame.
    public title {dialog}

    # Message to display in dialog.
    public text {}

    # Bitmap to display in dialog (empty string means none).
    public bitmap {warning}

    # Index of button that is to display the default ring (-1 means none).
    public default {0}
    
    # One or more strings to display in buttons across the
    # bottom of the dialog box.
    public buttons {OK}

    # font to use
    public font {-Adobe-Times-Medium-R-Normal-*-180-*}

    # if the message is longer than this, display a scrollbar
    public scroll_threshold {200}
}

