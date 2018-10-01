
# ========================================================================
# Minibase 
# (c) Copyright R. Ramakrishnan, 
# University of Wisconsin at Madison.
# (1995-1996) All Rights Reserved.
# Version 0.0
#
# See 'COPYRIGHT' file for the full disclaimer and user agreement.
# ========================================================================

# -----------------------------------------------------------------------------


# Create the main window and initialize the tree to 
# the given dir.
#
# If width and height are specified, they are used for the
# size of the canvas window.

set opt_src_path [lindex $argv 0]

source "$opt_src_path/util.tcl"
source "$opt_src_path/tree.tcl"
source "$opt_src_path/canvas.tcl"
source "$opt_src_path/general2.tcl"
source "$opt_src_path/fileselect.tcl"
source "$opt_src_path/catalog.tcl"
source "$opt_src_path/entry.tcl"
source "$opt_src_path/cost.tcl"

# Should we print some debugging statements
set debug 0

# Hack - are we in the SQL menu already
set in_sql_menu 0

# -----------
# Set some constants used for talking to the minibase main menu
set mmRet            0
set mmExit           3

set mmEnterSQL       1
set mmOpenCatalog    2
set mmToggleJoin     3
set mmListJoins      4

  # These are junk, not in the new interface
set mmCreateDB       9 
set mmListDBs        9
set mmOpenDB         9

set mmListRels       5
set mmListAccess     6

   # Wrong / Non-existent
set mmDumpTuples    12
   # Wrong / Non-existent
set mmInsertTuples  13

set mmFlipPlans      7
set mmOutputCatalog  8

set mmUtils         10

# ----------
# The utilities menu
set umExit           0

set umCreateRel      1
set umBuildIndex     2
set umDropIndex      3
set umDeleteRel      4
set umAddTuple       5
set umDeleteTuple    6
set umViewRel        7
set umListIndexes    8
set umRunStats       9

# ----------
# The SQL menu
set sqlClose          9

set sqlExecute        1
set sqlPruning        2
set sqlToggleAJM      3
set sqlListJM         4
set sqlToggleAM       5
set sqlListDB         6
set sqlRecomputePlan  8

set sqlListRels      10
set sqlListAM        11
set sqlFlipPlans     14
set sqlOutputCatalog 15
# -----------

###########################################################
# return the value of the environment variable of the empty
# string if its not defined

proc utilGetEnv {name} {
    global env
    if {[lsearch [array names env] $name] != -1} {
        return $env($name)
    }
    return ""
}

###########################################################
proc minibaseMakeWindow {} {
    set frame .frame
    frame $frame
    set canvas $frame.canvas
    canvas_create $frame $canvas
    set table $canvas.table
    tree $table
    minibaseToggleLayout $canvas $table
    set menubar .menubar
    set joinmethods .joinmethods
    set query .query
    set database .database
    set catalog .catalog
    set buttonStrip .strip

    wm geometry . 1000x600
    wm minsize . 100 100
    
    minibaseMakeMenuBar $canvas $table $menubar
    minibaseMakeQueryMessage $query
    minibaseMakeButtonStrip $canvas $table $buttonStrip
  #  minibaseMakeJoinMethodsList $joinmethods
    pack $frame -side top -expand yes -fill both 
}

proc minibaseCreateCat {f} {
    set x [pwd]
    set ts "$x/$f"
    .query.status configure -text "Creating $ts"
    update
    catalog.new.catalog CATALOG
    catalog.write CATALOG "$f"
    .query.status configure -text "Catalog $f created. Ready"
    update
}

proc minibaseDeleteCat {f} {
    set x [pwd]
    set ts "$x/$f"
    set ret [tk_dialog .confirm "Dialog" "Confirm deletion of $f" "" "" "Yes" "Cancel"]
    if {$ret == 0} {
        .query.status configure -text "Deleting $f";
        update;
        exec "/bin/rm" -f $f;
        .query.status configure -text "Catalog $f deleted. Ready";
        update
    } {
        .query.status configure -text "Catalog $f not deleted. Ready";
        update
    }
}

proc minibaseEditCat {f} {
    set x [pwd]
    set ts "$x/$f"
    .query.status configure -text "Opening $ts"
    update
    catalog.edit $f
    .query.status configure -text "Ready"
    update
}

proc minibaseOpenCat {f} {
    global relmenu
    global mmOpenCatalog

    set x [pwd]
    set ts "$x/$f"
    .query.status configure -text "Opening $ts"
    update
    # minibaseGoUpMenu 1
    minibaseSendCommand $mmOpenCatalog
    minibaseSendCommand $ts
    .query.status configure -text "Catalog $f opened. Ready"
    .strip.curr configure -text "Current Catalog: $f"
    minibaseActivateMenus
    minibaseUpdateOptRelMenu $relmenu
    update
}

proc minibaseCreateDB {f} {
    global mmCreateDB

    set x [pwd]
    set ts "$x/$f"
    .query.status configure -text "Creating $ts"
    update
    # minibaseGoUpMenu 1
    minibaseSendCommand $mmCreateDB
    minibaseSendCommand $ts
    minibaseSkipTo ".:"
    .query.status configure -text "Database $f created. Ready"
    update
}

proc minibaseDeleteDB {f} {
    set x [pwd]
    set ts "$x/$f"
    set ret [tk_dialog .confirm "Dialog" "Confirm deletion of $f" "" "" "Yes" "Cancel"]
    if {$ret == 0} {
        .query.status configure -text "Deleting $f";
        update;
        exec "/bin/rm" -f $f;
        .query.status configure -text "Database $f deleted. Ready";
        update
    } {
        .query.status configure -text "Database $f not deleted. Ready";
        update
    }
}

proc minibaseOpenDB {f} {
    global relmenu
    global mmOpenDB 

    set x [pwd]
    set ts "$x/$f"
    .query.status configure -text "Opening $ts"
    update
    # minibaseGoUpMenu 1
    minibaseSendCommand $mmOpenDB
    minibaseSendCommand $ts
    minibaseSkipTo ".:"
    .query.status configure -text "Database $f opened. Ready"
    .strip.curr configure -text "Current Database: $f"
    minibaseActivateMenus
    minibaseUpdateDbRelMenu $relmenu
    update
}

proc minibaseCreateDBBox {} {
    set w .cdb
    catch "destroy $w"
    toplevel $w -borderwidth 1 -class Dialog
    wm title $w "Create DB"
    tkwait visibility $w
    frame $w.top -relief raised -bd 1
    pack $w.top -side top -fill both
    frame $w.bot -relief raised -bd 1
    pack $w.bot -side bottom -fill both

    label $w.top.label -text "New Database Name:"
    entry $w.top.entry -width 20 -relief sunken -bd 2 -textvariable name
    pack $w.top.label $w.top.entry -side left -padx 1m -pady 2m

    # set f [mkFrame $w.buttons {fillx bottom} -relief raised -borderw 1]
    button $w.bot.create -text " Create " -command {minibaseCreateDB $name}
    pack $w.bot.create -side right -padx 1m -pady 2m

    bind $w <Return> "$w.bot.create flash"

    # mkButton $w.create " Create " "minibaseCreateDB $name" {bottom padx 7 pady 7}
}

# make the tree window's menu bar
#
# Args:
#
# canvas -  the canvas containing the tree
# tree   -  the tree widget
# menubar - the name to use for the menubar frame

set indexOnly 1

