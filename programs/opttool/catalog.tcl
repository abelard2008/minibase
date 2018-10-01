
#
# catalog.tcl
#
# minibase catalog editor and related utilities.
# all names beginning with `catalog' and `.catalog' are reserved.
#
# if the format of the catalogs changes, the code here should
# be revised to reflect the changes made.
#
# error checking is minimal since error handling is ill-defined.
#

#################################################################

proc catalog.write.method {m file} {
    global $m

    set method [set ${m}(method)]
    set cost   [set ${m}(cost)]
    set order  [set ${m}(order)]
    set which  [set ${m}(which)]

    if {$which == 1} {
        set clus     [set ${m}(clus)]
        set keys     [set ${m}(keys)]
        set pages    [set ${m}(pages)]
        set type     [set ${m}(type)]
        set filename [set ${m}(filename)]
        set rest     [set ${m}(rest)]

        puts $file "        $method $cost $order $which $clus $keys $pages $type $filename $rest"
    } else {
        puts $file "        $method $cost $order $which"
    }
}

proc catalog.write.attribute {a file} {
    global $a

    set attribute  [set ${a}(attribute)]
    set attrid     [set ${a}(attrid)]
    set type       [set ${a}(type)]
    set minval     [set ${a}(minval)]
    set maxval     [set ${a}(maxval)]
    set size       [set ${a}(size)]
    set offset     [set ${a}(offset)]
    set nummethods [llength [set ${a}(methods)]]

    puts $file "    $attribute $attrid $type $minval $maxval $size $offset $nummethods"

    foreach method [set ${a}(methods)] {
        catalog.write.method $a.$method $file
    }
}

proc catalog.write.relation {r file} {
    global $r

    set relation  [set ${r}(relation)]
    set numattrs  [llength [set ${r}(attributes)]]
    set card      [set ${r}(card)]
    set tuplesize [set ${r}(tuplesize)]
    set numpages  [set ${r}(numpages)]

    puts $file "$relation $numattrs $card $tuplesize $numpages"

    foreach attribute [set ${r}(attributes)] {
        catalog.write.attribute $r.$attribute $file
    }
}

proc catalog.write {catalog filename} {
    set status [catch {set file [open $filename "w"]} error_msg]
    if {$status != 0} {
        puts stderr "[catalog.tcl] open($filename,w): $error_msg"
        return
    }

    global $catalog

    puts $file [llength [set $catalog]]
    foreach relation [set $catalog] {
        catalog.write.relation $catalog.$relation $file
    }

    set status [catch {close $file} error_msg]
    if {$status != 0} {
        puts stderr "[catalog.tcl] close($filename): $error_msg"
    }
}

#################################################################

proc catalog.new.method {pfx method} {
    set m $pfx.$method
    global $m

    set ${m}(method) $method
    set ${m}(cost)   0
    set ${m}(order)  "A"
    set ${m}(which)  0
}

proc catalog.read.method {attribute relation catalog file} {
    set line [gets $file]
    scan $line "%s %d %s %d" method cost order which

    if {$which == 1} {
        scan $line "%s %d %s %d %d %d %d %d %s" \
            method cost order which clus keys pages type filename
    }

    set rest ""
    regexp "%.*$" $line rest

    set m $catalog.$relation.$attribute.$method
    global $m

    set ${m}(method) $method
    set ${m}(cost)   $cost
    set ${m}(order)  $order
    set ${m}(which)  $which

    if {$which == 1} {
        set ${m}(clus)     $clus
        set ${m}(keys)     $keys
        set ${m}(pages)    $pages
        set ${m}(type)     $type
        set ${m}(filename) $filename
        set ${m}(rest)     $rest
    }

    return $method
}

proc catalog.new.attribute {pfx attribute} {
    set a $pfx.$attribute
    global $a

    set ${a}(attribute) $attribute
    set ${a}(attrid)    0
    set ${a}(type)      T
    set ${a}(minval)    "A"
    set ${a}(maxval)    "Z"
    set ${a}(size)      0
    set ${a}(offset)    0
    set ${a}(methods)   {}
}

proc catalog.read.attribute {relation catalog file} {
    set line [gets $file]
    scan $line "%s %d %s %s %s %d %d %d" \
        attribute attrid type minval maxval size offset nummethods

    set a $catalog.$relation.$attribute
    global $a

    set ${a}(attribute)  $attribute
    set ${a}(attrid)     $attrid
    set ${a}(type)       $type
    set ${a}(minval)     $minval
    set ${a}(maxval)     $maxval
    set ${a}(size)       $size
    set ${a}(offset)     $offset
    set ${a}(nummethods) $nummethods
    set ${a}(methods)    {}

    while {$nummethods > 0} {
        set method [catalog.read.method $attribute $relation $catalog $file]
        lappend ${a}(methods) $method
        incr nummethods -1
    }

    return $attribute
}

