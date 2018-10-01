
frame .infoframe
frame .valframe

frame .nameframe
label .arnamelabel -text "Relation Name:"
label .arnameentry -width 20 -bd 2 -textvariable relname
pack .arnamelabel .arnameentry -in .nameframe -side left -fill both

frame .attrframe
label .attrlabel -text "Attribute Name:"
label .attrentry -width 20 -bd 2 -textvariable attrname
pack .attrlabel .attrentry -in .attrframe -side left -fill both

frame .orderframe
label .orderlabel -text "Index Order:" -anchor w
radiobutton .orandom -text Random -relief flat -variable ordtype -value "R" -anchor w
radiobutton .oasc -text Ascending -relief flat -variable ordtype -value "A" -anchor w
radiobutton .odesc -text Descending -relief flat -variable ordtype -value "D" -anchor w
pack .orderlabel .orandom .oasc .odesc -in .orderframe -side top -fill x

frame .typeframe
label .typelabel -text "Index Type:" -anchor w
radiobutton .tFileScan -text "File Scan" -relief flat -variable itype -value "F" -anchor w
radiobutton .tHash -text "Hash" -relief flat -variable itype -value "H" -anchor w
radiobutton .tBTree -text "BTree" -relief flat -variable itype -value "B" -anchor w
pack .typelabel .tFileScan .tHash .tBTree -in .typeframe -side top -fill x

frame .leafframe
label .leaflabel -text "Number of Leaf Pages:"
entry .leafentry -width 10 -relief sunken -bd 2 -textvariable leafPages
pack .leaflabel .leafentry -in .leafframe -side left -fill both

frame .clustframe
checkbutton .clust -text "Clustered Index" -variable clustered
pack .clust -in .clustframe -side left -fill both

frame .distframe
label .distlabel -text "Distinct Keys:"
entry .distentry -width 10 -relief sunken -bd 2 -textvariable dKeys
pack .distlabel .distentry -in .distframe -side left -fill both

frame .depthframe
label .depthlabel -text "Index Depth:"
entry .depthentry -width 10 -relief sunken -bd 2 -textvariable maxval
pack .depthlabel .depthentry -in .depthframe -side left -fill both

frame .fileframe
label .filelabel -text "Index Filename: "
entry .fileentry -width 20 -relief sunken -bd 2 -textvariable atsize 
pack .filelabel .fileentry -in .fileframe -side left -fill both

pack .leafframe .distframe .depthframe .clustframe -in .valframe -side top -fill both

frame .submitframe
button .relcancel -text Cancel -command minibaseatrCancel
button .relok -text Ok -command minibaseatrOk
pack .relcancel .relok -in .submitframe -side right -padx 1m -pady 1m

pack .typeframe .orderframe .valframe -in .infoframe -side left -padx 1m -pady 1m -fill both

pack .nameframe .attrframe .fileframe .infoframe .submitframe -side top -padx 1m -pady 1m -fill both