proc minibaseMakeMenuBar {canvas tree menubar} {
    global minibase
    global mrf
    global indexOnly
    global mmExit

    frame $menubar -relief raised -bd 2
    pack $menubar -side top -fill x -ipady 1m
   
    # menu items
    
    # program menu
    set mb [menubutton $menubar.program -text "Program" -menu $menubar.program.m]
    set m  [menu $mb.m]
    pack $mb -side left -padx 1m -ipadx 1m
    $m add command -label "Graceful Exit" -command {
        minibaseGoUpMenu 1
        minibaseSendCommand $mmExit
        destroy .
    }
    $m add command -label "Force Exit" -command {
        # minibaseGoUpMenu 1
        # minibaseSendCommand $mmExit
        destroy .
    }

    # catalog menu
    set mb  [menubutton $menubar.catalog -text "Catalogs" -menu $menubar.catalog.m]
    set m [menu $mb.m]
    pack $mb -side left -padx 1m -ipadx 1m
    $m add command -label "Create" -command "fileselect minibaseCreateCat"
    $m add command -label "Delete" -command "fileselect minibaseDeleteCat"
    $m add command -label "Edit" -command "fileselect minibaseEditCat"
    $m add command -label "Open" -command "fileselect minibaseOpenCat"

    # database menu
#set mb [menubutton $menubar.database -text "Database" -menu $menubar.database.m]
#   set m [menu $mb.m]
#   pack $mb -side left -padx 1m -ipadx 1m
#   $m add command -label "Create" -command "fileselect minibaseCreateDB"
#   $m add command -label "Delete" -command "fileselect minibaseDeleteDB"
#   $m add command -label "Open" -command "fileselect minibaseOpenDB"

    # view menu
    # now replaced by flip button
#    menubutton $menubar.view -text "View" -menu $menubar.view.m
#    menu $menubar.view.m -post "minibaseUpdateViewMenu $canvas $tree $menubar.view.m"
#    pack $menubar.view -side left -padx 1m -ipadx 1m
#    $menubar.view.m add command -label "Layout" \
#        -command "minibaseToggleLayout $canvas $tree"
#

    # join methods
    menubutton $menubar.join -text "Switches" -menu $menubar.join.m -state disabled
    menu $menubar.join.m 
    pack $menubar.join -side left -padx 1m -ipadx 1m
    $menubar.join.m add cascade -label "Toggle Joins" -menu $menubar.join.m.toggle
    $menubar.join.m add cascade -label "Join Descriptions" -menu $menubar.join.m.desc
    $menubar.join.m add checkbutton -label "Index-Only Plans Allowed" \
        -variable indexOnly -command "minibaseIndexOnlyPlans"

    menu $menubar.join.m.toggle
    menu $menubar.join.m.desc

#    menubutton $menubar.joind -text "Join Descriptions" -menu $menubar.joind.m
#    menu $menubar.joind.m
#    pack $menubar.joind -side left -padx 1m -ipadx 1m

    minibaseMakeJoinMethodsMenu $menubar.join.m.toggle $menubar.join.m.desc

    # Query menu
    set sql [menubutton $menubar.sql -text "Query" -menu $menubar.sql.m -state disabled]
    set sqlm [menu $sql.m]
    pack $sql -side left -padx 1m -ipadx 1m
    $sqlm add command -label "Enter QBE" -state disabled
    $sqlm add command -label "Enter SQL" -command "minibaseSQL"
    $sqlm add command -label "Help on Cost Formulas" -command "minibaseHelpCost"

    # relations menu
    set rel [menubutton $menubar.rel -text "Relations" -menu $menubar.rel.m -state disabled]
    global relmenu
    set relmenu [menu $rel.m]
    pack $rel -side left -padx 1m -ipadx 1m
    # minibaseUpdateRelMenu $relmenu
}

proc minibaseActivateMenus {} {
    #
    # activate the "Switches", "SQL", and "Relations" menus
    # when moving out of the "Catalog Editor" mode
    #

    .menubar.join configure -state normal
    .menubar.sql  configure -state normal
    .menubar.rel  configure -state normal
}

# switch IndexOnlyPlans on or off.

proc minibaseIndexOnlyPlans {} {
    global mmRet mmFlipPlans
    minibaseSendCommand $mmFlipPlans
    global mrf
    minibaseSendCommand $mmRet
    gets $mrf line
    if { $line == "1" } {
        minibaseDoRecomputation
    }

}

# The procedure below inserts text into a given text widget and
# applies one or more tags to that text.  The arguments are:
#
# w             Window in which to insert
# text          Text to insert (it's inserted at the "insert" mark)
# args          One or more tags to apply to text.  If this is empty
#               then all tags are removed from the text.

proc insertWithTags {w text args} {
    set start [$w index insert]
    $w insert insert $text
    foreach tag [$w tag names $start] {
        $w tag remove $tag $start insert
    }
    foreach i $args {
        $w tag add $i $start insert
    }
}

proc minibaseMakeJoinMethodsMenu {jm jmd} {
    global mmListJoins
    minibaseSendCommand $mmListJoins
    global costDescArray
    global joinset
    set desc [minibaseScanTo ".:"]
    while { [ string first "end -" $desc] != 0} {
        scan $desc "%d" num
        set costDescArray($num) [minibaseScanTo ".:"]
        set costDescArray($num) [string trimright $costDescArray($num) .:]
        set d [string trimright $desc .:]
        if { [string first "(Inactivated)" $d] == -1 } {
            set joinset($num) 1
        } else {
            regsub ".Inactivated." $d "" d
            set joinset($num) 0
        }


        $jm add checkbutton -label $d -variable joinset($num) -command "minibaseToggleJoinMethod $num"
        $jmd add command -label $d -command "minibaseShowCost $num"
        set desc [minibaseScanTo ".:"]
    }
}


proc minibaseShowCost {joinNum} {
    global costDescArray
    minibaseMessageBox "Cost" $costDescArray($joinNum)
}


proc minibaseToggleJoinMethod {num} {
#    set desc [$t get join:$num.first join:$num.last]
#    if { [string first "(Inactivated)" $desc] == -1 } {
#        $t insert join:$num.last " (Inactivated)"
#    } else {
#        $t delete join:$num.last-14c join:$num.last
#    }
#    minibaseGoUpMenu 2
    global mmRet mmToggleJoin

    minibaseSendCommand $mmToggleJoin
    minibaseSendCommand $num
    minibaseSendCommand $mmRet
    global mrf
    minibaseSendCommand $mmRet
    gets $mrf line
    if { $line == "1" } {
        minibaseDoRecomputation
    }
}

proc minibaseGetMenuLevel {} {
#   global mrf
    global mmRet
#   minibaseSendCommand $mmRet
#   gets $mrf line
    return "0"
}

proc minibaseMakeQueryMessage {query} {
    frame $query -relief raised -bd 2
    pack $query -side top -fill x -ipady 1m

    set qi [message $query.info -aspect 1500 -text "Current Query:"]
   
    set m [message $query.m -aspect 1500]
    pack $qi $m -side left -padx 1m 

    set s [message $query.status -foreground #f00 -bd 2 -aspect 1500 -text "Ready"]
    pack $s -side right -padx 20m    
}


proc minibaseMakeButtonStrip {canvas tree strip} {
    global minibase
    global mrf
    global relmenu

    set f [mkFrame $strip {fillx bottom} -relief raised -borderw 1]

    mkLabel $strip.curr "Catalog Editor Mode" {left padx 8}

    mkButton $f.help  " Help " " minibaseMakeHelpWindow {\
\tUsing the Minibase graphical user interface.


Using the left and right mouse buttons.  

1) Single clicking the left mouse button selects items such as buttons\
on the screen.

2) Double clicking the left mouse button on certain items (such as plan\
nodes) provides more information on that item.

