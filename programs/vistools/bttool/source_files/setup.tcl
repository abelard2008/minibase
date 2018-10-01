###################################################################
#
# Below are all the procedures that create the main window containing
# the btrees, the action buttons and the windows showing node conten-
# ts...
#
# Author: Harry Stavropoulos (harryst@cs.wisc.edu)
#
###################################################################


lappend auto_path $btree_code_dir/../library
lappend auto_path $btree_code_dir/source_files

source $btree_code_dir/../library/compat.tcl
source $btree_code_dir/source_files/config_file
set_app_defaults

#
# set the stepsize used in scrolling the log-file (may
# be changed interactively)
#

set nextIncr 1
set insertedKey ""
set insertedPageNo 0
set insertedSlotNo 0
set deletedKey ""
set deletedPageNo 0
set deletedSlotNo 0
set btreeName "btree"
set keySize 220
set deleteType 1

#
# create the main window
#

proc CreateToolBox { } {

    global widthsize 
    global width_steps
    global width_key
    global width_pageno
    global width_slotno
    global width_update
    global bg_window_action
    global fg_window_action
    global font_window_action
    global bg_window_contents
    global fg_window_contents
    global bg_button_act
    global bg_entry_steps
    global fg_entry_steps
    global font_window_contents
    global trace
    global nextIncr
    global insertedKey
    global insertedPageNo
    global insertedSlotNo
    global deletedKey
    global deletedPageNo
    global deletedSlotNo
    global btreeName
    global keySize
    global deleteType
    

    pack [frame .toolbox -relief raised -bd 2 ] \
            -side top -expand 1 -fill both -ipady 1m 

    #
    # create the command buttons
    #

    set commandbuttons ".toolbox.commands"
    frame $commandbuttons 
    frame $commandbuttons.next 


    button $commandbuttons.next.command -text next \
					-width $widthsize \
					-bg wheat \
					-command {
                                        $trace Next $nextIncr
    }
    entry $commandbuttons.next.nextIncr -relief sunken \
                                        -textvariable nextIncr \
                                        -background $bg_entry_steps \
                                        -foreground $fg_entry_steps \
                                        -width $width_steps  -bd 5 
    bind $commandbuttons.next.nextIncr <Key-Return> {$trace Next $nextIncr} 
    
    frame $commandbuttons.step_break
    button $commandbuttons.step_break.step  -text step \
				 -bg wheat \
                                 -width 13 \
				 -command {
            $trace Next 1
    }
    button $commandbuttons.step_break.break -text cont \
                                 -width 13 \
				 -bg wheat \
				 -command {
            $trace Break
    }
    
    frame $commandbuttons.create
    
    button $commandbuttons.create.command -text create_btree \
            -bg wheat \
            -width $width_update \
            -command {
        $trace CreateBtree $btreeName $keySize $deleteType
    }
    entry $commandbuttons.create.btreeName -relief sunken \
            -width 10 \
            -textvariable btreeName \
            -background $bg_entry_steps \
            -foreground $fg_entry_steps 
    entry $commandbuttons.create.keySize -relief sunken \
            -width 3 \
            -textvariable keySize \
            -background $bg_entry_steps \
            -foreground $fg_entry_steps  
    entry $commandbuttons.create.deleteType -relief sunken \
            -width 1 \
            -textvariable deleteType \
            -background $bg_entry_steps \
            -foreground $fg_entry_steps  
            
    frame $commandbuttons.insert 
    button $commandbuttons.insert.command -text insert \
					-width $width_update \
                                        -bg wheat \
					-command {
                                        $trace InsertRecord $insertedKey $insertedPageNo $insertedSlotNo 
                                }
                                $commandbuttons.insert.command config -state "disabled" 
    entry $commandbuttons.insert.key -relief sunken \
                                        -width $width_key \
                                        -textvariable insertedKey \
                                        -background $bg_entry_steps \
                                        -foreground $fg_entry_steps 
    entry $commandbuttons.insert.pageNo -relief sunken \
                                        -width $width_pageno \
                                        -textvariable insertedPageNo \
                                        -background $bg_entry_steps \
                                        -foreground $fg_entry_steps 
    entry $commandbuttons.insert.slotNo -relief sunken \
                                        -width $width_slotno \
                                        -textvariable insertedSlotNo \
                                        -background $bg_entry_steps \
                                        -foreground $fg_entry_steps 
    
    bind $commandbuttons.insert.key <Key-Return> {$trace InsertRecord $insertedKey $insertedPageNo $insertedSlotNo} 
    bind $commandbuttons.insert.pageNo <Key-Return> {$trace InsertRecord $insertedKey $insertedPageNo $inseredSlotno} 
    bind $commandbuttons.insert.slotNo <Key-Return> {$trace InsertRecord $insertedKey $insertedPageNo $insertedSlotNo} 

    frame $commandbuttons.delete 
    button $commandbuttons.delete.command -text delete \
					-width $width_update \
                                        -bg wheat \
					-command {
                                        $trace DeleteRecord $deletedKey $deletedPageNo $deletedSlotNo
    }
    $commandbuttons.delete.command config -state "disabled" 
    entry $commandbuttons.delete.key -relief sunken \
                                        -width $width_key \
                                        -textvariable deletedKey \
                                        -background $bg_entry_steps \
                                        -foreground $fg_entry_steps 
    entry $commandbuttons.delete.pageNo -relief sunken \
                                        -width $width_pageno \
                                        -textvariable deletedPageNo \
                                        -background $bg_entry_steps \
                                        -foreground $fg_entry_steps 
    entry $commandbuttons.delete.slotNo -relief sunken \
                                        -width $width_slotno \
                                        -textvariable deletedSlotNo \
                                        -background $bg_entry_steps \
                                        -foreground $fg_entry_steps 
    bind $commandbuttons.delete.key <Key-Return> {$trace DeleteRecord $deletedKey $deletedPageNo $deletedSlotNo} 
    bind $commandbuttons.delete.pageNo <Key-Return> {$trace DeleteRecord $deletedKey $deletedPageNo $deletedSlotNo} 
    bind $commandbuttons.delete.slotNo <Key-Return> {$trace DeleteRecord $deletedKey $deletedPageNo $deletedSlotNo} 

    

    #
    # create the windows showing the actions taken + the
    # node contents, IN MAIN WINDOW
    #


    Listbox .toolbox.log -vscroll 1 -hscroll 1 
    .toolbox.log.hscroll configure -width 10
    .toolbox.log.vscroll configure -width 10
    .toolbox.log.listbox configure -background $bg_window_action  \
				   -foreground $fg_window_action \
				   -font $font_window_action 
    
    Listbox .toolbox.node_contents -vscroll 1 -hscroll 1 
    .toolbox.node_contents.hscroll configure -width 10
    .toolbox.node_contents.vscroll configure -width 10
    .toolbox.node_contents.listbox configure \
				   -background $bg_window_contents \
                                   -foreground $fg_window_contents \
				   -font $font_window_contents 

    pack .toolbox.log -fill both -expand 1
    pack .toolbox.node_contents -fill both -expand 1
    pack $commandbuttons.next.command -fill x -padx 1m -ipadx 16m
    pack $commandbuttons.next.command $commandbuttons.next.nextIncr \
				      -side left -fill x

    pack $commandbuttons.next

    pack $commandbuttons.step_break.step $commandbuttons.step_break.break -side left -fill x -padx 1m -ipadx 1m
    pack $commandbuttons.step_break
    
    pack $commandbuttons.create.command -fill x -padx 1m -ipadx 16m 
    pack $commandbuttons.create.command $commandbuttons.create.btreeName \
            -side left -fill x
    pack $commandbuttons.create.command $commandbuttons.create.keySize \
            -side left -fill x
    pack $commandbuttons.create.command $commandbuttons.create.deleteType \
            -side left -fill x
    pack $commandbuttons.create
    
    pack $commandbuttons.insert.command -fill x -padx 1m -ipadx 16m
    pack $commandbuttons.insert.command $commandbuttons.insert.key \
				      -side left -fill x
    pack $commandbuttons.insert.command $commandbuttons.insert.pageNo \
				      -side left -fill x
    pack $commandbuttons.insert.command $commandbuttons.insert.slotNo \
				      -side left -fill x
    pack $commandbuttons.insert
    
    pack $commandbuttons.delete.command -fill x -padx 1m -ipadx 16m
    pack $commandbuttons.delete.command $commandbuttons.delete.key \
				      -side left -fill x
    pack $commandbuttons.delete.command $commandbuttons.delete.pageNo \
				      -side left -fill x
    pack $commandbuttons.delete.command $commandbuttons.delete.slotNo \
				      -side left -fill x
    pack $commandbuttons.delete
    
    pack $commandbuttons.create.command -fill x -padx 1m -ipadx 16m 
    pack $commandbuttons.create.command $commandbuttons.create.btreeName \
            -side left -fill x
    pack $commandbuttons.create.command $commandbuttons.create.keySize \
            -side left -fill x
    pack $commandbuttons.create.command $commandbuttons.create.deleteType \
            -side left -fill x
    pack $commandbuttons.create
    
    pack $commandbuttons
    
    if {[string match */ $trace] || $trace == ""} {
        .toolbox.commands.next.command config -state "disabled"
        .toolbox.commands.step_break.step config -state "disabled"
        .toolbox.commands.step_break.break config -state "disabled"
    }
}

