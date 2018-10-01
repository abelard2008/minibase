
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
# This is the MAIN program.  It expects as arguments in argv the path
# to the buffer-manager-viewer source directory and the directory
# containing the BM trace files.
#

set bm_code_dir  [lindex $argv 0]
set bm_trace_dir [lindex $argv 1]

source $bm_code_dir/source_files/setup.tcl


frame .table

set traceFile [fileselect $dir_w_bm_logs]

OperTime opertime -parent [ toplevel .statistics ]
Actionlist actionlisting -parent [ toplevel .log_file]
Trace $traceFile