3) Double clicking the right mouse button on certain items initiates an\
action on the item (such as toggling active access methods).

The above information is enough to get started for learning by\
experimentation.  More detailed instructions follow.



This program processes SQL queries.  To enter an SQL query click on the\
button labeled SQL in the lower left hand corner of the main window.  This\
will open a window labeled SQL with an area for entering SQL text.  After\
entering a query click on the button labeled Produce Plans.  If the query\
entered is valid SQL for the current database, information will be displayed\
in the previously empty area of the main window.  

This information is a table\
consisting of partial and complete plans considered by the optimizer for\
executing the given SQL query.  The first rows at the bottom of the table\
represent the available access methods for each of the relations involved\
in the query.  Double clicking on these nodes with the right mouse button\
will toggle the node between being grayed and normal.  When the node is\
grayed it is not considered during optimization.  The nodes above these\
represent query execution plans.  The top row contains nodes that\
represent plans for executing the entire SQL query.  Double\
clicking with the right mouse button on these nodes will cause a plan\
tree to be displayed for which the selected node is the root.  Double\
clicking with the right button on the leftmost node for one of these\
rows will cause pruning for that level to be turned off and on.  Double\
clicking with the left mouse button on any of the nodes (except the\
leftmost ones) will display more information about the node.


You can choose to deactivate (or reactivate) a join method by picking\
it from the joins menu. When a join method\
is inactivated it is not used in any plans that the optimizer\
produces.  You can get a description of the cost estimating function that the\
method uses by selecting it from the join description menu.

The optimizer will only select an index on a clause as a matching primary\
clause if it is a single conjunt by itself, not as part of an or. }" {right padx 7 pady 7}
    mkButton $f.close  " Close " {
        global mmExit
        minibaseGoUpMenu 1
        minibaseSendCommand $mmExit
        destroy . } {right padx 7 pady 7}

    mkButton $f.flip  " Flip Layout " {
        minibaseToggleLayout .frame.canvas .frame.canvas.table
        } {right padx 7 pady 7}

#        set sql [menubutton $f.sql -text "SQL" -menu $f.sql.m]
#        set sqlm [menu $f.sql.m]
#        pack $sql -side left -padx 1m -ipadx 1m
#        
#        $sqlm add command -label " Enter Query " -command "minibaseSQL"
#        $sqlm add command -label " Execute Query " -command "minibaseExecutePlan"

# no db menu for now
#        set db [menubutton $f.db -text "DB" -menu $f.db.m]
#        set dbm [menu $f.db.m]
#        pack $db -side left -padx 1m -ipadx 1m

#        minibaseUpdateDBMenu $dbm


#        $dbm add command -label "Open" -command "fileselect minibaseOpenDB"
#       $dbm add command -label "Create" -command "minibaseCreateDBBox"

#        set rel [menubutton $f.rel -text "Relations" -menu $f.rel.m]
#        set relmenu [menu $f.rel.m]
#        pack $rel -side left -padx 1m -ipadx 1m
#       minibaseUpdateRelMenu $relmenu


#        set cat [menubutton $f.cat -text "Catalogs" -menu $f.cat.m]
#        set catm [menu $f.cat.m]
#        pack $cat -side left -padx 1m -ipadx 1m

#    mkButton $f.sql  " SQL " "minibaseSQL" {left padx 7 pady 7}
#    mkButton $f.exec " Execute " "minibaseExecutePlan " {left padx 7 pady 7}
}


# This procedure is called each time the View menu is displayed
# to update the menu to reflect the current selection
# (This must be kept up to date with the above procedure that creates the menu)

proc minibaseUpdateViewMenu {canvas tree menu} {
    if {[utilGetConfigValue $tree -layout] == "horizontal"} {
        $menu entryconfig 5 -label "Vertical Layout"
    } else {
        $menu entryconfig 5 -label "Horizontal Layout"
    }
}


proc minibaseUpdateOptRelMenu {menu} {
    global mmListRels
    destroy $menu
    menu $menu
    minibaseSendCommand $mmListRels
    set desc [minibaseScanTo ".:"]
    set m 0;
    while { [ string first "end -" $desc] != 0} {
        set d [string trimright $desc .:]
        $menu add cascade -label $d -menu $menu.m$m
        menu $menu.m$m
        set reln($m) $d
        $menu.m$m add cascade -label "List Indexes" -menu $menu.m$m.index
        incr m
        set desc [minibaseScanTo ".:"]
    }

    for {set i 0} {$i < $m} {incr i} {
        minibaseUpdateOptIndexMenu $menu.m$i.index $reln($i)
    }
}

proc minibaseUpdateDbRelMenu {menu} {
    global mmListRels
    destroy $menu
    menu $menu
    minibaseSendCommand $mmListRels
    set desc [minibaseScanTo ".:"]
    set m 0;
    while { [ string first "end -" $desc] != 0} {
        set d [string trimright $desc .:]
        $menu add cascade -label $d -menu $menu.m$m
        menu $menu.m$m
        set reln($m) $d
        $menu.m$m add cascade -label "List Indexes" -menu $menu.m$m.index
        $menu.m$m add command -label "View Tuples" -command "minibaseViewTuples $d"
        $menu.m$m add command -label "Insert Tuples" -command "minibaseInsertTuples $d"
        $menu.m$m add command -label "Load Tuples" -command "fileselect {{minibaseLoadTuples $d}}"
        $menu.m$m add command -label "Delete Tuples" -command "minibaseDeleteTuples $d"
        $menu.m$m add command -label "Destroy Relation" -command "minibaseDestroyReln $d"
        incr m
        set desc [minibaseScanTo ".:"]
    }
    $menu add separator
    $menu add command -label "Create Relation" \
        -command "minibaseCreateReln"

    for {set i 0} {$i < $m} {incr i} {
        minibaseUpdateDbIndexMenu $menu.m$i.index $reln($i)
    }
}

set mcr_reln  ""
set mcr_nattr ""

set mcr_attr(0)  ""
set mcr_type(0)  ""
set mcr_nch(0)   ""

proc minibaseCreateReln {} {
    set w .mcr
    catch {destroy $w}

    toplevel $w
    label    $w.l
    frame    $w.f1
    label    $w.f1.l
    entry    $w.f1.e
    frame    $w.f2
    label    $w.f2.l
    entry    $w.f2.e
    frame    $w.f3
    button   $w.f3.b1
    button   $w.f3.b2

    global mcr_reln mcr_nattr
    set mcr_reln  ""
    set mcr_nattr ""

    $w.l configure -text "Create Relation"
    $w.f1.l configure -text "Name of relation"
    $w.f1.e configure -textvariable mcr_reln
    $w.f2.l configure -text "Number of attributes"
    $w.f2.e configure -textvariable mcr_nattr
    $w.f3.b1 configure -text "Done" -command {
        catch {destroy .mcr}
        minibaseCreateReln1
    }
    $w.f3.b2 configure -text "Cancel" -command {
        catch {destroy .mcr}
    }

    pack append $w $w.l {top expand fillx pady 4} \
        $w.f1 {top expand fillx} $w.f2 {top expand fillx} $w.f3 {top expand fillx}
    pack append $w.f1 $w.f1.l {left} $w.f1.e {left expand fillx}
    pack append $w.f2 $w.f2.l {left} $w.f2.e {left expand fillx}
    pack append $w.f3 $w.f3.b1 {left expand} $w.f3.b2 {left expand}
    tkwait window .mcr
}

