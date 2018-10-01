# canvas.tcl - utility routines for working with canvases
#
# -----------------------------------------------------------------------------
# Copyright 1993 Allan Brighton.
# 
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted,
# provided that the above copyright notice appear in all copies.  Allan
# Brighton make no representations about the suitability of this software
# for any purpose.  It is provided "as is" without express or implied
# warranty.
# -----------------------------------------------------------------------------


# create a canvas window with the given frame and name

proc canvas_create {frame canvas} {
    canvas $frame.canvas -scrollregion "0 0 0 0" 
    set vscroll [scrollbar $frame.vscroll \
	    -relief sunken \
	    -command "$canvas yview"]
    set hscroll [scrollbar $frame.hscroll \
	    -orient horiz \
	    -relief sunken \
	    -command "$canvas xview"]
    $canvas config -xscrollcommand "$hscroll set" -yscrollcommand "$vscroll set"
    
    pack append $frame $hscroll {bottom fillx} $vscroll {right filly} \
	    $canvas {expand fill}
    
    return $canvas
}

