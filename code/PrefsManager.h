/******************************************************************************
 PrefsManager.h

	Copyright (C) 2001 by Glenn W. Bach.

 *****************************************************************************/

#ifndef _H_PrefsManager
#define _H_PrefsManager

#include <jx-af/jx/JXPrefsManager.h>

// Preferences -- do not change ID's once they are assigned

enum
{
	kProgramVersionID = 1,
	kgCSFSetupID,
	kMainDirectorID,
	kMainToolBarID,
	kVersionCheckID
};

class PrefsManager : public JXPrefsManager
{
public:

	PrefsManager(bool* isNew);

	virtual ~PrefsManager();

	JString GetPrevVersionStr() const;

protected:

	virtual void	UpgradeData(const bool isNew, const JFileVersion currentVersion);
	virtual void	SaveAllBeforeDestruct();
};

#endif