proc minibaseCreateReln1 {} {
    set w .mcr1
    catch {destroy $w}

    toplevel $w
    label    $w.l
    pack append $w $w.l {top expand fillx pady 4}

    frame    $w.fl
    button   $w.fl.b1
    button   $w.fl.b2

    global mcr_reln mcr_nattr
    global mcr_attr mcr_type mcr_nch

    set i 0
    while {$i < $mcr_nattr} {
        frame $w.f$i
        label $w.f$i.l1
        entry $w.f$i.e1
        label $w.f$i.l2
        entry $w.f$i.e2
        label $w.f$i.l3
        entry $w.f$i.e3

        set mcr_attr($i) ""
        set mcr_type($i) ""
        set mcr_nch($i) ""

        $w.f$i.l1 configure -text " Attribute $i:    Name"
        $w.f$i.e1 configure -textvariable mcr_attr($i)
        $w.f$i.l2 configure -text "    Type (s/i/f)"
        $w.f$i.e2 configure -textvariable mcr_type($i)
        $w.f$i.l3 configure -text "    Size (only for s)"
        $w.f$i.e3 configure -textvariable mcr_nch($i)

        pack append $w $w.f$i {top expand fillx}
        pack append $w.f$i \
            $w.f$i.l1 {left} $w.f$i.e1 {left expand fillx} \
            $w.f$i.l2 {left} $w.f$i.e2 {left expand fillx} \
            $w.f$i.l3 {left} $w.f$i.e3 {left expand fillx}

        incr i
    }

    $w.l configure -text "Enter data for New Relation: $mcr_reln"
    $w.fl.b1 configure -text "Done" -command {
        global mcr_reln mcr_nattr mcr_attr mcr_type mcr_nch
        set ret "1\n$mcr_reln\n$mcr_nattr"
        set i 0
        while {$i < $mcr_nattr} {
            append ret "\n$mcr_attr($i)"
            append ret "\n$mcr_type($i)"
            if {$mcr_type($i) == "s"} {
                append ret "\n$mcr_nch($i)"
            }
            incr i
        }
        global mmRet mmUtils
        # minibaseGoUpMenu 1
        minibaseSendCommand $mmUtils
        minibaseSendCommand $ret
        minibaseSendCommand $mmRet
        global relmenu
        minibaseUpdateDbRelMenu $relmenu
        catch {destroy .mcr1}
    }
    $w.fl.b2 configure -text "Cancel" -command {
        catch {destroy .mcr1}
    }

    pack append $w $w.fl {top expand fillx}
    pack append $w.fl $w.fl.b1 {left expand} $w.fl.b2 {left expand}
    tkwait window .mcr1
}

proc minibaseViewTuples {reln} {
    global mmRet mmUtils umViewRel
    # minibaseGoUpMenu 1
    minibaseSendCommand $mmUtils
    minibaseSendCommand "$umViewRel
$reln"
    set result [minibaseScanTo "Enter a choice"]
    regsub "^Enter the relation to view: " $result "" result
    regsub "Enter a choice$" $result "" result
    minibaseResultWindow $result
    minibaseSendCommand $mmRet
}

proc minibaseInsertTuples {reln} {
#
# this function must be modified like the other directed entry functions
# when a routine to list the attributes of a relation is made available
# from the backend
#
#
# till then this is the safe way to go
#

    set tuple [entry.get_entry {
Tuple Entry

This entry box is used to provide information needed to insert tuples \
into a relation.  Provide the values of each attribute of the relation, \
_ONE_ entry per line.  The values should be of the proper type/width \
as applicable.  Syntax errors, excess/fewer fields than necessary, or \
any format errors will cause the GUI to hang.

}]
    if {$tuple != ""} {
    global mmRet mmUtils umAddTuple
        # minibaseGoUpMenu 1
        minibaseSendCommand $mmUtils
        minibaseSendCommand "$umAddTuple
$reln
$tuple"
        minibaseSendCommand $mmRet
    }
}

proc minibaseLoadTuples {reln file} {
    global mmRet mmUtils umAddTuple
    # minibaseGoUpMenu 1
    minibaseSendCommand $mmUtils

#
# WARNING: Command "umAddTuple" below should be changed to whatever
# one does the bulk-loading of tuples from a file
#

    minibaseSendCommand "$umAddTuple
$reln
$file"
    minibaseSendCommand $mmRet
}

set mdt_reln ""
set mdt_ndx  ""
set mdt_val  ""

proc minibaseDeleteTuples {reln} {
    set w .mdt
    catch {destroy $w}

    toplevel $w
    label    $w.l
    frame    $w.f1
    label    $w.f1.l
    entry    $w.f1.e
    frame    $w.f2
    label    $w.f2.l
    entry    $w.f2.e
    frame    $w.f3
    button   $w.f3.b1
    button   $w.f3.b2

    global mdt_reln mdt_ndx mdt_val
    set mdt_reln $reln
    set mdt_ndx  ""
    set mdt_val  ""

    $w.l configure -text "Delete Tuple in Relation: $reln"
    $w.f1.l configure -text {Attribute Index [0,1,..]}
    $w.f1.e configure -textvariable mdt_ndx
    $w.f2.l configure -text "Attribute Value"
    $w.f2.e configure -textvariable mdt_val
    $w.f3.b1 configure -text "Done" -command {
    global mmRet mmUtils umDeleteTuple
        global mdt_reln mdt_ndx mdt_val;
        # minibaseGoUpMenu 1;
        minibaseSendCommand $mmUtils;
        minibaseSendCommand "$umDeleteTuple
$mdt_reln
$mdt_ndx
$mdt_val"
        minibaseSendCommand $mmRet;
        catch {destroy .mdt}
    }
    $w.f3.b2 configure -text "Cancel" -command {
        catch {destroy .mdt}
    }

    pack append $w $w.l {top expand fillx pady 4} \
        $w.f1 {top expand fillx} $w.f2 {top expand fillx} $w.f3 {top expand fillx}
    pack append $w.f1 $w.f1.l {left} $w.f1.e {left expand fillx}
    pack append $w.f2 $w.f2.l {left} $w.f2.e {left expand fillx}
    pack append $w.f3 $w.f3.b1 {left expand} $w.f3.b2 {left expand}

    tkwait window $w
}

proc minibaseDestroyReln {reln} {
    set ret [tk_dialog .confirm "Dialog" "Confirm deletion of $reln" "" "" "Yes" "Cancel"]
    if {$ret == 0} {
    global mmRet mmUtils umDeleteRel
        # minibaseGoUpMenu 1
        minibaseSendCommand $mmUtils
        minibaseSendCommand "$umDeleteRel
$reln"
        minibaseSendCommand $mmRet
        global relmenu
        minibaseUpdateDbRelMenu $relmenu
    }
}

proc minibaseUpdateOptIndexMenu {menu reln} {
    global mmListAccess
    catch "destroy $menu"
    menu $menu
    minibaseSendCommand $mmListAccess
    minibaseSendCommand $reln
    set ind [minibaseScanTo ".:"]
    set m 0
    while { [ string first "end -" $ind] != 0} {
        set d [string trimright $ind .:]
        $menu add command -label $d
        menu $menu.m$m
        incr m
        set ind [minibaseScanTo ".:"]
    }
}

