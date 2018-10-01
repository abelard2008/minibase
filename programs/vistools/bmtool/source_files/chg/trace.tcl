###################################################################
#
# Class: Trace
#
# Description: The actions taken as the log-file is scanned are 
#              defined in this class. Also the actions taken 
#              when the user presses the control buttons.
# 
# Authors : Huseyin Bektas (bektas@cs.wisc.edu) &&
#           Harry Stavropoulos (harryst@cs.wisc.edu)
#
###################################################################

##source config_file


itcl_class Trace {

    constructor { config } {

    global show_lru_list
    global show_replacement_strategy_list
    global show_no_list

    set show_LRU                     $show_lru_list
    set show_replacement_strategy    $show_replacement_strategy_list
    set do_not_show_lists            $show_no_list

    global list_of_file_colors
    set colorlist    $list_of_file_colors

    global maximum_num_openedfiles
    set max_no_openedfiles $maximum_num_openedfiles

    global num_of_listed_actions
    set listed_actions $num_of_listed_actions

	if { ![file readable $this] } {
	    tkerror "$this: can not read the file"
	} else {
	    set fd [ open $this r ]
        }

	CreateMenu
        Startup 
    }



    method Startup {} {

	 global action_list
	 global replacement_strategy

	 if { [gets $fd line] == "-1" } {

             tkerror "the first line is EOF" 
	     return

	 } else {

            set eof 0
	    set action [ lindex $line 0 ]
            if { $action != "STARTUP" } {

                tkerror "you have to do STARTUP first"
                return

            } else {

		incr current_line_count

	        set replacement_strategy  [ lindex $line 3 ]

		#
		# This forces us to see only the LRU list 
		# if we have the LRU strategy in our log file
		# The only way to override this is by setting
		# show_no_list to 0 in the config file
		#

		if {$replacement_strategy == "LRU" }  {
		   set show_LRU 1
		   set show_replacement_strategy 0
                } else {
		}

		# This actually means "number of buffers (or frames)
		# in the pool" rather than "size in bytes of a buffer."
	        set buffer_size           [ lindex $line 2 ]

		$action_list insert end $line

	        set maxFrameNumber     [expr $buffer_size -1]
		set numrows            [Set_Rows $buffer_size]
		set numcolumns         [Set_Columns $buffer_size]
		set stat_occup_frames  0

		CreateTable $numrows $numcolumns $maxFrameNumber
		FrameCreate $this $replacement_strategy

            }
         }
    }



    method Set_Rows {buffer_size} {

	set numrows -1

	if {$buffer_size > 64} {
	    tkerror "cannot support buffer sizes greater than 64"
	    return

        }  elseif {$buffer_size > 56} {
	       set numrows 8
        }  elseif {$buffer_size > 42} {  
	       set numrows 7
        }  elseif {$buffer_size > 30} {
	       set numrows 6
        }  elseif {$buffer_size > 20} {
	       set numrows 5
        }  elseif {$buffer_size >= 16} {
	       set numrows 4
        } else {
	   tkerror "cannot support buffer sizes less than 16"
           return
        }

	return $numrows
    }



    method Set_Columns {buffer_size} {

        set numcolumns -1


        if {$buffer_size > 64} {
            tkerror "cannot support buffer sizes greater than 64"
            return

        }  elseif {$buffer_size > 49} {
               set numcolumns 8
        }  elseif {$buffer_size > 36} {  
               set numcolumns 7
        }  elseif {$buffer_size > 25} {
               set numcolumns 6
        }  elseif {$buffer_size > 16} {
               set numcolumns 5
        }  elseif {$buffer_size == 16} {
               set numcolumns 4
        } else {
           tkerror "cannot support buffer sizes less than 16"
           return
        }           

        return $numcolumns
    }



    method Next { numLines from_Break } {
      
	global action_list
        global bg_color
        global bg_button_act
        global act_button_width
        global cont_button_noact_fg

        set line ""
        set space " "

	for { set i 0 } { $i < $numLines } { incr i } {

	    if { [gets $fd line] == "-1" } {
		$action_list insert end "** EOF **"
		$action_list yview [expr [$action_list size] - $listed_actions]
		set eof 1
		.auxiliary.tool.left.break configure \
                             -activebackground $bg_button_act \
                             -activeforeground $cont_button_noact_fg \
                             -bg $bg_color \
                             -fg $cont_button_noact_fg \
                             -width $act_button_width \
                             -text "Cont$space " \
                             -command {
                                       $traceFile Break
                             }
                return
	    }

	    set breakpoint [ lindex $line 0]

	    if { $breakpoint == "*" } {

            #
            # Do not count it as a line if it has a breakpoint
            #

	       if { $from_Break == 1 } {
		   return
               }
	       incr i -1
	       continue
            }

	    incr stat_num_actions
	    incr current_line_count 1

	    set action   [ lindex $line 0 ]
	    set frame    [ lindex $line 1 ]
	    set page     [ lindex $line 2 ]
	    set fname    [ lindex $line 3 ]
	    if {$fname == ""} {set fname "<unknown>"}

            $action_list insert end $line
	    $action_list yview [expr [$action_list size] - $listed_actions]

	    if {[lsearch $openfile_list $fname] == -1} {
		lappend openfile_list $fname
		incr no_openedfiles
		Assign_filecolor $fname
	    }

		
	    switch $action {
		
		PAGE_READ {
                    if { ($frame > $maxFrameNumber) || ($frame < -1) } {
                       tkerror "Frame $frame does not exist"
                       return
                    }

		    incr stat_num_reads

		    if { $stat_occup_frames <= $maxFrameNumber } {
		       incr stat_occup_frames
		    }

		    set filecolor $colorArray($fname)
		    .table.$frame AssignPage $fname $page $filecolor
		}
		
		PAGE_WRITE {
                    if { ($frame > $maxFrameNumber) || ($frame < -1) } {
                       tkerror "Frame $frame does not exist"
                       return
                    }

		    set ret_WrPage [.table.$frame WritePage $page $fname]
		    if { $ret_WrPage == -1 } {

		       #
		       # if a dirty UNPINNED page was written
		       # decrement the number of unpinned dirty pages
		       #

		       incr stat_unpin_dirty -1
                    }
		}
		
		PAGE_PIN {
                    if { ($frame > $maxFrameNumber) || ($frame < -1) } {
                       tkerror "Frame $frame does not exist"
                       return
                    }

		    incr stat_num_pins

		    set ret_Pin [.table.$frame PinPage $page $fname $colorArray($fname)]
		    if { $ret_Pin == 1 } {
		        incr stat_pin_pages $ret_Pin
			Remv_from_Candidate_List $frame
                    }
		}
		
		PAGE_UNPIN {
                    if { ($frame > $maxFrameNumber) || ($frame < -1) } {
                       tkerror "Frame $frame does not exist"
                       return
                    }

		    set ret_UnPin [.table.$frame UnPinPage $page $fname]
		    if { $ret_UnPin == -1 } { 

			#
		        # page unpinned and clean
                        #

			incr stat_pin_pages -1
			Addin_Candidate_List $frame

                    } elseif { $ret_UnPin == -2 } {

			#
			# page unpinned and dirty
			#

                        incr stat_pin_pages -1
			incr stat_unpin_dirty 1
			Addin_Candidate_List $frame
 
		    } else {

			#
			# page still pinned - or error - do nothing
			#
		    }
		}

		PAGE_FREE {
                    if { ($frame > $maxFrameNumber) || ($frame < -1) } {
                       tkerror "Frame $frame does not exist"
                       return
                    }

		    set ret_Free [.table.$frame FreePage $page $fname]
		    incr stat_pin_pages $ret_Free
		    Addin_Candidate_List $frame
		}

		PAGE_DIRTY {

                    if { ($frame > $maxFrameNumber) || ($frame < -1) } {
                       tkerror "Frame $frame does not exist"
                       return
                    }

		    .table.$frame DirtyPage $page $fname
		}

		default {
		}
	    }		

            opertime AddPoints $stat_pin_pages $stat_unpin_dirty \
			       $stat_num_reads $stat_num_pins \
			       $stat_num_actions $stat_occup_frames
	}
    	return $i
    }



    method Prev {numlines} {

	seek $fd 0 start
	:: ClearAll $buffer_size
	
	gets $fd line
       
        set current_line_count_old $current_line_count

	Start
	Next [expr $current_line_count_old - $numlines] 0
    }



    method Start {} {

	global action_list
	global actionlisting

	seek $fd 0 start
	:: ClearAll $buffer_size

	gets $fd line
	set current_line_count 0
	set openfile_list ""
	set no_openedfiles 0
	set clr_cnt 0
	set frame_list "" 
	set LRU_frame_list ""
        set eof 0

        set stat_pin_pages     0
        set stat_unpin_dirty   0
        set stat_num_actions   0
	set stat_num_pins      0
        set stat_num_reads     0
	set stat_occup_frames  0

	actionlisting Clear
	$action_list insert end $line

	opertime Clear_Graph

	if { $do_not_show_lists } {
	   return
        }

	if { $replacement_strategy == "LRU" } {
	   .replacement.lru_list clear
	   return
        } elseif { $show_LRU } {
           .replacement.lru_list clear
        } else {
        }

	if { $show_replacement_strategy } {
           .replacement.list clear
        }
    }



    #
    # To set a breakpoint, insert a line starting with a "*"
    # at the appropriate place in the log file. The contents
    # of the line will be ignored
    #

    method Break {} {

	global action_list
        global traceFile
        global bg_color
        global bg_button_act
        global act_button_width
        global cont_button_act_text
	global cont_button_act_fg
	global cont_button_act_bg
	global cont_button_noact_fg

        set space " "

        .auxiliary.tool.left.break configure \
                                   -activebackground $cont_button_act_bg \
                                   -activeforeground $cont_button_act_fg \
                                   -bg $cont_button_act_bg \
				   -fg $cont_button_act_fg \
                                   -text "WAIT"  -command {}

        update idletasks

	Next 1 1

        while { ([lindex $line 0] != "*") && ($eof == 0) } {
	   Next 1 1
        }  
        
        if {$eof == 1} {
	   return
        }

        $action_list insert end "Breakpoint encountered" 
	$action_list yview [expr [$action_list size] - $listed_actions]

        .auxiliary.tool.left.break configure \
                                   -activebackground $bg_button_act \
                                   -activeforeground $cont_button_noact_fg \
                                   -bg $bg_color \
                                   -fg $cont_button_noact_fg \
                                   -width $act_button_width \
                                   -text "Cont$space " \
                                   -command {
                                              $traceFile Break
                                   }

        return
    }



    method Assign_filecolor {fname} {

	   set colorArray($fname) [lindex $colorlist $clr_cnt]
	   incr clr_cnt
    }



    method Addin_Candidate_List { frame } {

        if { $do_not_show_lists } {
           return
        }

        if { $replacement_strategy == "LRU" } {

           lappend LRU_frame_list $frame
           .replacement.lru_list clear
           .replacement.lru_list.listbox insert end [join $LRU_frame_list]
           focus .replacement.lru_list
           return

        } elseif { $show_LRU } {

           lappend LRU_frame_list $frame
           .replacement.lru_list clear
           .replacement.lru_list.listbox insert end [join $LRU_frame_list]
           focus .replacement.lru_list

        } else {
        }

        if { $show_replacement_strategy == 0 } {
           return
        } else {

           .replacement.list clear

	   switch $replacement_strategy {

	          MRU  {
	            set temp [linsert $frame_list 0 $frame]
	            set frame_list $temp
                    .replacement.list.listbox insert end [join $frame_list]
	            focus .replacement.list
                  }
   
	          default {
	          }
	   }
        }
    }



    method Remv_from_Candidate_List { frame } {

        if { $do_not_show_lists } {
           return
        }

        if { $replacement_strategy == "LRU" } {

           set indx [lsearch $LRU_frame_list $frame]
	   if { $indx>= 0 } {
	      set temp [lreplace $LRU_frame_list $indx $indx]
	      set LRU_frame_list $temp
              .replacement.lru_list clear
              .replacement.lru_list.listbox insert end [join $LRU_frame_list]
              focus .replacement.lru_list
           }
           return

        }  elseif { $show_LRU } {

           set indx [lsearch $LRU_frame_list $frame]
           if { $indx>= 0 } {
              set temp [lreplace $LRU_frame_list $indx $indx]
              set LRU_frame_list $temp
              .replacement.lru_list clear
              .replacement.lru_list.listbox insert end [join $LRU_frame_list]
              focus .replacement.lru_list
           }
        } else {
        }

        #
        # Update your other replacement list here, if you have to...
        #
        
        if { $show_replacement_strategy == 0 } {
           return
        } else {

           set indx [lsearch $frame_list $frame]
           if { $indx>= 0 } {
              set temp [lreplace $frame_list $indx $indx]
              set frame_list $temp
              .replacement.list clear
              .replacement.list.listbox insert end [join $frame_list]
              focus .replacement.list
           }
        }
    }



    protected fd                      ""
    protected file_list               ""
    protected maxFrameNumber           0
    protected current_line_count       0
    protected line                    ""
    protected buffer_size             -1
    protected openfile_list           ""
    protected eof                      0

    #global show_lru_list
    #global show_replacement_strategy_list
    #global show_no_list
    protected show_LRU                     0
    protected show_replacement_strategy    0
    protected do_not_show_lists            0


    protected LRU_frame_list    ""
    protected frame_list        ""


    protected replacement_strategy

    protected colorlist    0
  # global list_of_file_colors
  # protected colorlist    $list_of_file_colors


    #
    # colorArray stores the file-color matching
    #

    protected colorArray 

    protected max_no_openedfiles 0
  # global maximum_num_openedfiles
  # protected max_no_openedfiles $maximum_num_openedfiles

    protected clr_cnt         0 
    protected no_openedfiles  0

    protected listed_actions 0
  # global num_of_listed_actions
  # protected listed_actions $num_of_listed_actions

    public stat_pin_pages     0
    public stat_unpin_dirty   0
    public stat_num_actions   0
    public stat_num_pins      0
    public stat_num_reads     0

    #
    # variable used to keep track of the number
    # of frames occupied by pages - after a point
    # this will equal the # of frames, of course
    #

    public stat_occup_frames  0 
}
