
#
# entry.tcl
#
# a simple, synchronous, entry box.
# all names beginning with `.entry' and `entry.' are reserved.
#
# error checking is minimal since error handling is ill-defined.
#

#################################################################

set entry.entry_value ""

proc entry.get_entry {msg} {
    catch {destroy .entry}

    toplevel .entry
    frame    .entry.f1
    frame    .entry.b
    message  .entry.f1.msg
    text     .entry.f1.text
    button   .entry.b.done
    button   .entry.b.cancel

    wm title .entry "Entry"

    global entry.entry_value
    set entry.entry_value ""

    .entry.f1.msg configure -text "$msg" -width 500

    .entry.b.done configure -text "Done" \
        -command {
            global entry.entry_value
            set entry.entry_value [.entry.f1.text get 1.0 end]
            catch {destroy .entry}
        }

    .entry.b.cancel configure -text "Cancel" \
        -command {
            global entry.entry_value
            set entry.entry_value ""
            catch {destroy .entry}
        }

    pack append .entry                      \
        .entry.f1 {top expand fillx pady 4} \
        .entry.b  {top expand fillx pady 4}

    pack append .entry.f1           \
        .entry.f1.msg  {top  fillx} \
        .entry.f1.text {top expand fillx}

    pack append .entry.b              \
        .entry.b.done   {left expand} \
        .entry.b.cancel {left expand}

    tkwait window .entry
    return ${entry.entry_value}
}

#################################################################
