###################################################################
#
# Below is the definition of the listbox showing the contents of
# the log-file. It is configured (in config_file) to depict the
# last 25 actions taken by the bitmap - if you want to change this
# play with the the definitions in config_file.
#
# Author: Harry Stavropoulos (harryst@cs.wisc.edu)
#
###################################################################


##source config_file 


itcl_class Actionlist {

    constructor { config } {

    global action_list
    global bg_action_list
    global fg_action_list
    global action_list_font
    global action_list_geom


    Listbox $parent.log
    $parent.log.listbox configure -background $bg_action_list \
			          -foreground $fg_action_list \
                                  -font $action_list_font     \
				  -geometry $action_list_geom

    set action_list $parent.log.listbox
    pack $parent.log

    bind $action_list  <Double-Button-1> {
	Goto %W %y
    }

    }

    method Clear { } {
	$parent.log clear
    }

    protected listing ""
    public parent "."
} 


proc Goto { w y } {

    global traceFile

    set temp [$w nearest $y]
    $traceFile Start
    $traceFile Next $temp 0
    $w select clear
    update idletasks
}
