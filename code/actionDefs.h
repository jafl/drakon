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

#define kShowAllAction   "GPMShowAll"
#define kPauseAction     "GPMPause"
#define kContinueAction  "GPMContinue"
#define kReNiceAction    "GPMReNice"
#define kStopAction      "GPMStop"
#define kKillAction      "GPMKill"

// Preferences menu

#define kToolbarButtonsAction "GPMEditToolBar"

#endif
