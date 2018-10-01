###################################################################
#
# Class: Frame
#
# Description: The changes on each frame as the log-file is 
#              scanned are defined here...
#             
# Authors : Huseyin Bektas (bektas@cs.wisc.edu) &&
#           Harry Stavropoulos (harryst@cs.wisc.edu)
#
###################################################################

set miniroot [utilGetEnv "MINIBASE_ROOT"]
source "$miniroot/programs/vistools/bmtool/source_files/config_file"

itcl_class Frame {

    constructor { config } {
	set class [$this info class]

    set oldthis [info namespace tail $this]
    puts stderr "My name is $oldthis"
    puts stderr "This is $this"
    flush stderr
 
	::rename $oldthis $oldthis-tmp-
	::frame $oldthis -class $class -relief flat
	::rename $oldthis $oldthis-win-
	::rename $oldthis-tmp- $oldthis
 #  ::rename $this $this-tmp-
 #  ::frame $this -class $class -relief flat
 #  ::rename $this $this-win-
 #  ::rename $this-tmp- $this

    global ::frame_height
    global ::frame_width
    set height $frame_height
    set width  $frame_width

    global ::bg_light_green
    global ::untouched_frame_color 
    global ::dirty_page_color
    global ::clean_page_color
    global ::pin_color 
    global ::bg_color

    set nice_green     $bg_light_green
    set virgin_frame   $untouched_frame_color
    set dirty_page     $dirty_page_color
    set clean_page     $clean_page_color
    set color_of_pin   $pin_color
    set color_of_file  $bg_color
    set frameColor     $untouched_frame_color

	canvas $this.canvas -relief {raised} -borderwidth {2} \
		-height $height -width $width \
		-background $frameColor

	bind $this.canvas <Motion> "$this UpdateFrameInfo"

	pack $this.canvas
    }



    #
    # Color the file blob - if you want to change the size of the blob,
    # do it here...
    #

    method FileColor {color} {

        $this.canvas create oval [expr $width - 14] 3 [expr $width - 3] 14 \
                                       -fill $color -outline $color -width 3
    }



    #
    # Color the pin bitmap  - if you want to change the position of the
    # bitmap, do it here...
    #

    method PinColor { bg_color  fg_color } {

	global pin_bitmap

	$this.canvas create bitmap 10 10  -bitmap $pin_bitmap \
		      -background $bg_color   -foreground $fg_color
    }



    method UpdateFrameInfo {} {
	::UpdateFrameInfo $fileName $pageNumber \
		$frameNumber $numPins $numUnpins $color_of_file
    }



    method ResetFrame {} {

	$this.canvas configure -bg $virgin_frame

        #
        # unmark the frame
	#

        FileColor $virgin_frame
	PinColor $virgin_frame $virgin_frame

	set fileName ""
	set frameColor $virgin_frame
	set pageNumber 0
	set numPins 0
	set numUnpins 0
	set isDirty 0
        set isPinned 0
	set hit_ratio 0
	set in_memory 0
    }



    method PinPage { page fname fcolor } {

        if { $fname != $fileName } {
	    AssignPage $fname $page $fcolor
#           tkerror "This frame contains a page of file $fileName - not $fname"
#           return
        }

        if { $page != $pageNumber } {
	    set pageNumber $page
#           tkerror "This frame contains page $pageNumber - not $page"
#           return
        }

        #
        # if page is dirty, give a warning - ie this should not happen but
        # with current single-user implementation it happens
        #

        if { $isDirty } {
           tkerror "pinning a dirty page - warning"
	   if {$numPins == 0} {
               PinColor $dirty_page $color_of_pin
           }

        }  else {

           PinColor $clean_page $color_of_pin

        }

	incr numPins
	set isPinned 1
 
	#
	# return a value so that the number of pinned pages can be tracked.
	#

	if { [expr $numPins - $numUnpins] == 1 } {
	   return 1
        } else {
	   return 0
        }
    }



    method DirtyPage { page fname } {


        if { $fname != $fileName } {
           tkerror "This frame contains a page of file $fileName - not $fname"
           return
        }

        if { $page != $pageNumber } {
           tkerror "This frame contains page $pageNumber - not $page"
           return
        }

        #
        # if page is dirty, give a warning - ie this should not happen but
        # with current single-user implementation it happens
        #

        if { $isDirty } {

           tkerror "dirtying an already dirty page - warning"
           return

        } elseif { $isPinned } {

	   $this.canvas configure -bg $dirty_page
	   PinColor $dirty_page $color_of_pin
	   set isDirty 1

        } else {

	   set isDirty 1
	   this.canvas configure -bg $dirty_page
        }
    }



    method UnPinPage { page fname } {

        if { $fname != $fileName } {
           tkerror "This frame contains a page of file $fileName - not $fname"
           return
        }

        if { $page != $pageNumber } {
           tkerror "This frame contains page $pageNumber - not $page"
           return
        }

	#
	# if no pins left, erase the pin - page is dirty now
	#

        incr numUnpins
 
        if {$numUnpins == $numPins} {

           set isPinned 0 
	   if { $isDirty } {
               PinColor $dirty_page $dirty_page
	       return -2
           } else {
	       PinColor $clean_page $clean_page
	       return -1
           }

        } elseif {$numUnpins > $numPins} {

               tkerror "Unpinning a page more times than it has been pinned"
               return

        } else {

	   return 0

        } 
    }



    method FreePage { page fname } {

        if { $fname != $fileName } {
           tkerror "This frame contains a page of file $fileName - not $fname"
           return
        }

        if { $page != $pageNumber } {
           tkerror "This frame contains page $pageNumber - not $page"
           return
        }


	set answer [expr $numUnpins - $numPins]
        set numUnpins $numPins
	set isPinned 0 
	PinColor $clean_page $clean_page

	return $answer
    }



    method WritePage { page fname } {
	
#	 if { $fname != $fileName } {
#	    tkerror "This frame contains a page of file $fileName - not $fname"
#	    return
#	 }

	if { $page != $pageNumber } {
	   tkerror "This frame contains page $pageNumber - not $page"
	   return
        }

        if { $isDirty == 0 } {
           tkerror "writing a clean page"
	   return
        }

	#
	# write it - page is no longer dirty
	#
	
	set isDirty 0
	$this.canvas configure -bg $clean_page
	PinColor $clean_page $clean_page

	#
	# if it is NOT pinned, this means that we had a dirty page when 
	# we unpinned it. Return a value to decrease the number of 
	# unpinned dirty pages, since the page is clean, now.
	#

	if { $isPinned == 1 } {
	   return 0 
        } else {
	   return -1
        }
    }


    
    method AssignPage {fname pnumber filecolor} {

    #
    # if page is dirty, give a warning 
    #

	if { $isDirty } {
	   tkerror "Frame holds a dirty page - cannot read"
	   return
	}

	set fileName $fname
	set pageNumber $pnumber
	set color_of_file $filecolor
	
	$this.canvas configure -bg $clean_page
        FileColor $filecolor
	PinColor $clean_page $clean_page
    }


    protected height 0
    protected width  0 

    common totalInFrames 0

    protected fileName    ""
    protected pageNumber  0
    protected numPins     0
    protected numUnpins   0
    
    public frameNumber    0
    protected isDirty     0
    protected isPinned    0
    
    protected in_memory   0

    #
    # colors used
    #

    protected nice_green     0
    protected virgin_frame   0
    protected dirty_page     0
    protected clean_page     0
    protected color_of_pin   0
    protected color_of_file  0

    public frameColor 0

    common hit_ratio 0
}
