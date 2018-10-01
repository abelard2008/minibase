###################################################################
#
# Class: BTree
#
# Description: This the Btree; all the actions taken on it are
#              defined here.
#
# Author : Huseyin Bektas (bektas@cs.wisc.edu)
#
###################################################################

##source config_file

itcl_class BTree {

    constructor {config} {

	global nodedistance
	global bwidth
	global bm_color

    #puts stderr "This is $this"
    #flush stderr

	set class [$this info class]
        ::rename $this $this-tmp-
        ::frame  $this -class $class -relief flat
        ::rename $this $this-win-
        ::rename $this-tmp- $this
	
	set tree [ Tree $this.tree \
                -parentdistance $nodedistance \
                -borderwidth $bwidth \
                -layout vertical \
                -bitmap_color $bm_color ]
	
	pack $tree -side top -fill both -expand 1
	
	#
	# set canvas variable.
	#

	set canvas "$tree.c.canvas"
        
    }


    
    method newroot { newroot } {

	global child_bm

	set newroot n$newroot

	$tree add_node "" $newroot -label $newroot -bitmap $child_bm
	$canvas bind $newroot <Double-Button> "$this print_selected"
	lappend cur_select $newroot
	
	if { $left != "" && $right != "" } {
	    $tree movelink $left $newroot
	    $tree movelink $right $newroot
	}

	set root $newroot
        
	$tree center
	$tree draw
    }


 
    method print_selected {} {

	set page_id [ $tree get_selected ]
	set page_id [ string range $page_id 1 end ]
	exec cp /dev/null temp_node_contents
	print_btree_page $page_id 

	#
	# EOPG to know where to stop
	#

	puts "EOPG"
	flush stdout
	set fd [ open show_node_contents w+]
	exec cp temp_node_contents show_node_contents
	flush $fd
	seek $fd 0 start
	.toolbox.node_contents.listbox delete 0 end

	while { [gets $fd line] != -1 } {
	   if { $line == "EOPG" } {
	      break
           }
	   .toolbox.node_contents append $line
	}
    }
    


    method rootsplit { lchild rchild } {

	global child_bm

	set left n$lchild
	set right n$rchild
	
	if { $left != $root } {
	    $tree add_node "" $left -label $left -bitmap $child_bm
	    lappend cur_select $left
	    $canvas bind $left <Double-Button> "$this print_selected"
	}
	if { $right != $root } {
	    $tree add_node "" $right -label $right -bitmap $child_bm
	    lappend cur_select $right
	    $canvas bind $right <Double-Button> "$this print_selected"
	}
	$tree center
	$tree draw
    }
    

    
    method changeroot { oldroot newroot } {

        global root_bm
        
	set old n$oldroot
	set new n$newroot
	
        set cur_select [lreplace $cur_select 0 0]
        
        $tree root $new
        $canvas itemconfig $new:bitmap -bitmap $root_bm

	$tree center
	$tree draw
    }



    method deallocateroot { oldroot } {
                
        set old n$oldroot
        
        $tree rmlink $old
        
        set child_list ""
        set cur_select ""
        set root ""
        set left ""
        set right ""
        
        #$tree center
        #$tree draw
    }
    
    
    
    method split { lchild rchild } {

	global child_bm 
	global node_bm

	set left n$lchild
	set right n$rchild
	set parent [ $tree parent $left] 
	set child_list [ $tree subnodes $parent ]
	
	$tree add_node "" right_tree
	set isRight 0
	
	foreach i $child_list {
	    if { $i == $left } {
		set isRight 1
		continue
	    }
	    if { $isRight } {
		$tree movelink $i right_tree
	    }
	}
	
	$tree add_node [$tree parent $left] $right -label $right \
						   -bitmap $child_bm
	$canvas bind $right <Double-Button> "$this print_selected"
	lappend cur_select $right
	
	foreach i [ $tree subnodes right_tree ] {
	    $tree movelink $i $parent
	}
	
	if { ![ $tree isleaf $right ] } {
	    $canvas itemconfigure $left:bitmap -bitmap $node_bm
	}
	
	$tree rmlink right_tree
	$tree draw
    }
    
    

    method merge { lchild rchild } {

	global child_bm 
	global node_bm
        
	set left n$lchild
	set right n$rchild
        
        set child_list [$tree subnodes $right]
        foreach i $child_list {
            $tree movelink $i $left
        }

        $tree rmlink $right
        
        set position [lsearch $cur_select $right]
        set cur_select [lreplace $cur_select $position $position]
	set cur_select [lappend cur_select $left]
        
        $tree draw
    }
    
    

    method update_children { line } {

	global node_bm

	if { "n[lindex $line 1]" != $left } {
	    set cur "n[ lindex $line 1 ]"
	    set children_list [ lrange $line 3 end ]
	    
	    foreach i $children_list {
		if { ![ $tree isleaf "n$i" ] && \
			[ lsearch $cur_select "n$i"] != -1 } {
		    $canvas itemconfigure n$i:bitmap -bitmap $node_bm
		}
#the following is not always working
#                $tree movelink "n$i" $cur
	    }
            
            # heck to get around the previous commented problem 
            $tree add_node "" temp_node
            foreach i $children_list {
                $tree movelink "n$i" temp_node
            }
            foreach i $children_list {
                $tree movelink "n$i" $cur
            }

	    if { ![ $tree isleaf $cur ] && \
		    [ lsearch $cur_select "n$i" ] != -1 } {
		$canvas itemconfigure $cur:bitmap -bitmap $node_bm
	    }

            $tree center
	    $tree draw	
            
            # heck to make the tree's shape correctly drawn (bug in tktree?)
            $tree movelink temp_node $cur
            $tree rmlink temp_node
            $tree draw
	}
    }
	


    method visit_node { node } {

	global magnify_bm 

	set node_num $node
	set node n$node
	lappend cur_select $node
	$canvas itemconfig $node:bitmap -bitmap $magnify_bm 
	$tree draw
    }


    
    method center {} {
	$tree center
    }
 


    method clear_selection {} {

	global root_bm
	global leaf_bm
	global node_bm

	foreach i $cur_select {
	    if { [$tree isroot $i] } {
		$canvas itemconfig $i:bitmap -bitmap $root_bm
	    } elseif { [ $tree isleaf $i ] } {
		$canvas itemconfig $i:bitmap -bitmap $leaf_bm
	    } else {
		$canvas itemconfig $i:bitmap -bitmap $node_bm
	    }
	}
	set cur_select ""
	$canvas select clear
	$tree select_bitmap
    }


    #
    # toggle between vertical and horizontal layout
    #

    method toggle_tree_layout {} {
        $tree toggle_layout
    }


    #
    # Display a print dialog to print the tree as postscript
    #

    method print_tree {} {
        utilReUseWidget CanvasPrint $this.printtree -canvas $canvas
    }



    protected child_list ""
    protected tree ""
    protected canvas ""
    protected cur_select ""
    protected root ""
    protected left ""
    protected right ""
}




proc record_visited { node } {

    exec cp /dev/null temp_node_contents
    print_btree_page $node

    #
    # EOPG to know where to stop
    #

    puts "EOPG"
    flush stdout
    exec cp temp_node_contents p/previous_$node
}

