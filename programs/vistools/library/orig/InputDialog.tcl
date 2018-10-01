# InputDialog.tcl - dialog to display a message and get input from the user
#                   (based on the Dialog class)
#
# Author: Allan Brighton (allan@piano.sta.sub.org)


itcl_class InputDialog {
    inherit Dialog


    # create the dialog

    constructor {config} {
	Dialog::constructor

	# add an entry widget below the message
	global $this.choice
	pack [entry $this.entry] \
		-in $this.top.ext -side left -fill x -expand 1 \
		-padx 2m -pady 2m -ipady 1m
	bind $this.entry <Return> \
		"$this.button$default flash; set $this.choice $default"
    }


    # this method is redefined here to change the value
    # that is returned in $variable to be the contents of the entry widget

    method set_result {} {
	global $this.choice
	if {[set $this.choice] == $default} {
	    return [$this.entry get]
	}
	return {}
    }
}

