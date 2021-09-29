/******************************************************************************
 actionDefs.h

	Shared actions for use as menu item ID's and in keybinding tables.
	These preprocessor definitions allow them to be included in static
	menu definitions.

	Copyright (C) 2006 by John Lindal.

 ******************************************************************************/

#ifndef _H_ActionDefs
#define _H_ActionDefs

#include <jx-af/jx/jXActionDefs.h>	// for convenience

// Process menu

#define kShowAllAction   "ShowAll"
#define kPauseAction     "Pause"
#define kContinueAction  "Continue"
#define kReNiceAction    "ReNice"
#define kStopAction      "Stop"
#define kKillAction      "Kill"

// Preferences menu

#define kToolbarButtonsAction "EditToolBar"

#endif
