frame .nameframe
frame .pagesframe
frame .cardframe
frame .submitframe

label .namelabel -text "Relation name:"
entry .nameentry -width 20 -relief sunken -bd 2 -textvariable relname
pack .namelabel .nameentry -in .nameframe -side left -fill both

label .pageslabel -text "Number of pages:"
entry .pagesentry -width 5 -relief sunken -bd 2 -textvariable numpage
pack .pageslabel .pagesentry -in .pagesframe -side left -fill both

label .cardlabel -text "Cardinality:"
entry .cardentry -width 5 -relief sunken -bd 2 -textvariable card
pack .cardlabel .cardentry -in .cardframe -side left -fill both

button .relcancel -text Cancel -command minibaserelCancel
button .relok -text Ok -command minibaserelOk
pack .relcancel .relok -in .submitframe -side right -padx 1m -pady 1m

pack .nameframe .pagesframe .cardframe .submitframe -side top -padx 1m -pady 1m -fill x