proc catalog.new.relation {pfx relation} {
    set r $pfx.$relation
    global $r

    set ${r}(relation)   $relation
    set ${r}(card)       0
    set ${r}(tuplesize)  0
    set ${r}(numpages)   0
    set ${r}(attributes) {}
}

proc catalog.read.relation {catalog file} {
    set line [gets $file]
    scan $line "%s %d %d %d %d" relation numattrs card tuplesize numpages

    set r $catalog.$relation
    global $r

    set ${r}(relation)   $relation
    set ${r}(numattrs)   $numattrs
    set ${r}(card)       $card
    set ${r}(tuplesize)  $tuplesize
    set ${r}(numpages)   $numpages
    set ${r}(attributes) {}

    while {$numattrs > 0} {
        set attribute [catalog.read.attribute $relation $catalog $file]
        lappend ${r}(attributes) $attribute
        incr numattrs -1
    }

    return $relation
}

proc catalog.new.catalog {catalog} {
    global $catalog
    set $catalog {}
}

proc catalog.read {catalog filename} {
    set status [catch {set file [open $filename "r"]} error_msg]
    if {$status != 0} {
        puts stderr "[catalog.tcl] open($filename,r): $error_msg"
        return
    }

    global $catalog
    set $catalog {}

    set nrel [gets $file]

    while {$nrel > 0} {
        set relation [catalog.read.relation $catalog $file]
        lappend $catalog $relation
        incr nrel -1
    }

    set status [catch {close $file} error_msg]
    if {$status != 0} {
        puts stderr "[catalog.tcl] close($filename): $error_msg"
    }
}

#################################################################

set catalog.current_file      ""
set catalog.current_catalog   ""
set catalog.current_relation  ""
set catalog.current_attribute ""
set catalog.current_method    ""
set catalog.entry_value       ""

proc catalog.help {msg} {
    catch {destroy .catalog_help}

    toplevel  .catalog_help
    frame     .catalog_help.f1
    text      .catalog_help.f1.text
    scrollbar .catalog_help.f1.scroll
    button    .catalog_help.done

    wm title .catalog_help "Catalog Help"

    .catalog_help.f1.text   configure -yscrollcommand ".catalog_help.f1.scroll set"
    .catalog_help.f1.scroll configure -command        ".catalog_help.f1.text yview"
    .catalog_help.done      configure -text "Done" \
        -command {
            catch {destroy .catalog_help}
        }

    .catalog_help.f1.text insert current $msg

    pack append .catalog_help                \
        .catalog_help.f1   {top expand fill} \
        .catalog_help.done {top expand pady 4}

    pack append .catalog_help.f1                   \
        .catalog_help.f1.text   {left expand fill} \
        .catalog_help.f1.scroll {right filly}
}

proc catalog.get_entry {msg} {
    catch {destroy .catalog_entry}

    toplevel .catalog_entry
    frame    .catalog_entry.f1
    frame    .catalog_entry.b
    label    .catalog_entry.f1.label
    entry    .catalog_entry.f1.entry
    button   .catalog_entry.b.done
    button   .catalog_entry.b.cancel

    wm title .catalog_entry "Entry"

    global catalog.entry_value
    set catalog.entry_value ""

    .catalog_entry.f1.label configure -text         "  $msg  "
    .catalog_entry.f1.entry configure -textvariable catalog.entry_value

    .catalog_entry.b.done configure -text "Done" \
        -command {
            catch {destroy .catalog_entry}
        }

    .catalog_entry.b.cancel configure -text "Cancel" \
        -command {
            global catalog.entry_value
            set catalog.entry_value ""
            catch {destroy .catalog_entry}
        }

    pack append .catalog_entry                      \
        .catalog_entry.f1 {top expand fillx pady 4} \
        .catalog_entry.b  {top expand fillx pady 4}

    pack append .catalog_entry.f1             \
        .catalog_entry.f1.label {left  fillx} \
        .catalog_entry.f1.entry {right expand fillx}

    pack append .catalog_entry.b              \
        .catalog_entry.b.done   {left expand} \
        .catalog_entry.b.cancel {left expand}

    tkwait window .catalog_entry
    return ${catalog.entry_value}
}

