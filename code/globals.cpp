/******************************************************************************
 globals.cpp

	Access to global objects and factories.

	Copyright (C) 2001 by Glenn W. Bach.

 ******************************************************************************/

#include "globals.h"
#include "App.h"
#include "PrefsManager.h"
#include "MDIServer.h"
#include <jx-af/jcore/JRegex.h>
#include <jx-af/jcore/jStreamUtil.h>

#ifdef _J_HAS_SYSCTL
#include <sys/sysctl.h>
#endif

#include <jx-af/jcore/jAssert.h>

static App*				theApplication  = nullptr;		// owns itself
static PrefsManager*	thePrefsManager = nullptr;
static MDIServer*		theMDIServer    = nullptr;

static JSize			theSystemMemory = 0;
static const JRegex		totalMemoryPattern("^MemTotal:\\s*([0-9]+)");

/******************************************************************************
 CreateGlobals

	Returns true if this is the first time the program is run.

 ******************************************************************************/

bool
CreateGlobals
	(
	App* app
	)
{
	theApplication = app;

	bool isNew;
	thePrefsManager	= jnew PrefsManager(&isNew);

	JXInitHelp();

	theMDIServer = jnew MDIServer;

#ifdef _J_HAS_PROC
{
	std::ifstream ms("/proc/meminfo");
	JString line;
	while (ms.good() && !ms.eof())
	{
		line = JReadLine(ms);

		const JStringMatch m = totalMemoryPattern.Match(line, JRegex::kIncludeSubmatches);
		if (!m.IsEmpty())
		{
			m.GetSubstring(1).ConvertToUInt(&theSystemMemory);	// usually kB
			break;
		}
	}
}
#elif defined _J_HAS_SYSCTL
{
	int mib[] = { CTL_HW, HW_PHYSMEM };
	int memPages;
	size_t len = sizeof(memPages);
	if (sysctl(mib, 2, &memPages, &len, nullptr, 0) == 0)
	{
		theSystemMemory = memPages;	// bytes
	}
}
#endif

	return isNew;
}

/******************************************************************************
 DeleteGlobals

 ******************************************************************************/

void
DeleteGlobals()
{
	theApplication = nullptr;
	theMDIServer   = nullptr;

	// this must be last so everybody else can use it to save their setup

	jdelete thePrefsManager;
	thePrefsManager = nullptr;
}

/******************************************************************************
 CleanUpBeforeSuddenDeath

	This must be the last one called by App so we can save
	the preferences to disk.

	*** If the server is dead, you cannot call any code that contacts it.

 ******************************************************************************/

void
CleanUpBeforeSuddenDeath
	(
	const JXDocumentManager::SafetySaveReason reason
	)
{
	if (reason != JXDocumentManager::kAssertFired)
	{
//		theMDIServer->JPrefObject::WritePrefs();
	}

	// must be last to save everything

	thePrefsManager->CleanUpBeforeSuddenDeath(reason);
}

/******************************************************************************
 GetApplication

 ******************************************************************************/

App*
GetApplication()
{
	assert( theApplication != nullptr );
	return theApplication;
}

/******************************************************************************
 GetPrefsManager

 ******************************************************************************/

PrefsManager*
GetPrefsManager()
{
	assert( thePrefsManager != nullptr );
	return thePrefsManager;
}

/******************************************************************************
 ForgetPrefsManager

	Called when license is not accepted, to avoid writing prefs file.

 ******************************************************************************/

void
ForgetPrefsManager()
{
	thePrefsManager = nullptr;
}

/******************************************************************************
 GetMDIServer

 ******************************************************************************/

MDIServer*
GetMDIServer()
{
	assert( theMDIServer != nullptr );
	return theMDIServer;
}

/******************************************************************************
 GetVersionNumberStr

 ******************************************************************************/

const JString&
GetVersionNumberStr()
{
	return JGetString("VERSION");
}

/******************************************************************************
 GetVersionStr

 ******************************************************************************/

JString
GetVersionStr()
{
	const JUtf8Byte* map[] =
	{
		"version",   JGetString("VERSION").GetBytes(),
		"copyright", JGetString("COPYRIGHT").GetBytes()
	};
	return JGetString("Description::globals", map, sizeof(map));
}

/******************************************************************************
 GetWMClassInstance

 ******************************************************************************/

const JUtf8Byte*
GetWMClassInstance()
{
	return "Drakon";
}

const JUtf8Byte*
GetMainWindowClass()
{
	return "Drakon_Main_Window";
}

/******************************************************************************
 GetSystemMemory

 ******************************************************************************/

bool
GetSystemMemory
	(
	JSize* mem
	)
{
	*mem = theSystemMemory;
	return theSystemMemory != 0;
}
