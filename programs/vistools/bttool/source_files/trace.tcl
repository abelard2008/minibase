###################################################################
#
# Class: Trace
#
# Description: The actions taken as the log-file is scanned are
#              defined in this class. Also the actions taken
#              when the user presses the control buttons.
#
# Author : Harry Stavropoulos (harryst@cs.wisc.edu)
#
###################################################################


##source config_file


itcl_class Trace  {

    constructor {config} {

        if { ![file readable $this] } {
            tkerror "$this: can not read the file"
        } else {
            set fd [ open $this r ]
        }
    }
    destructor {
        close $fd
    }

    #                                       
    # in_break_mode - set to 1 so that Next{} will return
    #                 if it encounters a breakpoint (denoted
    #                 by a * as the first characted in a line
    #                 in the log file, followed by a space/nl)
    #

    method Break {} {

        update idletasks
 
	set in_break_mode 1

	while { [Next 1] != "breakpoint" } {}
	set in_break_mode 0
    }


    #                                       
    # numlines - the number of lines in the log file that
    #            the trace should proceed 
    #

    method Next { numLines } {

        global nextIncr
	global leaf_bm
	global node_bm
	global bg_button_act
	global bg_color
	global act_button_width
	global cont_button_act_text
	global cont_button_act_fg
	global cont_button_act_bg
	global cont_button_noact_fg


	if { $fd == "" || $fd == "-1" } {
            tkerror "Log file is not open"
            return
        }

	show_WAIT_label

        update idletasks


	for { set i 0 } { $i < $numLines } { incr i } {
            if { [gets $fd line] == "-1" } {
		# tkerror "we have reached the EOF"
		.toolbox.log append "EOF"
                show_std_label
                DisableReadTrace
                
                return "breakpoint"
            }


	    switch [ lindex $line 0 ] {

		STARTUP {
		    .toolbox.log append $line
		    .toolbox.log append \n
		    .tree clear_selection
		    catch {exec rm -f btree_file}
		    create_btree btree_file [lindex $line 1] [lindex $line 2]
                    set key_size [lindex $line 1]
                    EnableUpdate
                    .toolbox.commands.create.command config -state "disabled"
		}
                
		DONE {

		    Insrt $spaces $line
		    Insrt $spaces $spaces
		    .tree clear_selection
		    exec cp temp_node_contents path_snapshot
		    set visited_node_num -1
                    EnableUpdate
		}

		DO {
		    Insrt $spaces $line
		    incr visited_node_num
		}

		* {
		    if { $in_break_mode == "1" } {
                        .toolbox.log append "breakpoint"
                        show_std_label
		        return "breakpoint"
                    } else {
			incr i -1
		    }
                }

		NEWROOT {
		    Insrt $spaces $line 
                    set empty_tree 0
		    .tree newroot [lindex $line 1]
		}

		ROOTSPLIT {
                    Insrt $spaces $line
		    .tree rootsplit [lindex $line 3] \
			    [lindex $line 4 ]
              
		}
		
                CHANGEROOT {
                    Insrt $spaces $line
                    .tree changeroot [lindex $line 3] [lindex $line 6]
                }
                
                DEALLOCATEROOT {
                    Insrt $spaces $line
                    .tree deallocateroot [lindex $line 1]
                    set empty_tree 1
                }
                
		VISIT {
		    set old_tree_exists 1
		    set cur_node previous_[lindex $line 2]
                    Insrt $spaces $line
		    .tree visit_node [lindex $line 2]

		    incr visited_node_num

		    if {$visited_node_num == 1} {

			set cur_root [lindex $line 2]
			.show_old_path.path oldroot $cur_root

                    } else {

			if { [.tree.tree isleaf n[lindex $line 2] ] } {
			    set new_bitmap $leaf_bm
			} else {
			    set new_bitmap $node_bm
                        }
			.show_old_path.path.tree add_node \
					 previous_$cur_parent $cur_node \
					 -label $cur_node \
					 -bitmap $new_bitmap

                        .show_old_path.path.tree.c.canvas bind \
				$cur_node <Double-Button> \
					      "dump_this_node $cur_node"
                    }

		    .show_old_path.path.tree center
		    .show_old_path.path.tree draw
		    record_visited [lindex $line 2]
		    set cur_parent [lindex $line 2]
		}

		SPLIT {
                    Insrt $spaces $line
		    .tree split [lindex $line 5 ] [ lindex $line 6]
		}

		MERGE {
                    Insrt $spaces $line
		    .tree merge [lindex $line 2 ] [ lindex $line 3]
		}

		CHILDREN {
                    Insrt $spaces $line
		    .tree update_children $line
		}    

		INSERT {
		    .toolbox.log append $line
		    set index [ expr [.toolbox.log.listbox size]-1 ]
		    .toolbox.log.listbox select from $index
		    .toolbox.log.listbox yview $index
		    set cur_command $line		    
		    Remove_Previous_Path
                    DisableUpdate
		}

		DELETE {
		    .toolbox.log append $line
		    set index [ expr [.toolbox.log.listbox size]-1 ]
		    .toolbox.log.listbox select from $index
		    .toolbox.log.listbox yview $index
		    set cur_command $line
		    Remove_Previous_Path
                    DisableUpdate
		}

		PUTIN {
                    Insrt $spaces $line
		    if { $cur_command != "" } {
			set key_to_insert [lrange $cur_command 3 end]
			insert_btree_record $key_to_insert [lindex $cur_command 1] [lindex $cur_command 2]
			set cur_command ""
		    }
		}

		TAKEFROM {
                    Insrt $spaces $line
		    if { $cur_command != "" } {
			set key_to_delete [lrange $cur_command 3 end]
			delete_btree_record $key_to_delete [lindex $cur_command 1] [lindex $cur_command 2]
			set cur_command ""
		    }
		}
	    }

	}

        show_std_label

	return $i
    }



    method Insrt { str1 str2 } {
        .toolbox.log append "$str1 $str2"
    }

    
    #
    # Remove the icon of the path previously followed.
    #

    method Remove_Previous_Path {} {

        exec cp /dev/null temp_node_contents

        foreach match [glob -nocomplain -- p/*] {
             exec rm $match
        }
        if {$old_tree_exists && !$empty_tree } {
             .show_old_path.path.tree rmlink previous_$cur_root
        }
        .toolbox.node_contents.listbox delete 0 end
        .old_path_snap.log.listbox delete 0 end
     }


     method CreateBtree {btreeName keySize deleteType} {
         set command "STARTUP "
         append command $keySize
         append command " "
         append command $deleteType
         .toolbox.log append $command
         catch {exec rm -f $btreeName}
         create_btree $btreeName $keySize $deleteType
         set key_size $keySize
         DisableReadTrace
         EnableUpdate
         .toolbox.commands.create.command config -state "disabled"
     }
     
     
     
     method InsertRecord { key page_no slot_no } {
         # padding so that the key is really of lenght key_size
         set i [string length $key]
         incr i
         incr i
         incr i
         if { $i < $key_size} {
             set zero "  "
             set key $key$zero 
             incr i
             set zero " "
             while { $i < $key_size } {
                 set key $key$zero
                 incr i
             }
             set zero "0\0"
             set key $key$zero
         }
         
         DisableReadTrace
         insert_btree_record $key $page_no $slot_no
         UseTrace
     }
     
     method DeleteRecord {key page_no slot_no } {
         # padding so that the key is really of length key_size
         set i [string length $key]
         incr i
         incr i
         incr i
         if { $i < $key_size} {
             set zero "  "
             set key $key$zero 
             incr i
             set zero " "
             while { $i < $key_size } {
                 set key $key$zero
                 incr i
             }
             set zero "0\0"
             set key $key$zero
         }
         
         DisableReadTrace
         delete_btree_record $key $page_no $slot_no
         UseTrace
     }
     
     method UseTrace {} {
         # only use the trace to update the shape of the tree. 
         # no need to insert/delete the record to minibase in this case

         global leaf_bm
         global node_bm
         global bg_button_act
         global bg_color
         global act_button_width
         global cont_button_act_text
         global cont_button_act_fg
         global cont_button_act_bg
         global cont_button_noact_fg

         while { [gets $trace_fd line] != "-1" } {
             switch [ lindex $line 0 ] {
                 DONE {
                     Insrt $spaces $line
                     Insrt $spaces $spaces
                     .tree clear_selection
                     exec cp temp_node_contents path_snapshot
                     set visited_node_num -1
                 }

                 DO {
                     Insrt $spaces $line
                     incr visited_node_num
                 }

                 NEWROOT {
                     Insrt $spaces $line 
                     set empty_tree 0
                     .tree newroot [lindex $line 1]
                 }

                 ROOTSPLIT {
                     Insrt $spaces $line
                     .tree rootsplit [lindex $line 3] \
                             [lindex $line 4 ]
                     
                 }
                 
                 CHANGEROOT {
                     Insrt $spaces $line
                     .tree changeroot [lindex $line 3] [lindex $line 6]
                 }
                 
                 DEALLOCATEROOT {
                     Insrt $spaces $line
                     .tree deallocateroot [lindex $line 1]
                     set empty_tree 1
                 }
                 
                 VISIT {
                     set old_tree_exists 1
                     set cur_node previous_[lindex $line 2]
                     Insrt $spaces $line
                     .tree visit_node [lindex $line 2]

                     incr visited_node_num

                     if {$visited_node_num == 1} {
                         
                         set cur_root [lindex $line 2]
                         .show_old_path.path oldroot $cur_root

                     } else {

                         if { [.tree.tree isleaf n[lindex $line 2] ] } {
                             set new_bitmap $leaf_bm
                         } else {
                             set new_bitmap $node_bm
                         }
                         .show_old_path.path.tree add_node \
                                 previous_$cur_parent $cur_node \
                                 -label $cur_node \
                                 -bitmap $new_bitmap

                         .show_old_path.path.tree.c.canvas bind \
                                 $cur_node <Double-Button> \
                                 "dump_this_node $cur_node"
                     }

                     .show_old_path.path.tree center
                     .show_old_path.path.tree draw
                     record_visited [lindex $line 2]
                     set cur_parent [lindex $line 2]
                 }

                 SPLIT {
                     Insrt $spaces $line
                     .tree split [lindex $line 5 ] [ lindex $line 6]
                 }

                 MERGE {
                     Insrt $spaces $line
                     .tree merge [lindex $line 2 ] [ lindex $line 3]
                 }

                 CHILDREN {
                     Insrt $spaces $line
                     .tree update_children $line
                 }    
                 
                 INSERT {
                     .toolbox.log append $line
                     set index [ expr [.toolbox.log.listbox size]-1 ]
                     .toolbox.log.listbox select from $index
                     .toolbox.log.listbox yview $index
                     set cur_command $line		    
                     Remove_Previous_Path
                 }

                 DELETE {
                     .toolbox.log append $line
                     set index [ expr [.toolbox.log.listbox size]-1 ]
                     .toolbox.log.listbox select from $index
                     .toolbox.log.listbox yview $index
                     set cur_command $line
                     Remove_Previous_Path
                 }

                 PUTIN {
                     Insrt $spaces $line
                     set cur_command ""
                 }

                 TAKEFROM {
                     Insrt $spaces $line
                     set cur_command ""
                 }
             }
         }
         
         show_std_label

     }
     
     method EnableUpdate {} {
         .toolbox.commands.insert.command config -state "active"
         .toolbox.commands.delete.command config -state "active"
     }
     
     method DisableUpdate {} {
         .toolbox.commands.insert.command config -state "disabled"
         .toolbox.commands.delete.command config -state "disabled"
     }
     
     method DisableReadTrace {} {
         if { $read_trace_disabled == 0 } {
             set read_trace_disabled 1
             
             .toolbox.commands.next.command config -state "disabled"
             .toolbox.commands.step_break.step config -state "disabled"
             .toolbox.commands.step_break.break config -state "disabled"

             set trace_fd [open "btree_trace" r]
             seek $trace_fd 0 end
         }
     }
     
     #
     # Change the labels on the buttons to indicate that the
     # application is busy.
     #

     method show_WAIT_label {} {
        global cont_button_act_bg  cont_button_act_fg

        .toolbox.commands.next.command configure \
                                   -activebackground $cont_button_act_bg \
                                   -activeforeground $cont_button_act_fg \
                                   -bg $cont_button_act_bg \
                                   -fg $cont_button_act_fg \
                                   -text "WAIT"  -command {}

        .toolbox.commands.step_break.step configure \
                                   -activebackground $cont_button_act_bg \
                                   -activeforeground $cont_button_act_fg \
                                   -bg $cont_button_act_bg \
                                   -fg $cont_button_act_fg \
                                   -text "WAIT"  -command {}

        .toolbox.commands.step_break.break configure \
                                   -activebackground $cont_button_act_bg \
                                   -activeforeground $cont_button_act_fg \
                                   -bg $cont_button_act_bg \
                                   -fg $cont_button_act_fg \
                                   -text "WAIT"  -command {}
                           
       .toolbox.commands.create.command config -state "disabled"
    }


    #
    # Show the "standard" labels of the buttons.
    #

    method show_std_label {} {
        global bg_button_act cont_button_noact_fg bg_color

        .toolbox.commands.next.command configure \
                                   -activebackground $bg_button_act \
                                   -activeforeground $cont_button_noact_fg \
                                   -bg $bg_color \
                                   -fg $cont_button_noact_fg \
                                   -text "next" \
                                   -command {
                                             $trace Next $nextIncr
                                   }

        .toolbox.commands.step_break.step configure \
                                   -activebackground $bg_button_act \
                                   -activeforeground $cont_button_noact_fg \
                                   -bg $bg_color \
                                   -fg $cont_button_noact_fg \
                                   -text "step" \
                                   -command {
                                             $trace Next 1
                                   }


        .toolbox.commands.step_break.break configure \
                                   -activebackground $bg_button_act \
                                   -activeforeground $cont_button_noact_fg \
                                   -bg $bg_color \
                                   -fg $cont_button_noact_fg \
                                   -text "cont" \
                                   -command {
                                             $trace Break
                                   }
    }

    protected fd ""
    protected cur_command ""
    protected call_from_break 0
    protected in_break_mode 0
    protected spaces "    "
    protected visited_node_num -1
    protected cur_parent ""
    protected cur_root ""
    protected old_tree_exists 0
    protected empty_tree 1
    protected key_size 0
    protected read_trace_disabled 0
    protected trace_fd ""
    public fname "btree_log"
}