proc catalog.edit.method {method} {
    global catalog.current_attribute
    global catalog.current_method
    set catalog.current_method "${catalog.current_attribute}.${method}"

    catch {destroy .catalog_medit}

    toplevel  .catalog_medit
    label     .catalog_medit.label
    frame     .catalog_medit.f1
    frame     .catalog_medit.f2
    frame     .catalog_medit.f3
    frame     .catalog_medit.f4
    frame     .catalog_medit.f5
    frame     .catalog_medit.f6
    frame     .catalog_medit.f7
    frame     .catalog_medit.f8
    frame     .catalog_medit.f9
    frame     .catalog_medit.b
    label     .catalog_medit.f1.label
    entry     .catalog_medit.f1.entry
    label     .catalog_medit.f2.label
    entry     .catalog_medit.f2.entry
    label     .catalog_medit.f3.label
    entry     .catalog_medit.f3.entry
    label     .catalog_medit.f4.label
    entry     .catalog_medit.f4.entry
    label     .catalog_medit.f5.label
    entry     .catalog_medit.f5.entry
    label     .catalog_medit.f6.label
    entry     .catalog_medit.f6.entry
    label     .catalog_medit.f7.label
    entry     .catalog_medit.f7.entry
    label     .catalog_medit.f8.label
    entry     .catalog_medit.f8.entry
    label     .catalog_medit.f9.label
    entry     .catalog_medit.f9.entry
    button    .catalog_medit.b.help
    button    .catalog_medit.b.cancel
    button    .catalog_medit.b.done

    wm title .catalog_medit "Method Editor"

    set m ${catalog.current_method}
    global $m

    .catalog_medit.label configure -text "Method: $method"

    .catalog_medit.f1.label configure -text "                Cost "
    .catalog_medit.f2.label configure -text "               Order "
    .catalog_medit.f3.label configure -text "               Which "
    .catalog_medit.f4.label configure -text "          Clustering "
    .catalog_medit.f5.label configure -text "                Keys "
    .catalog_medit.f6.label configure -text "               Pages "
    .catalog_medit.f7.label configure -text "                Type "
    .catalog_medit.f8.label configure -text "            Filename "
    .catalog_medit.f9.label configure -text "Multi-Column Indexes "

    .catalog_medit.f1.entry insert 0 [set ${m}(cost)]
    .catalog_medit.f2.entry insert 0 [set ${m}(order)]
    .catalog_medit.f3.entry insert 0 [set ${m}(which)]

    set which [set ${m}(which)]
    if {$which == 1} {
        .catalog_medit.f4.entry insert 0 [set ${m}(clus)]
        .catalog_medit.f5.entry insert 0 [set ${m}(keys)]
        .catalog_medit.f6.entry insert 0 [set ${m}(pages)]
        .catalog_medit.f7.entry insert 0 [set ${m}(type)]
        .catalog_medit.f8.entry insert 0 [set ${m}(filename)]
        .catalog_medit.f9.entry insert 0 [set ${m}(rest)]
    }

    .catalog_medit.b.help configure -text "Help" \
        -command {
            global catalog.help.method
            catalog.help ${catalog.help.method}
        }

    .catalog_medit.b.cancel configure -text "Cancel" \
        -command {
            catch {destroy .catalog_medit}
        }

    .catalog_medit.b.done configure -text "Done" \
        -command {
            global catalog.current_method
            set m ${catalog.current_method}
            global $m

            set ${m}(cost)  [.catalog_medit.f1.entry get]
            set ${m}(order) [.catalog_medit.f2.entry get]
            set ${m}(which) [.catalog_medit.f3.entry get]

            set which [set ${m}(which)]
            if {$which == 1} {
                set ${m}(clus)     [.catalog_medit.f4.entry get]
                set ${m}(keys)     [.catalog_medit.f5.entry get]
                set ${m}(pages)    [.catalog_medit.f6.entry get]
                set ${m}(type)     [.catalog_medit.f7.entry get]
                set ${m}(filename) [.catalog_medit.f8.entry get]
                set ${m}(rest)     [.catalog_medit.f9.entry get]
            }

            catch {destroy .catalog_medit}
        }

    pack append .catalog_medit                         \
        .catalog_medit.label {top expand fillx pady 4} \
        .catalog_medit.f1    {top expand fillx}        \
        .catalog_medit.f2    {top expand fillx}        \
        .catalog_medit.f3    {top expand fillx}        \
        .catalog_medit.f4    {top expand fillx}        \
        .catalog_medit.f5    {top expand fillx}        \
        .catalog_medit.f6    {top expand fillx}        \
        .catalog_medit.f7    {top expand fillx}        \
        .catalog_medit.f8    {top expand fillx}        \
        .catalog_medit.f9    {top expand fillx}        \
        .catalog_medit.b     {top expand fillx pady 4}

    pack append .catalog_medit.f1              \
        .catalog_medit.f1.label {left  fillx} \
        .catalog_medit.f1.entry {right expand fillx}

    pack append .catalog_medit.f2              \
        .catalog_medit.f2.label {left  fillx} \
        .catalog_medit.f2.entry {right expand fillx}

    pack append .catalog_medit.f3              \
        .catalog_medit.f3.label {left  fillx} \
        .catalog_medit.f3.entry {right expand fillx}

    pack append .catalog_medit.f4              \
        .catalog_medit.f4.label {left  fillx} \
        .catalog_medit.f4.entry {right expand fillx}

    pack append .catalog_medit.f5              \
        .catalog_medit.f5.label {left  fillx} \
        .catalog_medit.f5.entry {right expand fillx}

    pack append .catalog_medit.f6              \
        .catalog_medit.f6.label {left  fillx} \
        .catalog_medit.f6.entry {right expand fillx}

    pack append .catalog_medit.f7              \
        .catalog_medit.f7.label {left  fillx} \
        .catalog_medit.f7.entry {right expand fillx}

    pack append .catalog_medit.f8              \
        .catalog_medit.f8.label {left  fillx} \
        .catalog_medit.f8.entry {right expand fillx}

    pack append .catalog_medit.f9              \
        .catalog_medit.f9.label {left  fillx} \
        .catalog_medit.f9.entry {right expand fillx}

    pack append .catalog_medit.b              \
        .catalog_medit.b.help   {left expand} \
        .catalog_medit.b.cancel {left expand} \
        .catalog_medit.b.done   {left expand}
}