proc minibaseUpdateDbIndexMenu {menu reln} {
    global mmListAccess
    catch "destroy $menu"
    menu $menu
    minibaseSendCommand $mmListAccess
    minibaseSendCommand $reln
    set ind [minibaseScanTo ".:"]
    set m 0
    while { [ string first "end -" $ind] != 0} {
        set d [string trimright $ind .:]
        $menu add cascade -label $d -menu $menu.m$m
        menu $menu.m$m
        $menu.m$m add command -label "Destroy Index" -command "minibaseDeleteIndex $reln \"$d\""
        incr m
        set ind [minibaseScanTo ".:"]
    }
    $menu add separator
    $menu add command -label "Create Index" -command "minibaseCreateIndex $menu $reln"
}

set mci_menu ""
set mci_reln ""
set mci_ndx_name ""
set mci_ndx_type ""

proc minibaseCreateIndex {menu reln} {
    set w .mci
    catch {destroy $w}

    toplevel $w
    label    $w.l
    frame    $w.f1
    label    $w.f1.l
    entry    $w.f1.e
    frame    $w.f2
    label    $w.f2.l
    entry    $w.f2.e
    frame    $w.f3
    button   $w.f3.b1
    button   $w.f3.b2

    global mci_menu mci_reln mci_ndx_name mci_ndx_type
    set mci_menu $menu
    set mci_reln $reln
    set mci_ndx_name ""
    set mci_ndx_type ""

    $w.l configure -text "Create Index for Relation: $reln"
    $w.f1.l configure -text "Attribute to Index"
    $w.f1.e configure -textvariable mci_ndx_name
    $w.f2.l configure -text "Index type (h/b)"
    $w.f2.e configure -textvariable mci_ndx_type
    $w.f3.b1 configure -text "Done" -command {
    global mmRet mmUtils umBuildIndex
        global mci_menu mci_reln mci_ndx_name mci_ndx_type;
        # minibaseGoUpMenu 1;
        minibaseSendCommand $mmUtils;
        minibaseSendCommand "$umBuildIndex
$mci_reln
$mci_ndx_name
$mci_ndx_type"
        minibaseSendCommand $mmRet;
        minibaseUpdateDbIndexMenu $mci_menu $mci_reln;
        catch {destroy .mci}
    }
    $w.f3.b2 configure -text "Cancel" -command {
        catch {destroy .mci}
    }

    pack append $w $w.l {top expand fillx pady 4} \
        $w.f1 {top expand fillx} $w.f2 {top expand fillx} $w.f3 {top expand fillx}
    pack append $w.f1 $w.f1.l {left} $w.f1.e {left expand fillx}
    pack append $w.f2 $w.f2.l {left} $w.f2.e {left expand fillx}
    pack append $w.f3 $w.f3.b1 {left expand} $w.f3.b2 {left expand}

    tkwait window $w
}

proc minibaseDeleteIndex {reln index} {
    minibaseMessageBox "Delete Index" "Not Here Yet - $index on $reln not deleted"
}

proc minibaseUpdateDBMenu {menu} {
    global mmListDBs
    catch "destroy $menu"
    menu $menu
    minibaseSendCommand $mmListDBs
    set desc [minibaseScanTo ".:"]
    set m 0;
    while { [ string first "end -" $desc] != 0} {
        set d [string trimright $desc .:]
        $menu add cascade -label $d -menu $menu.m$m
        menu $menu.m$m

        $menu.m$m add command -label "Open" -command "minibaseOpenDB $d"
        incr m
        set desc [minibaseScanTo ".:"]
    }
    $menu add command -label "Create" -command "minibaseCreateDBBox"
}


#
#  Show a help window
#
proc minibaseMakeHelpWindow {info} {
        set w .help

        catch "destroy $w"
        toplevel $w -borderwidth 1
        wm title $w "Minibase Help"

        set t [mkText $w.t {top} -height 25 -wrap word]
        set f [mkFrame $w.buttons {fillx bottom} -relief raised -borderw 1]
        mkButton $f.close  " Close " "destroy $w" {right padx 7 pady 7}
        $t insert current $info
}

proc minibaseMessageBox {caption message} {
    set w .msg

    catch "destroy $w"
    toplevel $w -borderwidth 1
    wm title $w $caption
    # wm minsize $w 50 20
    tkwait visibility $w
    grab $w

    mkMessage $w.m $message top -aspect 200 -justify left

    set f [mkFrame $w.buttons {fillx bottom} -relief raised -borderw 1]
    mkButton $f.dismiss " Dismiss " "destroy $w" { bottom padx 7 pady 7}
    
}


proc minibaseQBE {} {
    #
    # commands to support the QBE interface need to be added here
    #
}

proc minibaseSQL {} {
    set w .sql

    catch "destroy $w"
    toplevel $w -borderwidth 1
    wm title $w "SQL"
    tkwait visibility $w

    text $w.t
    pack $w.t
    set f [mkFrame $w.buttons {fillx bottom} -relief raised -borderw 1]
    mkButton $f.help  " Help " " minibaseMakeHelpWindow {
Enter your query in this box.  The editing features are currently pretty much non-existant right now, hopefully that will improve.
        }" {right padx 7 pady 7}
    mkButton $f.cancel  " Close " "minibaseCloseSql $w" {right padx 7 pady 7}
    mkButton $f.exec  " Execute " "minibaseExecutePlan $w.t" {right padx 7 pady 7}
    mkButton $f.plans  " Produce Plans " "minibaseProducePlans $w.t" \
            {right padx 7 pady 7}
    # bind $w.t <ButtonPress-2> "minibaseInsertSelection $w.t"
}

proc minibaseHelpCost {} {
    global minibase_cost_help
    minibaseMakeHelpWindow $minibase_cost_help
}


proc minibaseInsertSelection {t} {
#    if {[selection own] != ""} {
        set targets [selection get TARGETS]
        if {$targets != "" && [lsearch $targets STRING] >= 0} {
            $t insert insert [selection get]
        }
#    }
}


proc minibaseSend {msg} {
    global mrf
    puts $mrf $msg
    flush $mrf
}

proc minibaseSendCommand {command} {
    global mrf
    global debug

    minibaseSkipTo ".:"
    puts $mrf $command
    if { $debug == 1 } {
        puts stdout "In minibaseSendCommand: $command"
        flush stdout
    }
    flush $mrf
}


proc minibaseScanTo {s} {
    global mrf
    gets $mrf line
    set result [format "%s" $line]
    while { [string first $s $line] == -1 } {
        gets $mrf line
        set result [format "%s\n%s" $result $line]
    }
    return $result
}


proc minibaseSkipTo {s} {
    global mrf
    global debug

    gets $mrf line
    if { $debug == 1 } {
        puts stdout "GOT: $line"
    }

    while { [string first $s $line] == -1 } {
        gets $mrf line
        if { $debug == 1 } {
            puts stdout "GOT: $line"
        }
    }
}


proc minibaseLoadNode { id } {
    global mrf
    scan $id "_%d" shortId
    return [string trimright [format "ID: %d\n%s" $shortId [minibaseScanTo ".:"]] \n.:]
}


