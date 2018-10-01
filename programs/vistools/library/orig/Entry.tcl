# Entry.tcl - Itcl widget for displaying a labeled entry
#
# Author: Allan Brighton (allan@piano.sta.sub.org) 


itcl_class Entry {
    inherit FrameWidget


    #  Get the value in the entry

    method get {} {
	return [$this.entry get]
    }
   

    # called for traces on textvariable

    method trace_ {args} {
	if {!$notrace} {
	    _action_proc $changecmd
	}
    }


    #  called for for return or keypress in entry, calls action proc with new value

    method _action_proc {cmd} {
	lappend cmd [$this.entry get]
	eval $cmd
    }


    # constructor: create a new Entry widget

    constructor {config} {
	FrameWidget::constructor

	pack [label $this.label] \
	    -side $side -padx 1m -pady 2m -ipadx 1m -ipady 1m
	pack [entry $this.entry -relief sunken] \
	    -side $side -expand 1 -fill x -padx 1m -ipadx 1m -ipady 1m

	#  Explicitly handle config's that may have been ignored earlier
	foreach attr $config {
	    config -$attr [set $attr]
	}
    }

   
    # -- public variables --


    # set the text entry's label
    public text {} {
	if {[winfo exists $this.label]} {
	    $this.label config -text $text
	}
    }

    # set the value in the entry and make sure the end of the string is visible
    public value {} {
	if {[winfo exists $this.entry]} {
	    set prev_state [utilGetConfigValue $this.entry -state]
	    $this.entry config -state normal
	    set notrace 1
	    $this.entry delete 0 end
	    $this.entry insert 0 $value
	    $this.entry icursor end
	    set i [expr [$this.entry index end] \
		       - [utilGetConfigValue $this.entry -width] - 4] 
	    $this.entry view $i
	    $this.entry config -state $prev_state
	    set notrace 0
	    
	}
    }
    
    # set the entry's state (normal, disabled)
    public state {normal} {
	if {[winfo exists $this.entry]} {
	    $this.entry config -state $state
	}
    }
    
    # the action for <Return> in the entry, called with the new value
    public action {} {
	if {[winfo exists $this.entry]} {
	    bind $this.entry <Return> [list $this _action_proc $action]
	}
    }

    
    # pack option: use left for horizontal orient, top for vert.
    public side {left}

    
    # commands to evaluate whenever the entry value changes
    public changecmd {} {
	if {[winfo exists $this.entry]} {
	    global $this
	    $this.entry config -textvariable $this
	    trace variable $this w "$this trace_"
	}
    }


    # -- protected vars --

    # this flag is set to 1 to avoid tracing when changing the entry's value
    protected notrace 0
}

