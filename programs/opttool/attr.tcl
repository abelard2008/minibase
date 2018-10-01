frame .nameframe -bd 2
frame .attrframe 
frame .typeframe -bd 2 
frame .valframe -bd 2 
frame .minvalframe
frame .maxvalframe
frame .sizeframe 
frame .submitframe 
frame .infoframe 

label .arnamelabel -text "Relation name:"
label .arnameentry -width 20 -bd 2 -textvariable relname
pack .arnamelabel .arnameentry -in .nameframe -side left -fill both

label .attrnamelabel -text "Attribute name:"
entry .attrnameentry -width 20 -bd 2 -relief sunken -textvariable attrname
pack .attrnamelabel .attrnameentry -in .attrframe -side left -fill both

label .typelabel -text "Type:" -anchor w
radiobutton .tstring -text String -relief flat -variable indtype -value "T" -anchor w
radiobutton .tinteger -text Integer -relief flat -variable indtype -value "I" -anchor w
radiobutton .treal -text Real -relief flat -variable indtype -value "R" -anchor w

pack .typelabel .tstring .tinteger .treal -in .typeframe -side top -fill x

label .minvallabel -text "Minimum Value:"
entry .minvalentry -width 10 -relief sunken -bd 2 -textvariable minval
pack .minvallabel .minvalentry -in .minvalframe -side left -fill both

label .maxvallabel -text "Maximum Value:"
entry .maxvalentry -width 10 -relief sunken -bd 2 -textvariable maxval
pack .maxvallabel .maxvalentry -in .maxvalframe -side left -fill both

label .sizelabel -text "Size: "
entry .sizeentry -width 5 -relief sunken -bd 2 -textvariable atsize 
pack .sizelabel .sizeentry -in .sizeframe -side left -fill both

pack .minvalframe .maxvalframe .sizeframe  -in .valframe -side top -fill both -padx 1m -pady 1m

button .relcancel -text Cancel -command minibaseatrCancel
button .relok -text Ok -command minibaseatrOk
pack .relcancel .relok -in .submitframe -side right -padx 1m -pady 1m

pack .typeframe .valframe -in .infoframe -side left -padx 1m -pady 1m -fill both

pack .nameframe .attrframe .infoframe .submitframe -side top -padx 1m -pady 1m -fill x

