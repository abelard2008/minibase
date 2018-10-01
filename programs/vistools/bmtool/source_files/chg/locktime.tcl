###################################################################
#
# Class: OperTime
#
# Description: Below is the definition of the graph showing the
#              the buffer manager statistics. See also the man page...
#
# Author: Harry Stavropoulos (harryst@cs.wisc.edu)
#
###################################################################


itcl_class OperTime {
    constructor { config } {
	set graph [ blt_graph $parent.$this]
	CreateGraphHelp
	pack $graph

	$graph configure -title "Buffer Manager Statistics"
	$graph xaxis configure -title "Number of Operations"
	$graph yaxis configure -title "Ratios"
	
	SetActiveLegend $graph
	SetZoom $graph
	SetPrint $graph
	SetClosestPoint $graph

	Pin_ratio
	Unpin_ratio
	Hit_ratio

	$graph yaxis configure -min 0
	update idletasks
    }


    method Pin_ratio {} {
	$graph element create pin_ratio -bg powderblue -fg navyblue
    }

    method Unpin_ratio {} {
	$graph element create unp_dirty_ratio -bg purple -fg magenta
    }

    method Hit_ratio {} {
	$graph element create hit_ratio -bg green -fg cyan
    }

    method AddPoints { pinned unpin_dirty num_reads num_pins actions num_pages} {

	if { ($num_pages == 0) || ($pinned == 0) || ($num_pins == 0) } {
	   #
	   # takes care of the first times the function is invoked, when
	   # things are still getting initilized
	   #
	   return
        }
	set unpinned [expr $num_pages - $pinned]
	set rat1 [expr $pinned.0 / $num_pages]
	set rat2 [expr $unpin_dirty.0 / $pinned]
	set rat3 [expr $num_reads.0 / $num_pins]
	$graph element append pin_ratio   {$actions  $rat1}
	$graph element append unp_dirty_ratio {$actions  $rat2}
	$graph element append hit_ratio  {$actions  $rat3}
	update idletasks
    }

    method Clear_Graph {} {

	$graph element delete pin_ratio
	$graph element delete unp_dirty_ratio
	$graph element delete hit_ratio

        Pin_ratio
        Unpin_ratio
        Hit_ratio
    }

    protected graph ""
    public parent "."
}
