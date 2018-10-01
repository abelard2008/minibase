
# ========================================================================
# Minibase 
# (c) Copyright R. Ramakrishnan, 
# University of Wisconsin at Madison.
# (1995-1996) All Rights Reserved.
# Version 0.0
#
# See 'COPYRIGHT' file for the full disclaimer and user agreement.
# ========================================================================

#
# This is the MAIN program.  It expects as arguments in argv
# the path to the btree viewer source directory and the directory
# containing the btree trace files.
#

set btree_code_dir  [lindex $argv 0]
set btree_trace_dir [lindex $argv 1]

source $btree_code_dir/source_files/setup.tcl

Trace $trace
