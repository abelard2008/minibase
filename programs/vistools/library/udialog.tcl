# dialog.tcl - general purpose dialogs
#
# Author: Allan Brighton (allan@piano.sta.sub.org) 


# get a file name from the user and return it
# or the empty string

proc filename_dialog {{dir "."}} {
    if {![winfo exists .fs]} {
	FileSelect .fs -dir $dir
    }
    .fs config -dir $dir
    if {[.fs activate]} {
        return [.fs get]
    }
}


# error message routine with exit button

proc errexit_dialog {msg} {
    set w .errexit_dialog
    catch {$w delete}
    set d [Dialog $w \
	    -title Error \
	    -text "Error: $msg" \
	    -bitmap error \
	    -buttons {Continue Exit}]
    if {[$d activate] == 1} {
	exit 1
    }
}


# error  message routine

proc error_dialog {msg} {
    set w .error_dialog
    catch {$w delete}
    [Dialog $w \
	    -title Error \
	    -text "Error: $msg" \
	    -bitmap error] activate
}


# warning message routine

proc warning_dialog {msg} {
    set w .warning_dialog
    catch {$w delete}
    [Dialog $w \
	    -title Warning \
	    -text "Warning: $msg" \
	    -bitmap warning] activate
}


# info message routine

proc info_dialog {msg} {
    set w .info_dialog
    catch {$w delete}
    [Dialog $w \
	    -title Information \
	    -text "$msg" \
	    -bitmap info] activate
}


# Question the user and get the answer
# If the answer is yes, return 1, otherwise 0

proc confirm_dialog {msg} {
    set w .confirm_dialog
    catch {$w delete}
    set d [Dialog $w \
	    -title Confirm \
	    -text $msg \
	    -bitmap questhead \
	    -default 1 \
	    -buttons {Yes No Cancel}]
    return [expr {[$d activate] == 0}]
}


# Get some text input from the user and return it
# or an empty string (in case the user cancels the operation)

proc input_dialog {msg} {
    set w .input_dialog
    catch {$w delete}
    set d [InputDialog $w \
	    -title Input \
	    -text $msg \
	    -bitmap questhead \
	    -default 0 \
	    -buttons {OK Cancel}]
    return [$d activate]
}


