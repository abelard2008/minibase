# Choice.tcl - itcl megawidget for choosing options
#                 using a label and radiobuttons
#
# Author: Allan Brighton (allan@piano.sta.sub.org)
#


itcl_class Choice {
    inherit FrameWidget

    
    # return the current value of this option

    method get_value {} {
	global $this.choice
	return [set $this.choice]
    }

    
    # set the value for this option

    method set_value {s} {
	global $this.choice
	set $this.choice $s
	set value $s
    }


    constructor {config} {
	FrameWidget::constructor
	global $this.choice

	if {"$label" != ""} {
	    pack [label $this.label -text $label] \
		    -side $side
	}

	# append cur value to command
	if {"$command" != ""} {
	    append command [format { [set %s]} $this.choice $this.choice]
	}

	set n 0
	foreach i $choice {
	    pack [radiobutton $this.choice$n \
		    -text $i \
		    -value $i \
		    -variable $this.choice \
		    -command $command \
		    -state $state] \
		    -side $side -fill x -expand 1
	    incr n
	}
	set $this.choice [lindex $choice 0]
    }


    # public member variables


    # label for option
    public label {}

    # value to display
    public value {} {
	global $this.choice
	set $this.choice $value
    }

    # list of choices
    public choice {}

    # command to run when value is changed
    public command {}

    # set horizontal or vertical
    public layout {horizontal} {
	if {"$layout" == "horizontal"} {
	    set side left
	} else {
	    set side top
	}
    }

    # button states: normal, disabled
    public state {normal} {
	if {[winfo exists $this]} {
	    set n [llength $choice]
	    for {set i 0} {$i < $n} {incr i} {
		$this.choice$i config -state $state
	    }
	}
    }


    # protected members
    
    # pack option
    protected side {left}
}