proc catalog.edit.attribute {attribute} {
    global catalog.current_relation
    global catalog.current_attribute
    set catalog.current_attribute "${catalog.current_relation}.${attribute}"

    catch {destroy .catalog_aedit}

    toplevel  .catalog_aedit
    label     .catalog_aedit.label
    frame     .catalog_aedit.f1
    frame     .catalog_aedit.f2
    frame     .catalog_aedit.f3
    frame     .catalog_aedit.f4
    frame     .catalog_aedit.f5
    frame     .catalog_aedit.f6
    frame     .catalog_aedit.f7
    frame     .catalog_aedit.b1
    frame     .catalog_aedit.b2
    label     .catalog_aedit.f1.label
    entry     .catalog_aedit.f1.entry
    label     .catalog_aedit.f2.label
    entry     .catalog_aedit.f2.entry
    label     .catalog_aedit.f3.label
    entry     .catalog_aedit.f3.entry
    label     .catalog_aedit.f4.label
    entry     .catalog_aedit.f4.entry
    label     .catalog_aedit.f5.label
    entry     .catalog_aedit.f5.entry
    label     .catalog_aedit.f6.label
    entry     .catalog_aedit.f6.entry
    listbox   .catalog_aedit.f7.list
    scrollbar .catalog_aedit.f7.scroll
    button    .catalog_aedit.b1.new
    button    .catalog_aedit.b1.edit
    button    .catalog_aedit.b1.delete
    button    .catalog_aedit.b2.help
    button    .catalog_aedit.b2.cancel
    button    .catalog_aedit.b2.done

    wm title .catalog_aedit "Attribute Editor"

    set a ${catalog.current_attribute}
    global $a

    .catalog_aedit.label configure -text "Attribute: $attribute"

    .catalog_aedit.f1.label configure -text "  Attribute Id "
    .catalog_aedit.f2.label configure -text "Attribute Type "
    .catalog_aedit.f3.label configure -text " Minimum Value "
    .catalog_aedit.f4.label configure -text " Maximum Value "
    .catalog_aedit.f5.label configure -text "          Size "
    .catalog_aedit.f6.label configure -text "        Offset "

    .catalog_aedit.f1.entry insert 0 [set ${a}(attrid)]
    .catalog_aedit.f2.entry insert 0 [set ${a}(type)]
    .catalog_aedit.f3.entry insert 0 [set ${a}(minval)]
    .catalog_aedit.f4.entry insert 0 [set ${a}(maxval)]
    .catalog_aedit.f5.entry insert 0 [set ${a}(size)]
    .catalog_aedit.f6.entry insert 0 [set ${a}(offset)]

    .catalog_aedit.f7.list   configure -yscroll ".catalog_aedit.f7.scroll set"
    .catalog_aedit.f7.scroll configure -command ".catalog_aedit.f7.list yview"

    .catalog_aedit.b1.new configure -text "New Method" \
        -command {
            set method [catalog.get_entry "Method Name"]
            if {$method != ""} {
                global catalog.current_attribute
                set attribute ${catalog.current_attribute}
                if {[lsearch [set ${attribute}(methods)] $method] == -1} {
                    catalog.new.method $attribute $method
                    .catalog_aedit.f7.list insert end $method
                    lappend ${attribute}(methods) $method
                }
                catalog.edit.method $method
            }
        }

    .catalog_aedit.b1.edit configure -text "Edit Method" \
        -command {
            set l .catalog_aedit.f7.list
            if {[$l curselection] != ""} {
                catalog.edit.method [$l get [$l curselection]]
            }
        }

    .catalog_aedit.b1.delete configure -text "Delete Method" \
        -command {
            set l .catalog_aedit.f7.list
            if {[$l curselection] != ""} {
                set ndx    [$l curselection]
                set method [$l get $ndx]
                $l delete $ndx

                global catalog.current_attribute
                set a ${catalog.current_attribute}
                global $a
                set ${a}(methods) [lreplace [set ${a}(methods)] $ndx $ndx]

                global ${catalog.current_attribute}.$method
                unset ${catalog.current_attribute}.$method
            }
        }

    .catalog_aedit.b2.help configure -text "Help" \
        -command {
            global catalog.help.attribute
            catalog.help ${catalog.help.attribute}
        }

    .catalog_aedit.b2.cancel configure -text "Cancel" \
        -command {
            catch {destroy .catalog_aedit}
        }

    .catalog_aedit.b2.done configure -text "Done" \
        -command {
            global catalog.current_attribute
            set a ${catalog.current_attribute}
            global $a

            set ${a}(attrid) [.catalog_aedit.f1.entry get]
            set ${a}(type)   [.catalog_aedit.f2.entry get]
            set ${a}(minval) [.catalog_aedit.f3.entry get]
            set ${a}(maxval) [.catalog_aedit.f4.entry get]
            set ${a}(size)   [.catalog_aedit.f5.entry get]
            set ${a}(offset) [.catalog_aedit.f6.entry get]

            catch {destroy .catalog_aedit}
        }

    foreach method [set ${a}(methods)] {
        .catalog_aedit.f7.list insert end $method
    }

    pack append .catalog_aedit                         \
        .catalog_aedit.label {top expand fillx pady 4} \
        .catalog_aedit.f1    {top expand fillx}        \
        .catalog_aedit.f2    {top expand fillx}        \
        .catalog_aedit.f3    {top expand fillx}        \
        .catalog_aedit.f4    {top expand fillx}        \
        .catalog_aedit.f5    {top expand fillx}        \
        .catalog_aedit.f6    {top expand fillx}        \
        .catalog_aedit.f7    {top expand fill}         \
        .catalog_aedit.b1    {top expand fillx pady 4} \
        .catalog_aedit.b2    {top expand fillx pady 4}

    pack append .catalog_aedit.f1              \
        .catalog_aedit.f1.label {left  fillx} \
        .catalog_aedit.f1.entry {right expand fillx}

    pack append .catalog_aedit.f2              \
        .catalog_aedit.f2.label {left  fillx} \
        .catalog_aedit.f2.entry {right expand fillx}

    pack append .catalog_aedit.f3              \
        .catalog_aedit.f3.label {left  fillx} \
        .catalog_aedit.f3.entry {right expand fillx}

    pack append .catalog_aedit.f4              \
        .catalog_aedit.f4.label {left  fillx} \
        .catalog_aedit.f4.entry {right expand fillx}

    pack append .catalog_aedit.f5              \
        .catalog_aedit.f5.label {left  fillx} \
        .catalog_aedit.f5.entry {right expand fillx}

    pack append .catalog_aedit.f6              \
        .catalog_aedit.f6.label {left  fillx} \
        .catalog_aedit.f6.entry {right expand fillx}

    pack append .catalog_aedit.f7                    \
        .catalog_aedit.f7.list   {left  expand fill} \
        .catalog_aedit.f7.scroll {right filly}

    pack append .catalog_aedit.b1              \
        .catalog_aedit.b1.new    {left expand} \
        .catalog_aedit.b1.edit   {left expand} \
        .catalog_aedit.b1.delete {left expand}

    pack append .catalog_aedit.b2              \
        .catalog_aedit.b2.help   {left expand} \
        .catalog_aedit.b2.cancel {left expand} \
        .catalog_aedit.b2.done   {left expand}
}