proc minibaseLoadNodes {} {
    global mrf
    gets $mrf line
    global rootNodes
    global nodeArray
    global tableArray
    global amTableIndexes
    if { $amTableIndexes != "first" } {
        unset nodeArray
        unset tableArray
    }

    set amTableIndexes [list]
    while { [string first "start -" $line] == 0 } {
        gets $mrf line
        set row $line
        set tableArray($row) [list]
        set amTableIndexes [concat $row $amTableIndexes]
        set nodeArray($row) [format "0\n%s" $row]
        gets $mrf line
        set node [format "_%s" $line]
        while { [string first "start -" $line] != 0 && \
                [string first "end -" $line] != 0} {
            set tableArray($row) [concat $tableArray($row) $node]
            set nodeArray($node) [minibaseLoadNode $node]
            gets $mrf line
            set node [format "_%s" $line]
        }
                
    }
    gets $mrf line
    while { [string first "start -" $line] == 0 } {
        if { [string first "start - level" $line] == 0 } {
            scan $line "start - level%d" levelnum
            set level [format "Level:%d" $levelnum]
            set tableArray($level) $level
            set amTableIndexes [concat $level $amTableIndexes]
            set nodeArray($level) [format "Level %d above, level %d below" $levelnum [expr $levelnum - 1]]
            
            gets $mrf line
        }
        scan $line "start - row%d" rownum
        gets $mrf line
        set row [format "Plan:%s" $line]
        gets $mrf line
        set tableArray($row) [list]
        set amTableIndexes [concat $row $amTableIndexes]
        set nodeArray($row) [format "%d\n%s\n%s" $rownum $row $line]
        gets $mrf line
        set node [format "_%s" $line]
        while { [string first "start -" $line] != 0 && \
                [string first "end -" $line] != 0} {
            set tableArray($row) [concat $tableArray($row) $node]
            set nodeArray($node) [minibaseLoadNode $node]
            gets $mrf line
            set node [format "_%s" $line]
        }
    }
    gets $mrf line
    set rootNodes [list]
    while { [string first "end -" $line] != 0 } {
        set node [format "_%s" $line]
        set rootNodes [concat $rootNodes $node]
        gets $mrf line
    }    
    
}


proc minibaseShowTable {} {
    set canvas .frame.canvas
    set table $canvas.table
    global amTableIndexes
    global tableArray
    $table prune ""
    foreach row $amTableIndexes {
        minibaseAddNode $canvas $table "" $row
        set prev $row
        foreach node $tableArray($row) {
            minibaseAddNode $canvas $table $prev $node
            set prev $node

        }
    }
    $table draw
    tkwait visibility $canvas
#    tree_center $table
    $canvas bind rect <1> "minibaseSelectNode $canvas $table"
    $canvas bind text <1> "minibaseSelectNode $canvas $table"
    $canvas bind rect <3> "minibaseSelectNode $canvas $table"
    $canvas bind text <3> "minibaseSelectNode $canvas $table"
    $canvas bind rect <Double-Button-1> "minibaseToggleVerbose $canvas $table"
    $canvas bind text <Double-Button-1> "minibaseToggleVerbose $canvas $table"
    $canvas bind rect <Double-Button-3> "minibaseTableButton3Handler $canvas $table"
    $canvas bind text <Double-Button-3> "minibaseTableButton3Handler $canvas $table"

}


proc minibaseToggleVerbose {canvas table} {
    set node [minibaseNodeTag $canvas]
    if { $node == [lindex [$canvas gettags $node:verbose] 0] } {
        $canvas dtag $node:verbose
        $canvas itemconfig $node:text -text [minibaseGetNodeText $canvas $node]
    } else {
        $canvas addtag $node:verbose withtag $node
        $canvas itemconfig $node:text -text [minibaseGetNodeText $canvas $node]
    }
    minibaseLayoutNode $canvas $table $node
    $table nodeconfigure $node
    $table draw
}


proc minibaseTableButton3Handler {canvas table} {
    set node [minibaseNodeTag $canvas]
    global nodeArray
    if {[string first "_" $node] == 0 } {
        if {[minibaseGetChildren $node] == ""} {
            minibaseToggleAM $canvas $table $node
        } else {
            minibaseProducePlan $node
        }
    } else {
        if {[string first "Level:" $node] != 0 } {
            minibaseTogglePruning $canvas $table $node
        }
    }
}



proc minibaseToggleAM {canvas table node} {
    global minibase
    global nodeArray
    global omitAMlist
    global sqlToggleAM

    minibaseSendCommand $sqlToggleAM
    scan $nodeArray($node) "ID:%d" id
    minibaseSendCommand $id

    set ind [lsearch $omitAMlist $node]
    if { $ind == -1 } {
        lappend omitAMlist $node
    } else {
        set omitAMlist [lreplace $omitAMlist $ind $ind]
    }
    minibaseDoRecomputation
}


proc minibaseDoRecomputation {} {
    .query.status configure -text "Please Wait ..."
    update

    global sqlRecomputePlan

    minibaseSendCommand $sqlRecomputePlan

    global rootNodes
    global nodeArray
    global tableArray
    global amTableIndexes
    foreach row $amTableIndexes {
        .frame.canvas.table rmlink $row
    }
    minibaseLoadNodes
    .query.status configure -text "Ready"
    minibaseShowTable
}


proc minibaseTogglePruning {canvas table node} {
    global mrf
    global nodeArray
    global sqlPruning

    scan $nodeArray($node) "%d" row
    if {$row != 0 } {
        minibaseSendCommand $sqlPruning
        minibaseSendCommand $row
        set pruning [string range $nodeArray($node) \
                [expr [string first "pruning: " $nodeArray($node)] + 9] end]
        set temp [string trimright $nodeArray($node) $pruning]
        if { [string first $pruning on] == 0 } {
            set nodeArray($node) [format "%soff" $temp]
        } else {
            set nodeArray($node) [format "%son" $temp]
        }
        $canvas itemconfig $node:text -text $nodeArray($node)
        minibaseLayoutNode $canvas $table $node
        $table nodeconfigure $node
        $table draw

        minibaseDoRecomputation
    }
}


proc minibaseProducePlan {root} {
    set w .plan$root
    catch "destroy $w"
    toplevel $w -borderwidth 1
    wm title $w "Query Plan [string trimleft $root _]"
    wm geometry $w 800x600
    wm minsize $w 200 200
    set f [mkFrame $w.buttons {fillx bottom} -relief raised -borderw 1]
#    mkButton $f.help  " Help " " minibaseMakeHelpWindow {
#This is one possible sample plan.  
#        }" {right padx 7 pady 7}
    mkButton $f.cancel  " Cancel " "destroy $w" {right padx 7 pady 7}
#    mkButton $f.plans  " Execute " "minibaseExecutePlan $root" {right padx 7 pady 7}

    set canvas $w.canvas
    set tree $canvas.tree
    canvas_create $w $canvas
    tree $tree 
#"text_proc $tree" "bitmap_proc $tree"
#    minibaseBuildTree $tree "" $node
#    set dir [pwd]
#    tree_addNode $tree "" $dir bitmaps/dir.xbm [dir_tail $dir]
#    add_files_under_dir $tree $dir
    
    # add root dir
    minibaseAddNode $canvas $tree "" $root
    minibaseAddChildren $canvas $tree $root
    $tree draw
    tkwait visibility $canvas
    tree_center $tree
    $canvas bind rect <1> "minibaseSelectNode $canvas $tree"
    $canvas bind text <1> "minibaseSelectNode $canvas $tree"
    $canvas bind rect <3> "minibaseSelectNode $canvas $tree"
    $canvas bind text <3> "minibaseSelectNode $canvas $tree" 
    $canvas bind rect <Double-Button-1> "minibaseToggleVerbose $canvas $tree"
    $canvas bind text <Double-Button-1> "minibaseToggleVerbose $canvas $tree"
    $canvas bind rect <Double-Button-3> "minibaseToggleChildren $canvas $tree"
    $canvas bind text <Double-Button-3> "minibaseToggleChildren $canvas $tree"

}


