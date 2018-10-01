# ButtonFrame.tcl - itcl widget for displaying a frame with the standard
#                 buttons like "OK Cancel Help", with options for
#                 configuring other labels and actions
#
# Author: Allan Brighton (allan@piano.sta.sub.org)
#


itcl_class ButtonFrame {
    inherit FrameWidget


    # add a button to the row 

    method append {label cmd} {
	pack [button $this.but$seq -text $label -command $cmd] \
		-side left -fill x -expand 1 -padx 1m -pady 1m -ipadx 1m -ipady 1m
	incr seq
    }

    
    # create a row with 3 buttons

    constructor {config} {
	FrameWidget::constructor -relief raised -bd 1

	if {"$cancel_cmd" == ""} {
	    set cancel_cmd "destroy [winfo parent $this]"
	}

	foreach i "ok cancel help" {
	    append [set ${i}_label] [set ${i}_cmd]
	}
    }


    # public member variables

    # labels and commands for the standard buttons
    public ok_label {OK}
    public ok_cmd {}

    public cancel_label {Cancel}
    public cancel_cmd {}

    public help_label {Help}
    public help_cmd {}


    # protected vars

    # counter for button names
    protected seq {0}
}