proc catalog.edit.relation {relation} {
    global catalog.current_catalog
    global catalog.current_relation
    set catalog.current_relation "${catalog.current_catalog}.${relation}"

    catch {destroy .catalog_redit}

    toplevel  .catalog_redit
    label     .catalog_redit.label
    frame     .catalog_redit.f1
    frame     .catalog_redit.f2
    frame     .catalog_redit.f3
    frame     .catalog_redit.f4
    frame     .catalog_redit.b1
    frame     .catalog_redit.b2
    frame     .catalog_redit.b3
    label     .catalog_redit.f1.label
    entry     .catalog_redit.f1.entry
    label     .catalog_redit.f2.label
    entry     .catalog_redit.f2.entry
    label     .catalog_redit.f3.label
    entry     .catalog_redit.f3.entry
    listbox   .catalog_redit.f4.list
    scrollbar .catalog_redit.f4.scroll
    button    .catalog_redit.b1.new
    button    .catalog_redit.b1.edit
    button    .catalog_redit.b1.delete
    # button    .catalog_redit.b2.new
    # button    .catalog_redit.b2.edit
    # button    .catalog_redit.b2.delete
    button    .catalog_redit.b3.help
    button    .catalog_redit.b3.cancel
    button    .catalog_redit.b3.done

    wm title .catalog_redit "Relation Editor"

    set r ${catalog.current_relation}
    global $r

    .catalog_redit.label configure -text "Relation: $relation"

    .catalog_redit.f1.label configure -text "    Cardinality "
    .catalog_redit.f2.label configure -text "     Tuple Size "
    .catalog_redit.f3.label configure -text "Number of Pages "

    .catalog_redit.f1.entry insert 0 [set ${r}(card)]
    .catalog_redit.f2.entry insert 0 [set ${r}(tuplesize)]
    .catalog_redit.f3.entry insert 0 [set ${r}(numpages)]

    .catalog_redit.f4.list   configure -yscroll ".catalog_redit.f4.scroll set"
    .catalog_redit.f4.scroll configure -command ".catalog_redit.f4.list yview"

    .catalog_redit.b1.new configure -text "New Attribute" \
        -command {
            set attribute [catalog.get_entry "Attribute Name"]
            if {$attribute != ""} {
                global catalog.current_relation
                set relation ${catalog.current_relation}
                if {[lsearch [set ${relation}(attributes)] $attribute] == -1} {
                    catalog.new.attribute $relation $attribute
                    .catalog_redit.f4.list insert end $attribute
                    lappend ${relation}(attributes) $attribute
                }
                catalog.edit.attribute $attribute
            }
        }

    .catalog_redit.b1.edit configure -text "Edit Attribute" \
        -command {
            set l .catalog_redit.f4.list
            if {[$l curselection] != ""} {
                catalog.edit.attribute [$l get [$l curselection]]
            }
        }

    .catalog_redit.b1.delete configure -text "Delete Attribute" \
        -command {
            set l .catalog_redit.f4.list
            if {[$l curselection] != ""} {
                set ndx       [$l curselection]
                set attribute [$l get $ndx]
                $l delete $ndx

                global catalog.current_relation
                set r ${catalog.current_relation}
                global $r
                set ${r}(attributes) [lreplace [set ${r}(attributes)] $ndx $ndx]

                global ${catalog.current_relation}.$attribute
                unset ${catalog.current_relation}.$attribute
            }
        }

    .catalog_redit.b3.help configure -text "Help" \
        -command {
            global catalog.help.relation
            catalog.help ${catalog.help.relation}
        }

    .catalog_redit.b3.cancel configure -text "Cancel" \
        -command {
            catch {destroy .catalog_redit}
        }

    .catalog_redit.b3.done configure -text "Done" \
        -command {
            global catalog.current_relation
            set r ${catalog.current_relation}
            global $r

            set ${r}(card)      [.catalog_redit.f1.entry get]
            set ${r}(tuplesize) [.catalog_redit.f2.entry get]
            set ${r}(numpages)  [.catalog_redit.f3.entry get]

            catch {destroy .catalog_redit}
        }

    foreach attribute [set ${r}(attributes)] {
        .catalog_redit.f4.list insert end $attribute
    }

    pack append .catalog_redit                         \
        .catalog_redit.label {top expand fillx pady 4} \
        .catalog_redit.f1    {top expand fillx}        \
        .catalog_redit.f2    {top expand fillx}        \
        .catalog_redit.f3    {top expand fillx}        \
        .catalog_redit.f4    {top expand fill}         \
        .catalog_redit.b1    {top expand fillx pady 4} \
        .catalog_redit.b3    {top expand fillx pady 4}

    pack append .catalog_redit.f1              \
        .catalog_redit.f1.label {left  fillx} \
        .catalog_redit.f1.entry {right expand fillx}

    pack append .catalog_redit.f2              \
        .catalog_redit.f2.label {left  fillx} \
        .catalog_redit.f2.entry {right expand fillx}

    pack append .catalog_redit.f3              \
        .catalog_redit.f3.label {left  fillx} \
        .catalog_redit.f3.entry {right expand fillx}

    pack append .catalog_redit.f4                   \
        .catalog_redit.f4.list   {left expand fill} \
        .catalog_redit.f4.scroll {right filly}

    pack append .catalog_redit.b1              \
        .catalog_redit.b1.new    {left expand} \
        .catalog_redit.b1.edit   {left expand} \
        .catalog_redit.b1.delete {left expand}

    pack append .catalog_redit.b3              \
        .catalog_redit.b3.help   {left expand} \
        .catalog_redit.b3.cancel {left expand} \
        .catalog_redit.b3.done   {left expand}
}

