/******************************************************************************
 globals.h

   Copyright (C) 2001 by Glenn W. Bach.

 *****************************************************************************/

#ifndef _H_globals
#define _H_globals

// we include these for convenience

#include "App.h"
#include "PrefsManager.h"
#include <jx-af/jx/JXDocumentManager.h>
#include <jx-af/jx/jXGlobals.h>

class MDIServer;

App*			GetApplication();
PrefsManager*	GetPrefsManager();
void			ForgetPrefsManager();
MDIServer*		GetMDIServer();

const JString&	GetVersionNumberStr();
JString			GetVersionStr();

bool			GetSystemMemory(JSize* mem);

	// called by App

bool	CreateGlobals(App* app);
void	DeleteGlobals();
void	CleanUpBeforeSuddenDeath(const JXDocumentManager::SafetySaveReason reason);

#endif
