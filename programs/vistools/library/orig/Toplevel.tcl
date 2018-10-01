# TopLevelWidget.tcl - itcl base class for popup windows
#
# This class is based on code found in some itcl demo classes
# and uses the standard trick of creating a window with the same
# name as the object so that the object behaves like a Tk widget.
#
# Author: Allan Brighton (allan@piano.sta.sub.org) 

itcl_class TopLevelWidget {

    # Create a toplevel widget with the same name as this object.
    # Note that it is not now practical to name you itcl widget ".",
    # but this should change in Tk-4.0

    constructor {config} {
	set class [$this info class]

	::rename $this $this-tmp-
	toplevel $this -class $class
	::rename $this $this-win
	::rename $this-tmp- $this
	set binding "rename $this-win {}"

	# make popup windows open and close with the parent windows
	# (assuming main window is a child of "." and "." is unmapped)
	if {"[winfo parent $this]" != "." && $transient} {
	    wm transient $this [winfo parent $this]
	} 

	wm iconname $this $class
	if {$withdraw} {
	    wm withdraw $this
	} elseif {$center} {
	    wm withdraw $this
	    after 0 $this center_window
	}

	# catch destroy and remove the object's command, therefore
	# removing the object
	bind $this <Destroy> "\
		[bind [winfo class $this] <Destroy>] ;
		[bind $this <Destroy>] ;
		$binding"
    }


    # handle valid "-name value" assignments

    method config {config} {}


    #  destroy the widget and its window
    
    destructor {
	destroy $this
    }


    # Start an itcl application - that is, an application with an itcl
    # TopLevel widget as its main window. 
    # This proc assumes that the command line args (global: $argv) 
    # should be passed on to the class (which is derived from this class).
    # If any errors occur, we print a usage message based on the itcl
    # info about the class.

    proc start {} {
	global argv

	# hide the "." window since an itcl class will replace it
	wm withdraw .

	# get the derived class name from the calling scope
	set class [uplevel info class]
	set name .[string tolower $class]

	# create the object at global scope
	if {[catch {uplevel #0 "$class $name -standalone 1 $argv"} msg]} {
	    catch {start_err_ $class $name $msg}
	    exit 1
	}

	# exit when class exits - don't wait for "."
	if {[winfo exists $name]} {
	    tkwait window $name
	}
	exit 0
    }

    
    # This proc is called by "start" above, in case of errors.
    # It attempts to print an informative message indicating
    # which option was wrong and what options are available by
    # using the itcl "info" command.

    proc start_err_ {class name msg} {
	global argv0
	set prog [file rootname [file tail $argv0]]
	set sub [regsub -- {syntax error in config assignment ([^:]*:) unrecognized variable} \
		    $msg {invalid option: \1} newmsg]
	if {$sub} {
	    puts stderr "$prog: $newmsg"
	    puts stderr "Usage: $prog ?option value? ..."
	    puts stderr "Options: "
	    foreach i [lreverse [$class :: info public]] {
		# don't include the options from this base class
		lassign [split $i :] base {} opt
		if {"$base" != "[info class]"} {
		    puts stderr "\t-$opt"
		}
	    }
	} else {
	    puts stderr "$prog: $msg"
	}
    }


    # Use this method to quit the application if you might want to 
    # reuse the window later

    method quit {} {
	if {$standalone} {
	    $this delete
	} else {
	    wm withdraw $this
	}
    }


    # run the given tcl command in the scope of this class
    # while displaying the (blt) busy cursor

    method busy {cmd} {
	global errorInfo errorCode

	if {[incr busy_count] == 1} {
	    blt_busy hold $this
	    update idletasks
	}
	
	# save any errors and report them later
	if {[catch [list uplevel $cmd] msg]} {
	    set savedInfo $errorInfo
	    set savedCode $errorCode
	    set status 1
	} else {
	    set status 0
	}

	if {[incr busy_count -1] == 0} {
	    blt_busy release $this
	}

	if {$status} {
	    error $msg $savedInfo $savedCode
	}
    }


    # run the given tcl command and print out any errors

    method test {cmd} {
	if {[catch [list uplevel $cmd] msg]} {
	    puts stderr "test: $msg"
	    error $msg
	}
    }


    # add a menu button to the menubar
    # if side is specified, the menu button is placed on the given side

    method add_menubutton {label {side left}} {
	set f $this.menubar
	set name [string tolower $label]
	set m $f.$name.m
	lappend mblist [set mb $f.$name]
	pack [menubutton $mb -text $label -menu $m] \
	    -side $side -padx 1m -ipadx 1m
	menu $m
	return $m
    }


    # add a menubar to the main frame

    method add_menubar {} {
	pack [frame $this.menubar -relief raised -bd 2] \
	    -side top -fill x -ipady 1m
    }


    # setup menubar traversal

    method setup_menubar_traversal {} {
	eval "tk_menuBar $this.menubar $mblist"
    }


    # Center this window on the screen. 

    method center_window {} {
	wm withdraw $this
	update idletasks
	set parent [winfo parent $this]
	set x [expr [winfo screenwidth $this]/2 - [winfo reqwidth $this]/2 \
		- [winfo vrootx $parent]]
	set y [expr [winfo screenheight $this]/2 - [winfo reqheight $this]/2 \
		- [winfo vrooty $parent]]
	wm geom $this +$x+$y
	wm deiconify $this
    }


    # set debugging on/off

    method debug {onoff} {
	global debug verbose
	::set debug [::set verbose [expr {"$onoff" == "on"}]]
	cmdtrace $onoff
    }


    # -- public variables --

    # true if running as separate process
    public standalone {0}

    # if true, center the application window on startup
    public center {1}

    # if true, withdraw the application window on startup
    public withdraw {0}

    # if true and this is a child of another TopLevel widget,
    # make the toplevel window transient - so that it will
    # open and close with its parent
    public transient {1}

    # -- protected variables --

    # count used for busy cursor
    protected busy_count {0}

    # list of menu buttons in menu bar, if any
    protected mblist {}

    # -- common variables --

    # first toplevel window created replaces "." as the main window
    common main_window {}

}

