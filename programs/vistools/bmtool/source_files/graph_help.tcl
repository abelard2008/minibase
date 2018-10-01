##source config_file

proc CreateGraphHelp { } {
    # build widget .frame0

    global bg_help_color
    global bg_show_man_page_color
    global bg_man_color
    global fg_man_color

    frame .statistics.frame0a \
	    -borderwidth {2} \
	    -relief {raised}
    
    # build widget .statistics.frame0a.menubutton2
    menubutton .statistics.frame0a.menubutton2 \
	    -menu {.statistics.frame0a.menubutton2.m} \
	    -bg $bg_help_color \
	    -text {HELP} 
    
    # build widget .statistics.frame0a.menubutton2.m
    menu .statistics.frame0a.menubutton2.m
    .statistics.frame0a.menubutton2.m add command \
	    -label {Show Statistics Info}  \
	    -background $bg_show_man_page_color \
	    -command {
            exec xterm -bg $bg_man_color -fg $fg_man_color \
                       -geometry 75x45 -title Plots \
                       -e /bin/sh -c "man -local $bm_code_dir/source_files/manual/plot.man"
    }
#                       -e tcsh show_plot_help
    
    # pack widget .statistics.frame0a
    pack append .statistics.frame0a \
	    .statistics.frame0a.menubutton2 {right frame center}
    
    # pack widget .
    pack append .statistics \
	    .statistics.frame0a {top frame center fillx}
    
}
