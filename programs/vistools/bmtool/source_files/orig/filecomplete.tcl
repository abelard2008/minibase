###################################################################
#
# Below is the definition of the window where the log-file to be
# read is selected. This is really an adaptation of Example 17-4
# in the first edition of Brent Welch's book "Practical Programming
# in Tcl and Tk" - copyright Prentice Hall. I took out all the stuff
# that looked unnecessary.
#
# Author: Harry Stavropoulos (harryst@cs.wisc.edu)
#
###################################################################

##source config_file

# Dialog chapter

# fileselect returns the selected pathname, or {}

proc fileselect { dir_w_bm_logs {why "Welcome to the Buffer Manager visualization"} {default {}} } {
	global fileselect

	catch {destroy .fileselect}
	set t [frame .fileselect ]
	pack $t 

	global welcome_color
    
	message $t.msg -aspect 1000 -text $why -fg $welcome_color
	pack $t.msg -side top -fill x
    
	global message_prompt_color

	label $t.dir -text "PLEASE SELECT A LOG FILE" \
			    -width 15 -relief flat \
			    -fg $message_prompt_color
	pack $t.dir -side top -fill x

	# Create an entry for the pathname
	# The value is kept in fileselect(path)
	frame $t.top

	global file_prompt_color

	label $t.top.l -text "File:" -padx 0 -fg $file_prompt_color
	set e [label  $t.top.path -relief sunken \
		   -textvariable fileselect(path) -anchor w]

	pack $t.top -side top -fill x
	pack $t.top.l -side left
	pack $t.top.path -side right -fill x -expand true
	set fileselect(pathEnt) $e
    
	focus $e
    
	# Create a listbox to hold the directory contents
	listbox $t.list -yscrollcommand [list $t.scroll set]
	scrollbar $t.scroll -command [list $t.list yview]

	# A single click copies the name into the entry
	# A double-click selects the name
 	bind $t.list <Button-1> {fileselectClick %y}
	bind $t.list <Double-Button-1> {
		fileselectClick %y ; fileselectOK
	}
    
	# Warp focus to listbox so the user can use arrow keys
	bind $e <Tab> "focus $t.list ; $t.list select adjust 0"
	bind $t.list <Return> fileselectTake
	bind $t.list <space> {fileselectTake ; break}
	bind $t.list <Tab> "focus $e"
    
	# Create the OK and Cancel buttons
	# The OK button has a rim to indicate it is the default
	frame $t.buttons -bd 10
	frame $t.buttons.ok -bd 2 -relief sunken
	button $t.buttons.ok.b -text OK \
		-command fileselectOK
	button $t.buttons.cancel -text Cancel \
		-command fileselectCancel

	# Pack the list, scrollbar, and button box
	# in a horizontal stack below the upper widgets
	pack $t.list -side left -fill both -expand true
	pack $t.scroll -side left -fill y
	pack $t.buttons -side left -fill both
	pack $t.buttons.ok $t.buttons.cancel \
		-side top -padx 10 -pady 5
	pack $t.buttons.ok.b -padx 4 -pady 4
    
	# Initialize variables and list the directory
	if {[string length $default] == 0} {
		set fileselect(path) {}
		set dir $dir_w_bm_logs
	} else {
		set fileselect(path) [file tail $default]
		set dir [file dirname $default]
	}
	set fileselect(dir) {}
	set fileselect(done) 0

	# Wait for the listbox to be visible so
	# we can provide feedback during the listing 
	tkwait visibility .fileselect.list
	fileselectList $dir

	tkwait variable fileselect(done)
	destroy .fileselect
	return $fileselect(path)
}


proc fileselectList { dir {files {}} } {
	global fileselect

	set fileselect(dir) $dir
	.fileselect.list delete 0 end
	.fileselect.list insert 0 Listing...
	update idletasks
	.fileselect.list delete 0
	if {[string length $files] == 0} {
		# List the directory and add an
		# entry for the parent directory
		set files [glob -nocomplain $fileselect(dir)/*]
	}
	set others {}
	foreach f [lsort $files] {
		if [file isdirectory $f] {
			continue
		} else {
			lappend others [file tail $f]
		}
	}
	foreach f $others {
	    .fileselect.list insert end $f
	}
}


proc fileselectOK {} {
	global fileselect
    
	set path $fileselect(dir)/$fileselect(path)
    
	if [file exists $path] {
		set fileselect(path) $path
		set fileselect(done) 1
		return
	}
}


proc fileselectCancel {} {
	global fileselect
	set fileselect(done) 1
	set fileselect(path) {}
}


proc fileselectClick { y } {
	# Take the item the user clicked on
	global fileselect
	set l .fileselect.list
	$l select clear
	set a [$l nearest $y]]
	set fileselect(path) [$l get [$l nearest $y]]
	focus $fileselect(pathEnt)
}


proc fileselectTake {} {
	# Take the currently selected list item and
	# change focus back to the entry
	global fileselect
	set l .fileselect.list
	set fileselect(path) [$l get [$l curselection]]
	focus $fileselect(pathEnt)
}
