# LabelMessage.tcl - Itcl widget displaying a label with a message
#
# Author: Allan Brighton (allan@piano.sta.sub.org) 


itcl_class LabelMessage {
    inherit FrameWidget

    #  constructor: create a new LabelMessage
    constructor {config} {
	FrameWidget::constructor

	#  Pack widgets into this window
	pack [label $this.label] \
	    -side left
	pack [entry $this.entry -relief flat -state disabled] \
	    -side left -expand 1 -fill x -padx 5

	$this.entry config -relief flat

	#  Explicitly handle config's that may have been ignored earlier
	foreach attr $config {
	    config -$attr [set $attr]
	}
    }


    # -- public variables -- 

    # set the text for the label
    public text {} {
	if {[winfo exists $this.label]} {
	    $this.label config -text $text
	}
    }

    # set the text for the message
    public value {} {
	if {[winfo exists $this.entry]} {
	    $this.entry config -state normal
	    $this.entry delete 0 end
	    $this.entry insert 0 $value
	    $this.entry config -state disabled
	}
    }
}

