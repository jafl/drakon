// This file was automatically generated by jx_menu_editor.
// Do not edit it directly!
// Any changes you make will be silently overwritten.

#ifndef _H_MainDirector_Preferences
#define _H_MainDirector_Preferences

static const JUtf8Byte* kPreferencesMenuStr =
"* %i EditToolBar::MainDirector"
;

#include "MainDirector-Preferences-enum.h"


static void ConfigurePreferencesMenu(JXTextMenu* menu, const int offset = 0) {
	if (offset == 0 && JXMenu::GetDisplayStyle() == JXMenu::kWindowsStyle) {
		menu->SetShortcuts("#r");
	}
};

#endif
