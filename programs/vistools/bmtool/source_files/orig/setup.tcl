###################################################################
#
# Below are all the procedures that create the main window containing
# the buffer manager, the action buttons and the replacement strategy.
#
# Author: Harry Stavropoulos (harryst@cs.wisc.edu)
#
###################################################################

lappend auto_path $bm_code_dir/../library
lappend auto_path $bm_code_dir/source_files
set_app_defaults

option add *font 9x15bold

source $bm_code_dir/../library/compat.tcl
source $bm_code_dir/source_files/config_file
source $bm_code_dir/source_files/filecomplete.tcl
source $bm_code_dir/source_files/locktime.tcl
source $bm_code_dir/source_files/action_listing.tcl
source $bm_code_dir/source_files/frame.tcl
source $bm_code_dir/source_files/trace.tcl


set incrNext 1
set incrPrev 1


proc CreateTable { numRow numCol maxFrameNumber} {

    global bg_unusable_frame
    global fg_unusable_frame
    global unusable_frame_bitmap
    global incrNext 
    global incrPrev


    set frame 0

    for { set x 0 } { $x < $numRow } { incr x } {

	frame .table.row$x

	for { set y 0 } { $y < $numCol } { incr y } {

	    #
	    # if frame is "unusable" (cos less than the depicted
	    # frames are used according to the log-file) then
	    # show "invalidated" frames...
	    #

	    if { $frame > $maxFrameNumber } {

		#
		# Comment out ONE of the following:
		#
		# indicate "unusable" frames by interdiction sign - colors
		# are set here... - not in the config file...
		#

		# Frame .table.$frame -frameNumber $frame -frameColor white
		# .table.$frame.canvas create bitmap 30 30 -bitmap @$bitmap_dir/no.bmp \
	        #                      -background red2  -foreground white

		#
		# ...or, indicate "unusable" frames by red X
		#


		Frame .table.$frame -frameNumber $frame -frameColor $bg_unusable_frame
		.table.$frame.canvas create bitmap 30 30 -bitmap $unusable_frame_bitmap \
				      -background $bg_unusable_frame \
				      -foreground $fg_unusable_frame

	    } else {

	        Frame .table.$frame -frameNumber $frame

            }

	    pack .table.$frame -side left -in .table.row$x
	    incr frame
	}
	pack .table.row$x
    }
    pack .table
}



proc Create_LRU_ReplacementFrame { path } { 

    #
    # this variable is NOT used here - if you want to create a similar
    # listing for another replacement strategy, you HAVE to include it
    # and use it as the title of the listbox - see below "Listbox $path..."
    # 
    # global replacement_strategy                       ******
    #

    #
    # color configurations
    #

    global fg_LRU_replacement_list
    global bg_LRU_replacement_list
    global font_LRU_replacement_list

    #
    # May want to change the title - see above
    #

    Listbox $path.lru_list -vscroll 0 -hscroll 1 -title "LRU"
    $path.lru_list.hscroll configure -width 10
    $path.lru_list.listbox configure -relief sunken \
				 -background $bg_LRU_replacement_list \
                                 -foreground $fg_LRU_replacement_list \
                                 -font $font_LRU_replacement_list \
                                 -geometry 1x1

    pack $path.lru_list  -fill x -expand true
}



proc Create_ReplacementFrame { path } {

    global replacement_strategy

    #
    # color configurations
    #

    global fg_replacement_list
    global bg_replacement_list
    global font_replacement_list

    Listbox $path.list -vscroll 0 -hscroll 1 -title "$replacement_strategy"
    $path.list.hscroll configure -width 10
    $path.list.listbox configure -relief sunken \
                                 -background $bg_replacement_list \
                                 -foreground $fg_replacement_list \
                                 -font $font_replacement_list \
                                 -geometry 1x1

    pack $path.list  -fill x -expand true
}



proc Create_CurrentReplacementStrategyFrame { replacement_strategy } {

    global fg_current_replacement_strategy_label
    global bg_current_replacement_strategy_label
    global fg_current_replacement_strategy
    global bg_current_replacement_strategy

    frame .current_replacement
    frame .current_replacement.replacement_str  \
			       -bg $bg_current_replacement_strategy_label 
    label .current_replacement.replacement_prompt \
                               -text "The current replacement strategy is: " \
                               -fg $fg_current_replacement_strategy_label \
                               -bg $bg_current_replacement_strategy_label
    label .current_replacement.replacement_str.label \
                               -textvariable replacement_strategy \
                               -fg $fg_current_replacement_strategy \
                               -bg $bg_current_replacement_strategy
     
    pack .current_replacement.replacement_str.label -fill x -expand 1
    pack .current_replacement.replacement_prompt .current_replacement.replacement_str \
						 -side left -fill x -expand 1
}