#
#  Show a help window
#
proc minibaseResultWindow {info} {
    set w .result
    
    catch "destroy $w"
    toplevel $w -borderwidth 1
    wm title $w "Minibase Query Result"
    
    set t [mkText $w.t {top} -height 25 -wrap word]
    set f [mkFrame $w.buttons {fillx bottom} -relief raised -borderw 1]
    mkButton $f.close  " Close " "destroy $w" {right padx 7 pady 7}
    $t insert current $info
}

proc minibaseCloseSql {w}  {
    global in_sql_menu sqlClose

    if {$in_sql_menu == 1} {
        minibaseSendCommand $sqlClose
    }

    set in_sql_menu 0
    destroy $w
}

proc minibaseExecutePlan {t}  {
    global sqlExecute
    global in_sql_menu sqlClose

    minibaseProducePlans $t
    minibaseSendCommand $sqlExecute
    set result [string trimright [minibaseScanTo ".:"] .:]
    minibaseResultWindow $result

    
    minibaseSendCommand $sqlClose
    set in_sql_menu 0

 # Error (not currently tested for) 
 #  minibaseMessageBox "Error" "You need to enter a new query first."
}


proc minibaseGetChildren { node } {
    global nodeArray
    set result [list]
    set children [string range $nodeArray($node) \
            [expr [string first "Children:" $nodeArray($node)] + 9] end]
    set child ""
    while { $child != "_" && [string length $children] > 0 } {
        set child ""
        scan $children "%d" child
        set child [format "_%s" $child]
        set children [string range $children [string length $child] end]
        if { $child != "_" } {
            set result [concat $result $child]
        }
    }
    return $result
}

# Go Up i times in the menu system
proc minibaseGoUpMenu {i} {
    global mmRet

    while { $i > 0 } {
        minibaseSendCommand $mmRet
        incr i -1
    }
}

proc minibaseProducePlans {t} {
    global mrf
    global queryText
    global mmEnterSQL sqlClose
    global in_sql_menu

    if {$in_sql_menu == 1} {
        minibaseSendCommand $sqlClose
    }

    minibaseSendCommand $mmEnterSQL
    set in_sql_menu 1
    set queryText [$t get 1.0 end]
    append queryText ";"
    set queryText [string range $queryText 0 [string first ";" $queryText]]
 
    .query.status configure -text "Please Wait ..."
    update
    minibaseSendCommand $queryText

    .query.m configure -text $queryText

    set line [minibaseScanTo ".:"]

    set line [string trimright $line ".:"]

    global omitAMlist

    set omitAMlist [list]
    .query.status configure -text "Ready"
    if { [string first "error" $line] == 0 } {
        minibaseMessageBox "Error" [string range $line 8 end]
    } else {
        minibaseLoadNodes
        minibaseShowTable
    }
}




# Erase the tree and make dir the new root 

proc minibaseSetNewRoot {canvas tree dir} {
    # get the normalized pathname
    set path [exec /bin/csh -cf "if ( -d ${dir}/ ) cd $dir; pwd"]
    
    if {"$path" == "" || ![file isdirectory $path]} {return}
    
    $tree prune ""
    minibaseAddNode $canvas $tree "" [utilDirTail $path]
    minibaseAddChildren $canvas $tree $path
    $tree draw
    
    # update the dir entry
    global minibase
    set minibase(direntry) $path
    set minibase(list) ""
    wm iconname . [utilDirTail $dir]
}



# select the current node's parent, child or sibling
# depending on the value of direction (Left, Right, Up or Down)

proc minibaseSelectNext {canvas tree direction} {
    set id [$canvas select item]
    set path [lindex [$canvas gettags $id] 0]
    
    # for vertical trees its different...
    if {[utilGetConfigValue $tree -layout] == "vertical"} {
        case $direction in {
            Left        {set direction Up}
            Right        {set direction Down}
            Up        {set direction Left}
            Down        {set direction Right}
        }
    }
    
    case $direction in {
        Left        {
                    set node [$tree parent $path]
                     if {"$node" == ""} {
                         minibaseToggleParent $canvas $tree
                         set node [$tree parent $path]
                    }
               }
        Right        {
                    set node [$tree child $path]
                    if {"$node" == ""} {
                        minibaseAddChildren $canvas $tree $path
                        set node [$tree child $path]
                        $tree draw
                    }
                }
        Down        {
                    set node [$tree sibling $path]
                    if {"$node" == ""} {
                        set node [$tree child [$tree parent $path]]
                    }
                }
        Up        {
                    set next [$tree child [$tree parent $path]]
                    while {"$next" != ""} {
                        set node $next
                        set next [$tree sibling $next]
                        if {"$next" == "$path"} {
                            break;
                        }
                    }
                }
        default {return}
    }
    if {"$node" != ""} {
        set next [lindex [$canvas find withtag $node] 1]
        $canvas select from $next 0
        $canvas select to $next [string length [lindex [$canvas itemconf $next -text] 4]]
    }
}



# return the pathname (dir) for the item currently selected (bitmap or text)

proc minibaseNodeTag {canvas} {
    set tag [$canvas select item]
    if {"$tag" == ""} {
        return [lindex [$canvas gettags selected] 0]
    }
    return [lindex [$canvas gettags $tag] 0]
}

  

proc minibaseToggleChildren {canvas tree} {
    set node [minibaseNodeTag $canvas]
     if [$tree isleaf $node] {
        minibaseAddChildren $canvas $tree $node
     } else {
        $tree prune $node
    }
    $tree draw
}



# highlight the node

proc minibaseSelectNode {canvas tree} {
    global minibase
    set path [lindex [$canvas gettags current] 0]
    minibaseDeSelectNode $canvas
    $canvas itemconfig $path:rect -fill $minibase(nodeSelectColor) -tags "[$canvas gettags $path:rect] selected" 
}



# stop highlighting the node

proc minibaseDeSelectNode {canvas} {
    global omitAMlist
    global minibase
    set node [lindex [$canvas gettags selected] 0]
    if { [lsearch $omitAMlist $node] > -1 } {
        $canvas itemconfig selected -fill $minibase(omitColor)
    } else {
        $canvas itemconfig selected -fill White
    }
    $canvas dtag selected
}



# If the selection is a directory, make it the new root of the tree
#
# If the node already is the root, the node's parent is added
# to the tree

proc minibaseToggleParent {canvas tree} {
    global minibase
    set path [minibaseGetPath $canvas]
    minibaseDeSelectBitmap $canvas
    if [$tree isroot $path] {
       set dir [file dirname $path]
       if {$dir != $path} {
            minibaseAddNode $canvas $tree "" [utilDirTail $dir]
            set minibase(direntry) $dir
                wm iconname . [utilDirTail $dir]
            set tail [file tail $path]
            foreach i [dir lsdirs $dir] {
                if {$i == $tail} {
                     $tree movelink $path $dir
                } else {
                    minibaseAddNode $canvas $tree $dir/$i "$i"
                }
            }
       }
    } else {
        $tree root $path
         set minibase(direntry) $path
         wm iconname . [utilDirTail $path]
    }
    $tree draw
}


