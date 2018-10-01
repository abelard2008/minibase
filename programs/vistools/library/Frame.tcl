# FrameWidget.tcl - itcl base class for widgets with their own frame
#
# Author: Allan Brighton (allan@piano.sta.sub.org) 


itcl_class FrameWidget {

    # Create a frame with the same name as this object
    # Options for the frame may be specified in args

    constructor {args} {
	set class [$this info class]
	::rename $this $this-tmp-
	eval "frame $this -class $class $args"
	::rename $this $this-win
	::rename $this-tmp- $this

	# catch destroy and remove the object's command, therefore
	# removing the object
	bind $this <Destroy> "\
		[bind [winfo class $this] <Destroy>] ;
		[bind $this <Destroy>] ;
		rename $this-win {}"
    }

    #  used to change public attributes

    method config {config} {}


    # pass these options explicitly to the frame

    method frameconfig {args} {
	eval "$this-win configure $args"
    }


    #  destroy window containing widget
    
    destructor {
	destroy $this
    }


    # run the given tcl command while displaying the busy cursor

    method busy {cmd} {
	set w [winfo toplevel $this]
	if {[incr busy_count] == 1} {
	    blt_busy hold $w
	    #focus none
	    update idletasks
	}
	set status [catch [list uplevel $cmd] msg]
	if {[incr busy_count -1] == 0} {
	    blt_busy release $w
	}
	if {$status} {
	    error $msg
	}
    }

    # protected variables


    # count used for busy cursor
    protected busy_count {0}
}