proc CreateInfoFrame { path } {

    global fg_info_label
    global info_label_width
    global bg_color

    set left [ frame $path.left ]
    set right [ frame $path.right ]

    label $left.fname         -text "File Name   :"
    label $left.pageNumber    -text "Page Number :"
    label $left.frameNumber   -text "Frame Number:"
    label $left.pagePins      -text "Page Pins   :"
    label $left.pageUnpins    -text "Page Unpins :"

    pack $left.fname $left.pageNumber $left.frameNumber \
         $left.pagePins $left.pageUnpins -anchor w

    frame $right.fname 
    canvas $right.fname.canvas -height 11 -width 16  -bg $bg_color
    label $right.fname.name -width $info_label_width  -foreground $fg_info_label
    label $right.pageNumber                           -foreground $fg_info_label 
    label $right.frameNumber                          -foreground $fg_info_label
    label $right.pagePins                             -foreground $fg_info_label
    label $right.pageUnpins                           -foreground $fg_info_label

    pack $right.fname.name $right.fname.canvas -side left -anchor w
    pack $right.fname $right.pageNumber $right.frameNumber \
         $right.pagePins $right.pageUnpins -anchor w


    pack $left $right -side left
}



proc CreateToolFrame { path } {

    global bg_button_act
    global fg_button_act
    global bg_button_steps
    global fg_button_steps
    global act_button_width
    global act_button_entry_width

    set left [ frame $path.left ]
    set right [ frame $path.right ]

    #
    # This is used to left align the words in the action-buttons
    #

    set space " "


    button $left.start -activebackground $bg_button_act \
		       -width $act_button_width \
		       -text "Start" \
		       -command {
                                  $traceFile Start
    }

    button $left.next  -activebackground $bg_button_act \
		       -width $act_button_width \
		       -text "Next$space " \
		       -command {
	                          $traceFile Next $incrNext 0
    }

    button $left.prev  -activebackground $bg_button_act\
		       -width $act_button_width \
		       -text "Prev$space " \
		       -command {
	                          $traceFile Prev $incrPrev
    }

    button $left.break -activebackground $bg_button_act \
		       -width $act_button_width \
		       -text "Cont$space " \
		       -command {
				  $traceFile Break
    }

    label $left.exit -text " "
#   button $left.exit  -activebackground $bg_button_act \
#	       -width $act_button_width \
#	       -text "Exit$space " \
#	       -command {
#                          exit
#   }

    pack $left.start -ipadx 5
    pack $left.next  -ipadx 5
    pack $left.prev  -ipadx 5
    pack $left.break -ipadx 5
    pack $left.exit  -ipadx 5
    

    entry $right.incrNext -relief sunken -textvariable incrNext \
	  -width $act_button_entry_width  -foreground $fg_button_steps \
					  -background $bg_button_steps 

    bind $right.incrNext <Key-Return> { $traceFile Next $incrNext 0 }
    entry $right.incrPrev -relief sunken -textvariable incrPrev \
	  -width $act_button_entry_width -foreground $fg_button_steps \
					  -background $bg_button_steps

    bind $right.incrPrev <Key-Return> { $traceFile Prev $incrPrev }
    message $right.break 


    pack $right.incrNext $right.incrPrev $right.break 
    pack $right $left -side right
}



proc UpdateFrameInfo { fileName pageNumber frameNumber numPins numUnpins color } {

    set right ".auxiliary.info.right"

    $right.fname.name    config -text  [ format "%-13s" $fileName ]  
    $right.pageNumber    config -text  [ format "%3d" $pageNumber ] 
    $right.frameNumber   config -text  [ format "%3d" $frameNumber ]
    $right.pagePins      config -text  [ format "%3d" $numPins ]    
    $right.pageUnpins    config -text  [ format "%3d" $numUnpins ] 
    $right.fname.canvas  create oval 0 0 11 11 -fill $color -outline $color -width 0
}


proc ClearAll {buffer_size} {

    for { set frm 0 } { $frm < $buffer_size } { incr frm } {
	.table.$frm ResetFrame 
    }
}


proc FrameCreate { trace_filename replacement_strategy } {

    global show_replacement_strategy_list
    global show_lru_list
    global show_no_list


    frame .replacement

    frame .auxiliary
    frame .auxiliary.info
    CreateInfoFrame .auxiliary.info
    frame .auxiliary.tool
    CreateToolFrame .auxiliary.tool

    pack .auxiliary -side bottom -expand 1 -fill x
    pack .auxiliary.info  -side left -fill x
    pack .auxiliary.tool  -side right -fill x


    if { $show_no_list } {

       Create_CurrentReplacementStrategyFrame $replacement_strategy

       pack .current_replacement -fill both -expand 1
       pack .table -fill both -expand 1
       return

    } elseif { $replacement_strategy == "LRU" } {

       Create_LRU_ReplacementFrame .replacement
       Create_CurrentReplacementStrategyFrame $replacement_strategy

       pack .replacement -side bottom  -fill x -expand 1
       pack .current_replacement -fill both -expand 1
       pack .table -fill both -expand 1
       return

    } elseif { $show_lru_list } {
    
       Create_LRU_ReplacementFrame .replacement 
       Create_CurrentReplacementStrategyFrame $replacement_strategy

       if { $show_replacement_strategy_list } {
	  Create_ReplacementFrame .replacement 
       }

       pack .replacement -side bottom  -fill x -expand 1
       pack .current_replacement -fill both -expand 1
       pack .table -fill both -expand 1
       return
    } else {
    }
}
