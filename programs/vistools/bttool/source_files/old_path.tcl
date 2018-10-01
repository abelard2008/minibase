###################################################################
#
# Class: Old_Path
#
# Description: This is the search path that has been followed.  All the 
#              actions taken on it are defined here.
#
# Author : Harry Stavropoulos (harryst@cs.wisc.edu)
#
###################################################################


##source config_file


itcl_class Old_Path {

    constructor {config} {

	global nodedistance_old_path
	global bwidth_old_path
	global bm_color_old_path

	set class [$this info class]
        ::rename $this $this-tmp-
        ::frame $this -class $class -relief flat
        ::rename $this $this-win-
        ::rename $this-tmp- $this
	
	set tree [ Tree $this.tree \
                -parentdistance $nodedistance_old_path \
                -borderwidth $bwidth_old_path \
                -layout vertical \
                -bitmap_color $bm_color_old_path ]
	
	pack $tree -side top -fill both -expand 1
	
	#
	# set canvas variable.
	#

	set canvas "$tree.c.canvas"
    }

 
    method oldroot { newroot } {

	global root_bm

        set newroot previous_$newroot

        $tree add_node "" $newroot -label $newroot -bitmap $root_bm
        $canvas bind $newroot <Double-Button> "dump_this_node $newroot"

        $tree center
        $tree draw
    }


    #
    # Display a print dialog to print the tree as postscript
    #

    method print_tree {} {
	utilReUseWidget CanvasPrint $this.printtree -canvas $canvas
    }


    #
    # toggle between vertical and horizontal layout
    #

    method toggle_tree_layout {} {
        $tree toggle_layout
    }
 

    protected tree ""
    protected canvas ""
}



proc dump_this_node { node } {

    set fd [ open p/$node r ]
    .old_path_snap.log.listbox delete 0 end

    while { [gets $fd line] != -1 } {
       if { $line == "EOPG" } {
          break
       }
       .old_path_snap.log append $line
    }
}
