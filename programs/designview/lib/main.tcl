
#
# Location of designview source files
#
if [info exists env(DBNPATH)] {
    set __DBNDIR__ $env(DBNPATH)
} else {
    set __DBNDIR__ [pwd]
    set env(DBNPATH) $__DBNDIR__
}
source $__DBNDIR__/lib/globals.tcl

#
# Set display options
#
#setOptions

#
# Create the main window menus
#
menu_setup

#
# Initialize modules
#
rframe_init
detail_init

#
# Now the main window widgets will be created...
#
frame .top
set __MAIN__buttonFrame [frame .top.mainbuttons -relief raised -bd 2]
set __MAIN__detailButton [button $__MAIN__buttonFrame.detail \
	-text "View Details" \
	-command { main_viewDetails }]
set __MAIN__sessionButton [button $__MAIN__buttonFrame.session \
	-text "Start Session" -command { main_newSession }]
set __MAIN__sqlButton [button $__MAIN__buttonFrame.sql \
	-text "Show SQL" -command { sql_display }]

set __MAIN__buttons [list $__MAIN__detailButton $__MAIN__sessionButton \
	$__MAIN__sqlButton]
foreach b $__MAIN__buttons {
    $b config -state disabled
    pack $b -side top -padx 5 -pady 2 -fill x
}
pack $__MAIN__buttonFrame -side left -fill y -pady 0

set __MAIN__canvasFrame .top.cframe
set __MAIN__canvas \
	[ScrolledCanvas $__MAIN__canvasFrame 200 200 { 0 0 200 200 }]
pack $__MAIN__canvasFrame -side top -anchor nw -expand true -fill both

pack .top -side top -anchor nw -fill both -expand true

#
# Configure the main window
#
wm title . designview
wm geometry . 500x225+25+50
main_setDetailVisible 0