proc catalog.edit {file} {
    set catalog CATALOG
    global $catalog

    global catalog.current_catalog
    set catalog.current_catalog $catalog

    global catalog.current_file
    set catalog.current_file $file

    catalog.read $catalog $file

    catch {destroy .catalog_cedit}

    toplevel  .catalog_cedit
    label     .catalog_cedit.label
    frame     .catalog_cedit.f1
    frame     .catalog_cedit.b1
    frame     .catalog_cedit.b2
    listbox   .catalog_cedit.f1.list
    scrollbar .catalog_cedit.f1.scroll
    button    .catalog_cedit.b1.new
    button    .catalog_cedit.b1.edit
    button    .catalog_cedit.b1.delete
    button    .catalog_cedit.b2.help
    button    .catalog_cedit.b2.cancel
    button    .catalog_cedit.b2.done

    wm title .catalog_cedit "Catalog Editor"

    .catalog_cedit.label     configure -text    "Catalog: $file"
    .catalog_cedit.f1.list   configure -yscroll ".catalog_cedit.f1.scroll set"
    .catalog_cedit.f1.scroll configure -command ".catalog_cedit.f1.list yview"

    .catalog_cedit.b1.new configure -text "New Relation" \
        -command {
            set relation [catalog.get_entry "Relation Name"]
            if {$relation != ""} {
                global catalog.current_catalog
                set catalog ${catalog.current_catalog}
                if {[lsearch [set $catalog] $relation] == -1} {
                    catalog.new.relation $catalog $relation
                    global $catalog
                    .catalog_cedit.f1.list insert end $relation
                    lappend $catalog $relation
                }
                catalog.edit.relation $relation
            }
        }

    .catalog_cedit.b1.edit configure -text "Edit Relation" \
        -command {
            set l .catalog_cedit.f1.list
            if {[$l curselection] != ""} {
                catalog.edit.relation [$l get [$l curselection]]
            }
        }

    .catalog_cedit.b1.delete configure -text "Delete Relation" \
        -command {
            set l .catalog_cedit.f1.list
            if {[$l curselection] != ""} {
                set ndx      [$l curselection]
                set relation [$l get $ndx]
                $l delete $ndx

                global catalog.current_catalog
                set catalog ${catalog.current_catalog}
                global $catalog
                set $catalog [lreplace [set $catalog] $ndx $ndx]

                global ${catalog.current_catalog}.$relation
                unset ${catalog.current_catalog}.$relation
            }
        }

    .catalog_cedit.b2.help configure -text "Help" \
        -command {
            global catalog.help.catalog
            catalog.help ${catalog.help.catalog}
        }

    .catalog_cedit.b2.cancel configure -text "Cancel" \
        -command {
            catch {destroy .catalog_cedit}
        }

    .catalog_cedit.b2.done configure -text "Done" \
        -command {
            global catalog.current_catalog
            global catalog.current_file
            catalog.write ${catalog.current_catalog} ${catalog.current_file}

            catch {destroy .catalog_cedit}
        }

    global $catalog
    foreach relation [set $catalog] {
        .catalog_cedit.f1.list insert end $relation
    }

    pack append .catalog_cedit                         \
        .catalog_cedit.label {top expand fillx pady 4} \
        .catalog_cedit.f1    {top expand fill}         \
        .catalog_cedit.b1    {top expand fillx pady 4} \
        .catalog_cedit.b2    {top expand fillx pady 4}

    pack append .catalog_cedit.f1                   \
        .catalog_cedit.f1.list   {left expand fill} \
        .catalog_cedit.f1.scroll {right filly}

    pack append .catalog_cedit.b1              \
        .catalog_cedit.b1.new    {left expand} \
        .catalog_cedit.b1.edit   {left expand} \
        .catalog_cedit.b1.delete {left expand}

    pack append .catalog_cedit.b2              \
        .catalog_cedit.b2.help   {left expand} \
        .catalog_cedit.b2.cancel {left expand} \
        .catalog_cedit.b2.done   {left expand}
}

