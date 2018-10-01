# defaults.tcl - set default X resources and bindings
#
# Author: Allan Brighton (allan@piano.sta.sub.org)


# setup default X resources
# set up some default resources for color, mono or unknown (PC with Windows?)

proc set_default_resources {} {
    switch [winfo screenvisual .] {
	{staticgray} {
	    # Mono 
	    set normalbg white
	    set normalfg black
	    set accentbg black
	    set accentfg white
	    set workspbg white
	    set workspfg black
	    set darkbg white
	    set darkfg white
	}
	{unknown} {
	    # PC with Windows ?
	    set normalbg #c0c0c0
	    set normalfg black
	    set accentbg maroon
	    set accentfg white
	    set workspbg #c0c0c0
	    set workspfg black
	    set darkbg #808080
	    set darkfg #a0a0a0
	}
	{default} {
	    # color or greyscale ?
	    catch {tk colormodel . color}
	    set normalbg #b5c4a2
	    set normalfg black
	    set accentbg maroon
	    set accentfg white 
	    set workspbg #daedc3
	    set workspfg black 
	    set darkbg #7a846e
	    set darkfg #cee0ba
	    option add Tk*activeBackground #cee0ba
	    option add Tk*activeForeground black 
	}
    }

    # add the options to the option database, but allow
    # them to be overridden by the user's X defaults

    option add Tk*background $normalbg
    option add Tk*foreground $normalfg
    option add Tk*accentBackground $accentbg
    option add Tk*accentForeground $accentbg
    option add Tk*workspaceBackground $workspbg
    option add Tk*workspaceForeground $workspfg
    option add Tk*darkBackground $darkbg
    option add Tk*darkForeground $darkfg
    #option add Tk*normalFont -adobe-courier-medium-r-*-*-*-120-*-*-*-*-*-*
    #option add Tk*boldFont -adobe-courier-bold-r-*-*-*-120-*-*-*-*-*-*
    #option add Tk*labelFont -adobe-helvetica-bold-r-*-*-*-120-*-*-*-*-*-*
    
    # Now get the user's X defaults and use them to set specific 
    # widget properties

    #set normal_font [option get . normalFont Tk]
    #set bold_font [option get . boldFont Tk]
    #set label_font [option get . labelFont Tk]
    set normalbg [option get . background Tk]
    set normalfg [option get . foreground Tk]
    set accentbg [option get . accentBackground Tk]
    set accentfg [option get . accentForeground Tk]
    set workspbg [option get . workspaceBackground Tk]
    set workspfg [option get . workspaceForeground Tk]
    set darkbg   [option get . darkBackground Tk]
    set darkfg   [option get . darkForeground Tk]

    option add Tk*Scrollbar.Background $darkbg 
    option add Tk*Scrollbar.Foreground $darkfg 
    option add Tk*Canvas.background $workspbg 
    option add Tk*Canvas.foreground $workspfg 
    option add Tk*Listbox.background $workspbg 
    option add Tk*Listbox.foreground $workspfg 
    option add Tk*Entry.relief sunken
    option add Tk*Scrollbar.relief sunken
    option add Tk*Radiobutton.relief flat
    option add Tk*Checkbutton.relief flat
    #option add Tk*Font $normal_font
    #option add Tk*Label.font $label_font
    #option add Tk*Canvas.font $label_font
    #option add Tk*Listbox.font $normal_font
    #option add Tk*Checkbutton.font $label_font
}


# set up some default bindings for Tk widgets

proc set_default_bindings {} {
    bind Entry <2> {%W insert insert [utilGetSelection]}
    bind Entry <1> "[bind Entry <1>]; %W select clear"
    bind Entry <Double-Button-1> {%W select from 0; %W select to end}
    bind Entry <Return> { }
    bind Entry <Tab> { }
    bind Entry <Right> {%W icursor [expr [%W index insert]+1]}
    bind Entry <Left> {%W icursor [expr [%W index insert]-1]}
    bind Text <2> {%W insert insert [utilGetSelection]}
}


# set up the application defaults

proc set_app_defaults {} {
    set_default_resources
    set_default_bindings
}
