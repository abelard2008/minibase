##source config_file

proc CreateOldPathMenu { } {

    global bg_help_color
    global bg_show_man_page_color
    global bg_man_color
    global fg_man_color

    # build widget .show_old_path.frame0
    frame .show_old_path.frame0 \
	    -borderwidth {2} \
	    -relief {raised}
    
    # build widget .frame0.menubutton1
    menubutton .show_old_path.frame0.menubutton1 \
	    -menu {.show_old_path.frame0.menubutton1.m} \
	    -text {File} \
	    -underline {0}
    
    # build widget .show_old_path.frame0.menubutton1.m
    menu .show_old_path.frame0.menubutton1.m
    .show_old_path.frame0.menubutton1.m add command \
	    -label {Print Tree} \
	    -underline {0} \
	    -command {
	.show_old_path.path print_tree
    }

    # build widget .show_old_path.frame0.menubutton2
    menubutton .show_old_path.frame0.menubutton2 \
            -menu {.show_old_path.frame0.menubutton2.m} \
            -bg $bg_help_color \
            -text {HELP}

    # build widget .show_old_path.frame0.menubutton2.m
    menu .show_old_path.frame0.menubutton2.m
    .show_old_path.frame0.menubutton2.m add command \
            -label {Show Help Page} \
            -background $bg_show_man_page_color \
            -command {

            exec xterm -bg $bg_man_color -fg $fg_man_color  \
                       -geometry 75x45 -title Tickle_BTree \
                       -e /bin/sh -c "man -local $btree_code_dir/source_files/manual/old_path.man"
    }
    
    # build widget .show_old_path.frame0.menubutton0
    menubutton .show_old_path.frame0.menubutton0 \
	    -menu {.show_old_path.frame0.menubutton0.m} \
	    -text {View} \
	    -underline {0}
    
    # build widget .show_old_path.frame0.menubutton0.m
    menu .show_old_path.frame0.menubutton0.m
    .show_old_path.frame0.menubutton0.m add command \
	    -label {Toggle Tree Layout} \
	    -command {
	.show_old_path.path toggle_tree_layout
    }
    
    # pack widget .show_old_path.frame0
    pack append .show_old_path.frame0 \
	    .show_old_path.frame0.menubutton1 {left frame center} \
	    .show_old_path.frame0.menubutton2 {right frame center} \
	    .show_old_path.frame0.menubutton0 {left frame center}
    
    # pack widget .show_old_path
    pack append .show_old_path \
	    .show_old_path.frame0 {top frame center fillx}
}
