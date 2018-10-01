# Tree.tcl - itcl widget for displaying trees
#
# This widget is based on the Tk tree widget, which is implemented 
# in C++.  It simplifies things by creating its own frame, canvas 
# and scrollbars and defines a standard layout, where each node may
# have a label, a bitmap or both.
# 
# See the man page itclTree(n) for a description.
#
# Author: Allan Brighton (allan@piano.sta.sub.org) 

itcl_class Tree {
    inherit FrameWidget

    #  create a new Tree widget

    constructor {config} {
	FrameWidget::constructor
	
	# create Canvas object
	set canvas_obj [Canvas $this.c \
		-relief sunken -bd 3]
	# name of Tk canvas
	set canvas $canvas_obj.canvas
	set canvas_bg [utilGetConfigValue $canvas -bg]
	pack $canvas_obj -fill both -expand 1

        # need a more general way of combining public vars and resources...
	if {"$font" == ""} {
	    set font -adobe-helvetica-bold-r-*-*-*-120-*-*-*-*-*-*
	}

	# create the tree widget
	set tree [tree $canvas.t \
		-parentdistance $parentdistance \
		-borderwidth $borderwidth \
		-layout $layout]
	set_default_bindings
    }

    
    # Set the bindings for the tree canvas
    
    method set_default_bindings {} {
	$canvas bind text <1> "focus %W; $this select_node"
	$canvas bind bitmap <1> "$this select_bitmap"
	$canvas bind text <Shift-1> "focus %W; $this select_node"
	$canvas bind bitmap <Shift-1> "$this select_bitmap"
    }
    

    # Make sure the tree is visible in the canvas window by scrolling to center
    # the tree in the view

    method center {} {
	set scrollincrement [utilGetConfigValue $canvas -scrollincrement]
	set scrollregion [utilGetConfigValue $canvas -scrollregion]
	scan $scrollregion "%d %d %d %d" x1 y1 x2 y2
	
	if {[utilGetConfigValue $tree -layout] == "horizontal"} {
	    set height [utilGetConfigValue $canvas -height]
	    $canvas yview [expr {(($y2 - $y1)/2 - $height/2)/$scrollincrement}]
	    $canvas xview 0
	} else {
	    set width [utilGetConfigValue $canvas -width]
	    $canvas xview [expr {(($x2 - $x1)/2 - $width/2)/$scrollincrement}]
	    $canvas yview 0
	}
    }
    
  
    # layout the components of the given node depending on whether
    # the tree is vertical or horizontal
    
    method layout_node {node} {
	set label $node:text
	set bitmap $node:bitmap
	
	if {[utilGetConfigValue $tree -layout] == "horizontal"} {
	    if {[scan [$canvas bbox $label] "%d %d %d %d" x1 y1 x2 y2] == 4} {
		$canvas itemconfig $bitmap -anchor se
		$canvas coords $bitmap $x1 $y2
	    }
	} else {
	    if {[scan [$canvas bbox $bitmap] "%d %d %d %d" x1 y1 x2 y2] == 4} {
		$canvas itemconfig $label -anchor n
		$canvas coords $label [expr {$x1+($x2-$x1)/2}] $y2
	    }
	}
    }
    
    
    # add the given node to the tree under the given parent node,
    # if not already there
    #
    # optional args: -label     - text for node label
    #                -bitmap    - bitmap to use for nodes
    #
    
    method add_node {parent node args} {
	
	set label {}
	set bitmap {}
	utilGetArgs $args

	if {"$label" == "" && "$bitmap" == ""} {
	    set label $node
	}

	# don't add the node if its already there
	if [llength [$canvas find withtag $node]] {
	    return
	}
	
	if {"$bitmap" != ""} {
	    $canvas create bitmap 0 0 -bitmap $bitmap \
		    -tags [list $node bitmap $node:bitmap] \
		    -foreground $bitmap_color \
		    -background $canvas_bg
	}
	
	if {"$label" != ""} {
	    $canvas create text 0 0 \
		    -text "$label" \
		    -font $font \
		    -tags [list $node text $node:text]
	}

	set line [$canvas create line 0 0 0 0 \
		-tag [list line $parent:$node:line] \
		-width 1 \
		-capstyle round \
		-fill $line_color]
	
	layout_node $node
	$tree addlink $parent $node $line -border 2
    }
    
    
    # select the current node's label
    
    method select_node {} {
	set tag [lindex [$canvas gettags current] 0]
	$canvas select from current 0
	$canvas select to current \
		[string length [lindex [$canvas itemconf current -text] 4]]
	deSelect_bitmap
	set selected_node $tag
    }
    
    
    # de-select all node labels
    
    method deselect_node {} {
	$canvas select clear
	set selected_node {}
    }
    

    # highlight the node's bitmap
    
    method select_bitmap {} {
	set tag [lindex [$canvas gettags current] 0]
	focus none
	deselect_node
	deSelect_bitmap
	# save bitmaps bg color
	$canvas itemconf current \
		-background $bitmap_select_bg \
		-tags "[$canvas gettags current] selected" 
	set selected_node $tag
    }


    # stop highlighting the node's bitmap
    
    method deSelect_bitmap {} {
	$canvas itemconfig selected -background $canvas_bg
	$canvas dtag selected
	set selected_node {}
    }
    

    # return the tag of the item currently selected
    
    method get_selected {} {
	return $selected_node
    }


    # return the tag of the current item (the item under the mouse)
    
    method get_current {} {
	return [lindex [$canvas gettags current] 0]
    }


    # Toggle the layout of the tree between vertical and horizontal

    method toggle_layout {} {
	set scrollincrement [utilGetConfigValue $canvas -scrollincrement]
	if {[utilGetConfigValue $tree -layout] == "horizontal"} {
	    $tree config -layout vertical
	} else {
	    $tree config -layout horizontal
	}
	
	# change the layout of the nodes so that the bitmap is on top for
	# vertical trees and at left for horizontal trees
	foreach i [$canvas find withtag text] {
	    set node [lindex [$canvas gettags $i] 0]
	    layout_node $node
	    $tree nodeconfig $node
	}
	
	$tree draw
	center
    }


    # pass these methods on to the tree widget unchanged
    # (this just generates methods on the fly...)

    foreach i {addlink ancestors child draw isleaf isroot movelink \
	       nodeconfigure nodeconfig prune parent root rmlink \
	       sibling subnodes} {
	method $i {args} [format {return [eval "$tree %s $args"]} $i]
   }
   
    
    # -- public vars --

    
    # default line color
    public line_color gray50
 
    # tree widget options 
    public parentdistance 30
    public borderwidth 20
    public layout horizontal

    # bitmap color
    public bitmap_select_bg lightblue
    public bitmap_color gray50

    # font to use for tree labels
    public font {}


    # -- protected vars --

    
    # tree widget
    protected tree

    # itcl Canvas widget
    protected canvas_obj

    # tk canvas widget
    protected canvas

    # background color of canvas
    protected canvas_bg
    
    # currently selected node
    protected selected_node {}
}


