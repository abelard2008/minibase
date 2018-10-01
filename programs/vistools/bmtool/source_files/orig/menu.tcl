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
    
    menubutton .frame0.menubutton1 \
            -menu {.frame0.menubutton1.m} \
            -text {File} 

    menu .frame0.menubutton1.m
    .frame0.menubutton1.m add command \
            -label {Exit} \
            -command {
                       exit
            }

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
            exec xterm -bg $bg_man_color -fg $fg_man_color \
                       -geometry 75x45 -title Tickle_BM \
                       -e /bin/sh -c "man -local $bm_code_dir/source_files/manual/bm.man"
    }
#                       -e tcsh show_man_page
    
    # pack widget .frame0
    pack append .frame0 \
            .frame0.menubutton1 {left frame center} \
	    .frame0.menubutton2 {right frame center}
    
    # pack widget .
    pack append . \
	    .frame0 {top frame center fillx}
    
}
