# CheckEntry.tcl - itcl widget combining a checkbutton and an entry
#
# Author: Allan Brighton (allan@piano.sta.sub.org) 


itcl_class CheckEntry {
    inherit FrameWidget

    #  Get the value in the entry

    method get {} {
	global $this.var
	if {[set $this.var]} {
	    return [$this.entry get]
	}
	return {}
    }

    
    # return 1 if the checkbox is selected, otherwise 0

    method var {} {
	global $this.var
	return [set $this.var]
    }
   

    #  called for Return in the entry

    method _action_proc {} {
	if {"$action" != ""} {
	    set a $action
	    lappend a [$this get]
	    eval $a
	}
    }
   

    #  constructor: create new CheckEntry

    constructor {config} {
	FrameWidget::constructor
	global $this.var

	pack [checkbutton $this.check -text "$text" \
		  -variable $this.var ] \
	    -side left -padx 1m -pady 2m -ipadx 1m -ipady 1m
	pack [entry $this.entry -relief sunken] \
	    -side left -expand 1 -fill x -padx 2m -pady 2m -ipadx 2m -ipady 1m

	bind $this.entry <1> "[bind Entry <1>]; $this.check select"

	#  Explicitly handle config's that may have been ignored earlier
	foreach attr $config {
	    config -$attr [set $attr]
	}
    }


    # -- public variables --


    # the text of the label
    public text {} {
	if {[winfo exists $this.check]} {
	    $this.check config -text $text
	}
    }

    # set the value in the entry
    # some code is added here to ensure that the last part of the string is visible
    public value {} {
	if {[winfo exists $this.entry]} {
	    global $this.var
	    set $this.var 1
	    $this.entry delete 0 end
	    $this.entry insert 0 $value
	    $this.entry icursor end
	    set i [expr [$this.entry index end] \
		       - [utilGetConfigValue $this.entry -width] - 4] 
	    $this.entry view $i
	}
    }

    
    # the action for <Return> in the entry
    public action {} {
	if {[winfo exists $this.entry]} {
	    bind $this.entry <Return> "$this _action_proc"
	}
    }
}