proc setTrace {f} {
    global trace
    set trace $f
}

fileselect setTrace

. configure -background $bg_color

CreateMenu

BTree .tree
pack .tree -side left -fill both -expand 1

#
# create the window to show the old path
#

toplevel .show_old_path
CreateOldPathMenu
Old_Path .show_old_path.path
pack .show_old_path.path -side left -fill both -expand 1

#
# create the window to show the contents of nodes before the
# update
# 

toplevel .old_path_snap
Listbox .old_path_snap.log -hscroll 1 -vscroll 1
.old_path_snap.log.hscroll configure -width 10
.old_path_snap.log.vscroll configure -width 10
.old_path_snap.log.listbox configure  \
			     -background $bg_node_old_contents \
			     -foreground $fg_node_old_contents \
	                     -font $font_node_old_contents \
			     -geometry $geom_node_old_contents
pack .old_path_snap.log

CreateToolBox 

#
# set the max dimensions of the windows - make them resizable
#

wm positionfrom . user
wm sizefrom . ""
wm maxsize . 1152 900
wm title . $main_window_title

wm sizefrom .show_old_path ""
wm maxsize .show_old_path 1152 900
wm title .show_old_path $path_followed_window_title

wm sizefrom .old_path_snap
wm maxsize .old_path_snap 1152 900
wm title .old_path_snap $old_contents_window_title
