##source config_file

proc CreateMenu { } {

    global bg_help_color
    global bg_show_man_page_color
    global bg_man_color
    global fg_man_color

    # build widget .frame0
    frame .frame0 \
	    -borderwidth {2} \
	    -relief {raised}
    
    # build widget .frame0.menubutton1
    menubutton .frame0.menubutton1 \
	    -menu {.frame0.menubutton1.m} \
	    -text {File} \
	    -underline {0}
    
    # build widget .frame0.menubutton1.m
    menu .frame0.menubutton1.m
    .frame0.menubutton1.m add command \
	    -label {Print Tree} \
	    -underline {0} \
	    -command {
	.tree print_tree
    }
    .frame0.menubutton1.m add separator
    .frame0.menubutton1.m add command \
	    -label {Exit} \
	    -command {
		       foreach match [glob -nocomplain -- p/*] {
		           exec rm $match
		       }
                       delete_btree
	               exit
	    }
    
    # build widget .frame0.menubutton2
    menubutton .frame0.menubutton2 \
	    -menu {.frame0.menubutton2.m} \
            -bg $bg_help_color \
	    -text {HELP} 
    
    # build widget .frame0.menubutton2.m
    menu .frame0.menubutton2.m
    .frame0.menubutton2.m add command \
	    -label {Show Manual Page} \
            -background $bg_show_man_page_color \
	    -command {

            exec xterm -bg $bg_man_color -fg $fg_man_color  \
                       -geometry 75x45 -title Tickle_BTree \
                       -e /bin/sh -c "man -local $btree_code_dir/source_files/manual/btree.man"
    }
    
    # build widget .frame0.menubutton0
    menubutton .frame0.menubutton0 \
	    -menu {.frame0.menubutton0.m} \
	    -text {View} \
	    -underline {0}
    
    # build widget .frame0.menubutton0.m
    menu .frame0.menubutton0.m
    .frame0.menubutton0.m add command \
	    -label {Toggle Tree Layout} \
	    -command {
	.tree toggle_tree_layout
    }
    
    # pack widget .frame0
    pack append .frame0 \
	    .frame0.menubutton1 {left frame center} \
	    .frame0.menubutton2 {right frame center} \
	    .frame0.menubutton0 {left frame center}
    
    # pack widget .
    pack append . \
	    .frame0 {top frame center fillx}
    
}