#################################################################

set catalog.help.method {
                  Welcome to the Method Editor.

The name of the method being edited is displayed at the top of
the editor window.  The fields specified in the entry widgets can
be edited.

Cost:                 the cost of using this method for accessing the relation
Order:               `R' for random, `A' for ascending, `D' for descending.
Which:               `0' to ignore following fields, `1' to consider them.

Clustering:           `1' if index is clustered, `0' otherwise.
Keys:                 number of unique keys
Pages:                number of pages accessed to reach a leaf
Type:                 selection type:
                          `0' for range
                          `1' for exact match
                          `2' for both range and exact match
                          `3' for undefined
Filename:             name of the index file
Multi-Column Indexes: the names of the attributes for multi-column
                      specified enclosed by `%' characters.  The order
                      of the attribute names is signifiant.  The current
                      attribute is implicitly assumed to prefix the
                      rest of the attributes specified in this field.

All of the above fields should be valid.  The editor does not
check your input for validity and silently accepts your changes.
So please make sure that you specify legal values for all fields.

To save your current editing changes to the method and quit the
editor, click on "Done".  The updates are stored in memory and
will be written to disk when the catalog editor is closed.

To *discard* all your current changes, click on "Cancel".  The
changes made to the fields of the methods will be discarded.
}

set catalog.help.attribute {
                Welcome to the Attribute Editor.

The name of the attribute being edited is displayed at the top of
the editor window.  The fields specified in the entry widgets can
be edited.

Attribute Id:   a unique id for the attribute in this relation
Attribute Type: either `T' for string, `I' for integer.
Minimum Value:  smallest value for attribute (string or integer).
Maximum Value:  largest value for attribute (string or integer).
Size:           size of attribute (in bytes).
Offset:         offset of the attribute in the enclosing relation tuple.

All of the above fields should be valid.  The editor does not
check your input for validity and silently accepts your changes.
So please make sure that you specify legal values for all fields.

The listbox shows the names of the methods (i.e. indexes) on the
attribute.  The methods can be edited or discarded, and new
methods can be defined for the attribute.

To save your current editing changes to the attribute and quit
the editor, click on "Done".  The updates are stored in memory
and will be written to disk when the catalog editor is closed.

To *discard* all your current changes, click on "Cancel".  The
changes made to the fields of the attribute will be discarded.
However, new methods will be retained, and deleted methods will
*not* be recovered.  Beware.
}

