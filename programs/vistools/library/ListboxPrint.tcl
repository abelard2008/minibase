# ListboxPrint.tcl - Print Dialog Box for printing the contents of a listbox
#
# usage: ListboxPrint
#
# Author: Allan Brighton (allan@piano.sta.sub.org) 

itcl_class ListboxPrint {
    inherit PrintDialog


    # create a new instance of this class

    constructor {config} {
	PrintDialog::constructor
    }


    # print the contents of the listbox to the given file descriptor
    
    method print {fd} {
	set n [$listbox size]
	for {set i 0} {$i < $n} {incr i} {
	    puts $fd [$listbox get $i]
	}
    }


    # Public data

    # listbox to print
    public listbox {}
}


