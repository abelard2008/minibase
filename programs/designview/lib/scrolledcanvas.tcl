
#
# ScrolledCanvas
#
# Creates a frame containing a canvas with vertical and horizontal
# scrollbars. The name of the frame is passed in as the 'c' parameter.
#
# This is a slightly modified version of an example from Brent Welch's
# "Practical Programming in Tcl and Tk".
#

proc ScrolledCanvas { c width height region } {

    set bcolor lightblue

    frame $c -bd 0 -bg $bcolor
    #canvas $c.canvas -width $width -height $height
    canvas $c.canvas -bg $bcolor \
	    -highlightthickness 0 \
	    -xscrollcommand [list $c.xscrollframe.xscroll set] \
	    -yscrollcommand [list $c.yscrollframe.yscroll set] \
	    -width 50 -height 50

    frame $c.yscrollframe
    scrollbar $c.yscrollframe.yscroll -orient vertical -width 10 \
	    -command [list $c.canvas yview] -highlightthickness 0
    pack $c.yscrollframe.yscroll -side top -anchor nw -expand true -fill y

    frame $c.xscrollframe
    frame $c.xscrollframe.cushion -width 14 -height 14
    scrollbar $c.xscrollframe.xscroll -orient horizontal -width 10 \
	    -command [list $c.canvas xview] -highlightthickness 0
    pack $c.xscrollframe.xscroll -side left -anchor sw -expand true -fill x
    pack $c.xscrollframe.cushion -side right -anchor nw

    pack $c.xscrollframe -side bottom -anchor sw -fill x
    pack $c.yscrollframe -side right -anchor ne -expand true -fill y
    pack $c.canvas -side top -fill both -expand true

    return $c.canvas

}