# remove the selected node and its subnodes from the display

proc minibaseHideNode {canvas tree} {
    set path [minibaseGetPath $canvas]
    if {"$path" != "" && ![$tree isroot $path]} {
        $tree rmlink $path
        $tree draw
    }
}


# Toggle the layout of the tree between vertical and horizontal

proc minibaseToggleLayout {canvas tree} {
    if {[utilGetConfigValue $tree -layout] == "horizontal"} {
        $tree config -layout vertical
    } else {
        $tree config -layout horizontal
    }
    
    # change the layout of the nodes so that the bitmap is on top for
    # vertical trees and at left for horizontal trees
    foreach i [$canvas find withtag text] {
        set node [lindex [$canvas gettags $i] 0]
        minibaseLayoutNode $canvas $tree $node
        $tree nodeconfig $node
    }
    
    $tree draw
    tree_center $tree
}


# layout the components of the given node depending on whether
# the tree is vertical or horizontal

proc minibaseLayoutNode {canvas tree node} {
    global nodeArray
    set text $node
    $canvas coords $text:rect 0 0 0 0
    $canvas coords $text:text 0 0
#    set bitmap $node
    scan [$canvas bbox $text] "%d %d %d %d" x1 y1 x2 y2
    set width [expr "$x2-$x1"]
    set height [expr "$y2-$y1"]
    if {[utilGetConfigValue $tree -layout] == "horizontal"} {
        $canvas itemconfig $text:text -anchor e
        $canvas coords $text:text $x1 $y2
        $canvas coords $text:rect [expr "$x1-$width"] [expr "$y1+($height/2)"] \
                                  [expr "$x2-$width+1"] [expr "$y2+($height/2)+1"]
    } else {
        $canvas itemconfig $text:text -anchor n
        $canvas coords $text:text $x1 $y2
        $canvas coords $text:rect [expr "$x1-($width/2)"] [expr "$y1+$height"] \
                                  [expr "$x2-($width/2)"] [expr "$y2+$height"]
    }
}


proc minibaseAddChildren {canvas tree parent} {
    set children [minibaseGetChildren $parent]
    foreach i $children {
          minibaseAddNode $canvas $tree $parent $i
        minibaseAddChildren $canvas $tree $i
    }
}



proc minibaseGetNodeText {canvas node} {
    global nodeArray
    set midstart [string first "\[" $nodeArray($node)]
    if { $midstart != -1 } {
        set midend [string first "\]" $nodeArray($node)]
    } else {
        set midstart 0
        set midend -1
    }
    set start [string range $nodeArray($node) 0 [expr $midstart - 1]]
    set mid [string range $nodeArray($node) [expr $midstart + 1] \
            [expr $midend - 1]]
    set tail [string range $nodeArray($node) [expr $midend + 1] end]
    if { $node == [lindex [$canvas gettags $node:verbose] 0] } {
        return [format "%s%s%s" $start $mid $tail]
    } else {
        return [format "%s%s" $start $tail]
    }
}


# add a node to the tree (if not already there)
#
# Args: 
#        canvas        -        tree's canvas
#        tree        -        the tree
#        parent        -        parent of node
#        node        -        the node

proc minibaseAddNode {canvas tree parent node} {
    global minibase
    global nodeArray
    global omitAMlist
    # don't add the node if its already there
    if [llength [$canvas find withtag $node]] {
        return
    }
    
    set font "-Adobe-Helvetica-Medium-R-Normal--*-100-*"

    set nodebkcolor White
    if { [lsearch $omitAMlist $node] > -1 } {
        set nodebkcolor $minibase(omitColor)
    }
    $canvas create rectangle 0 0 0 0 -fill $nodebkcolor -tags "$node rect $node:rect" -outline $minibase(lineColor)
    $canvas create text 0 0 -text [minibaseGetNodeText $canvas $node] -font $font -tags "$node text $node:text"
    set line [$canvas create line 0 0 0 0 -tag "line" -width 1 -capstyle round -fill $minibase(lineColor)]
    minibaseLayoutNode $canvas $tree $node
    $tree addlink $parent $node $line -border 7
}

# local initialization

proc minibaseInit {} {
    global minibase
    global amTableIndexes

    global mrf
    global indexOnly
    global dbname

    if [file exists $dbname] {
        set mrf [open "|minibase -d $dbname -r -b 50" r+]
    } else {
        set mrf [open "|minibase -d $dbname -s 200 -b 50" r+]
    }

    # these 2 variables are used to keep track of the current open file list and dir
    set minibase(node) ""
    set minibase(list) ""

    set amTableIndexes "first"
    set indexOnly "1"

    # List of files to ignore (should be an option ?)
    set minibase(ignore) {.* *% *~ core *.BAK #* *.o}
}


# parse the command line options
#
# Usage: minibase database-name ?dir? ?option arg option arg?
#

proc minibaseOptions {argv} {
    global minibase dbname
    
    # save options 
    set minibase(argv) "$argv"
    
    # for error message
    set usage {
usage: minibase ?dir? ?options...?
            
Option              Arg Type      Description
-----------------------------------------------------------------------
-nodeselectcolor  Tk color      color of directory bitmaps when selected
-linecolor          Tk color      color of tree lines
-colormodel         keyword       set to "color" or "monochrome"
}
    
    # set initial values
    set minibase(lineColor) ""
    set minibase(nodeSelectColor) ""
    set minibase(omitColor) black
    
    # parse options
    set n [llength $argv]
    for {set i 0} {$i < $n} {incr i} {
        set opt [lindex $argv $i]
        if {"[string index $opt 0]" == "-" && "$opt" != "-lookahead"} {
            set arg [lindex $argv [incr i]]
        }
        case $opt in {
            -nodeselectcolor  {set minibase(nodeSelectColor) $arg}
            -colormodel       {tk colormodel . $arg; puts stderr "color = [tk colormodel .]"}
            -linecolor        {set minibase(lineColor) $arg}
            default           {if [file isdirectory $opt] {
                                  set minibase(cwd) $opt
                                 } else { 
                                   set dbname $opt
                                   #puts stderr "$usage"
                                   #exit 1
                              }}
        }
    }
    
    # set any values that were not explicitly set as options
    if {"$minibase(nodeSelectColor)" == ""} {
        set minibase(nodeSelectColor) lightblue2
    }
    
    if {"$minibase(lineColor)" == ""} {
        set minibase(lineColor) black
    }

    # set some resources
    option add *Listbox*font "-Adobe-Helvetica-Medium-R-Normal--*-100-*"

}


# ---------------------------------------------------------------------------
# main 

set miniroot [utilGetEnv "MINIBASE_ROOT"]
lappend auto_path "$miniroot/programs/opttool"

wm title . "Minibase"
wm geometry . 600x400
option readfile minibase.config

# A default name for the database
set dbname "/tmp/db"

# local initialization
minibaseInit

# Get over the first menu (select "Minibase")
minibaseSendCommand 1


# parse the command line options
minibaseOptions $argv

# create the main window
minibaseMakeWindow 

# Activate the menus (hack - database is already open)
minibaseActivateMenus
minibaseUpdateDbRelMenu $relmenu