set catalog.help.relation {
                 Welcome to the Relation Editor.

The name of the relation being edited is displayed at the top of
the editor window.  The fields specified in the entry widgets can
be edited.

Cardinality:     the cardinality of the relation.
Tuple Size:      the size of each tuple (in bytes).
Number of Pages: the number of database pages occupied by the relation.

All of the above fields should be valid positive integers.  The
editor does not check your input for validity and silently
accepts your changes.  So please make sure that you specify legal
values for all fields.

The listbox shows the names of the attributes of the relation.
The attributes can be edited or discarded, and new attributes can
be defined for the relation.

To save your current editing changes to the relation and quit the
editor, click on "Done".  The updates are stored in memory and
will be written to disk when the catalog editor is closed.

To *discard* all your current changes, click on "Cancel".  The
changes made to the fields of the relation will be discarded.
However, new attributes/methods will be retained, and deleted
attributes/methods will *not* be recovered.  Beware.
}

set catalog.help.catalog {
                 Welcome to the Catalog Editor.

The name of the catalog file being edited in displayed at the top
of the editor window.  The listbox shows the names of all
relations currently defined in the catalog.

From this window, you can edit the currently loaded catalog in
three ways:

    (1) Add new relations to the catalog.
    (2) Edit existing relations.
    (3) Delete relations from the catalog.

Clicking on the appropriate buttons will invoke the appropriate
action.

To save your current editing changes to the catalog file and quit
the editor, click on "Done".  The changes from the current
editing session will be written to the catalog file.

To *discard* all your current changes, click on "Cancel".  The
disk file that the current catalog was loaded from will not be
modified.  To start afresh, simply reinvoke the catalog editor
from the main menu of the GUI.
}

#################################################################
