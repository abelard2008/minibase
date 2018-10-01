# tree.tcl - utility class for working with trees
#
# -----------------------------------------------------------------------------
# Copyright 1993 Allan Brighton.
# 
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted,
# provided that the above copyright notice appear in all copies.  Allan
# Brighton make no representations about the suitability of this software
# for any purpose.  It is provided "as is" without express or implied
# warranty.
# -----------------------------------------------------------------------------


# create a tree with the given callback actions
# text_action is evaluated when a text node is double-clicked and
# bitmap_action when a bitmap is double-clicked

proc tree_create {tree {text_action ""} {bitmap_action}} {

    global tree_private
    tree $tree

    set tree_private($tree,text_action) $text_action
    set tree_private($tree,bitmap_action) $bitmap_action

    tree_set_default_bindings $tree

    return $tree
}
    
    
# set the bindings for the tree canvas
    
proc tree_set_default_bindings {tree} {
    global tree_private
    set canvas [$tree canvas]
    $canvas bind text <1> "focus %W; tree_selectNode $tree"
    $canvas bind bitmap <1> "tree_selectBitmap $tree"

    $canvas bind text <Double-Button-1> "after 1 $tree_private($tree,text_action)"
    $canvas bind bitmap <Double-Button-1> "$tree_private($tree,bitmap_action)"
       
    bind $canvas <ButtonPress-2> "$canvas scan mark %x %y"
    bind $canvas <B2-Motion> "$canvas scan dragto %x %y"
}
    
    
# make sure the tree is visible in the canvas window by scrolling to center
# the tree in the view

proc tree_center {tree} {
    set canvas [$tree canvas]
    set scrollregion [utilGetConfigValue $canvas -scrollregion]
    scan $scrollregion "%d %d %d %d" x1 y1 x2 y2
	
    if {[utilGetConfigValue $tree -layout] == "horizontal"} {
        set scrollincrement [utilGetConfigValue $canvas -yscrollincrement]
	set height [utilGetConfigValue $canvas -height]
	$canvas yview scroll [expr "(($y2 - $y1)/2 - $height/2)/($scrollincrement + 1)"] units
	$canvas xview scroll 0 units
    } else {
        set scrollincrement [utilGetConfigValue $canvas -xscrollincrement]
	set width [utilGetConfigValue $canvas -width]
	$canvas xview scroll [expr "(($x2 - $x1)/2 - $width/2)/($scrollincrement + 1)"] units
	$canvas yview scroll 0 units
    }
}
    
    
# layout the components of the given node depending on whether
# the tree is vertical or horizontal
    
proc tree_layoutNode {tree node} {
    set canvas [$tree canvas]
    set label $node:text
    set bitmap $node:bitmap
	
    if {[utilGetConfigValue $tree -layout] == "horizontal"} {
	scan [$canvas bbox $label] "%d %d %d %d" x1 y1 x2 y2
	$canvas itemconfig $bitmap -anchor se
	$canvas coords $bitmap $x1 $y2
    } else {
	scan [$canvas bbox $bitmap] "%d %d %d %d" x1 y1 x2 y2
	$canvas itemconfig $label -anchor n
	$canvas coords $label [expr "$x1+($x2-$x1)/2"] $y2
    }
}
    
    
# add the given node to the tree under the given parent node 
# (if not already there)  using the given bitmap and
# the given label, if provided.
    
proc tree_addNode {tree parent node bitmap {label ""}} {
    set canvas [$tree canvas]

    if {"$label" == ""} {
	set label $node
    }

    # don't add the node if its already there
    if [llength [$canvas find withtag $node]] {
	return
    }
	
    $canvas create bitmap 0 0 -bitmap "@$bitmap" \
	-tags [list $node bitmap $node:bitmap]
		
    $canvas create text 0 0 \
	-text "$label" \
	-tags [list $node text $node:text]
		
    set line [$canvas create line 0 0 0 0 \
		  -tag "line" \
		  -width 1 \
		  -capstyle round \
		  -fill grey]
		
    tree_layoutNode $tree $node
    $tree addlink $parent $node $line -border 2
}
    
    
# select the current node's label
    
proc tree_selectNode {tree} {
    set canvas [$tree canvas]
    $canvas select from current 0
    $canvas select to current \
	[string length [lindex [$canvas itemconf current -text] 4]]
    tree_deSelectBitmap $tree
    set current [tree_getCurrent $tree]
}
   
    
# de-select all node labels
    
proc tree_deselectNode {tree} {
    set canvas [$tree canvas]
    $canvas select clear
}
    

# highlight the node's bitmap
    
proc tree_selectBitmap {tree} {
    set canvas [$tree canvas]
    set current [lindex [$canvas gettags current] 0]
    tree_deselectNode $tree
    tree_deSelectBitmap $tree
    $canvas itemconfig current \
	-background black \
	-tags "[$canvas gettags current] selected" 
}


# stop highlighting the node's bitmap
    
proc tree_deSelectBitmap {tree} {
    set canvas [$tree canvas]
    $canvas itemconfig selected \
	-background [utilGetConfigValue $canvas -background]
    $canvas dtag selected
}
   

# return the text of the item currently selected
    
proc tree_getCurrent {tree} {
    set canvas [$tree canvas]
    set id [$canvas select item]
    if {"$id" == ""} {
	return [lindex [$canvas gettags selected] 0]
    }
    return [lindex [$canvas gettags $id] 0]
}


# Toggle the layout of the tree between vertical and horizontal

proc tree_toggleLayout {tree} {
    set canvas [$tree canvas]
    if {[utilGetConfigValue $tree -layout] == "horizontal"} {
        $tree config -layout vertical
    } else {
        $tree config -layout horizontal
    }
    
    # change the layout of the nodes so that the bitmap is on top for
    # vertical trees and at left for horizontal trees
    foreach i [$canvas find withtag text] {
        set node [lindex [$canvas gettags $i] 0]
        tree_layoutNode $tree $node
        $tree nodeconfig $node
    }
    
    $tree draw
    tree_center $tree
}


# return the value of the given tk option for the given widget

proc utilGetConfigValue {w option} {
    return [lindex [$w config $option] 4]
}

