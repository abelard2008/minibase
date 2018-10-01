# PrintDialog.tcl - Base class of Popup dialog for specifying printer options
#
# Author: Allan Brighton (allan@piano.sta.sub.org) 


itcl_class PrintDialog {
    inherit TopLevelWidget

    # this method is called when the save button is pressed 
    # to save the print configuration for this user in a file 
    # for later loading

    method save {} {
	set printcmd [$this.printcmd get]
	set filename [$this.print_to_file get]
	set print_to_file [$this.print_to_file var]
	set fd [open $userfile w]
	puts $fd $magic
	puts $fd [list $printcmd $print_to_file $filename]
	close $fd
    }

    
    # load the user config file (created by save) if one exists,
    # otherwise set the default values. The file is searched for
    # in $HOME and the current dir.

    method load {} {

	# determine file name, see if one exists
	if {"$userfile" == ""} {
	    set userfile [utilGetConfigFilename $this]
	}
	if {![file exists $userfile]} {
	    return
	}

	# read the file
	set fd [open $userfile]

	if {"[gets $fd]" != "$magic"} {
	    return
	} 
	lassign [gets $fd] printcmd print_to_file filename
	close $fd
    }


    # This method is called when the OK button is pressed
    # Open the file or pipe and pass the fd to the subclass print
    # method.

    method ok {args} {
	busy {
	    set printcmd [$this.printcmd get]
	    set file [$this.print_to_file get]

	    if {"$file" == ""} {
		set fd [open "|$printcmd" w]
	    } else {
		set fd [open $file w]
	    }

	    virtual print $fd
	    close $fd
	}
	
	# make the window go away
	quit
    }

    
    # should be redefined in subclass

    method print {fd} {
	puts "nothing to print..."
    }



    # constructor: create a PrintDialog window

    constructor {config} {
	TopLevelWidget::constructor

	wm minsize $this 10 10
	wm title $this "Printer Options"

	# load user default file, if any
	virtual load

	# printer name entry
	pack [frame $this.top -bd 3 -relief groove] \
	    -side top -fill x -expand 1
	pack [Entry $this.printcmd -text "Print Command:" \
		  -action "$this ok"] \
	    -side left -expand 1 -fill x -in $this.top
	$this.printcmd config -value $printcmd

	# frame for misc other items added by sublasses
	pack [frame $this.config] \
	    -side top -fill both -expand 1

	# print to file entry
	pack [frame $this.ptf -bd 3 -relief groove] \
	    -side top -fill x -expand 1
	pack [CheckEntry $this.print_to_file -text "Print to File:" \
		 -action "$this ok"] \
	    -side left -expand 1 -fill x -in $this.ptf
	if {$print_to_file} {
	    $this.print_to_file config -value $filename
	}

	# button Row
	pack [ButtonFrame $this.btns \
		-ok_cmd "$this ok" \
		-cancel_cmd "$this quit"] \
		-side bottom -fill x -expand 1 -ipady 1m
	$this.btns append {Save} "$this save"
    }


    # -- public variables --

    # user defaults file (without path, $HOME will be added)
    public userfile {}

    # default print command
    public printcmd {lpr}

    # bool: print to file ?
    public print_to_file {0}

    # file name for print to file
    public filename {}


    # -- protected vars --

    # header string for userfile for comparison
    protected magic {# print settings v1.0}

}

