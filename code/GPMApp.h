/******************************************************************************
 GPMApp.h

	Copyright (C) 2001 by Glenn W. Bach.

 *****************************************************************************/

#ifndef _H_GPMApp
#define _H_GPMApp

#include <jx-af/jx/JXApplication.h>

class GPMApp : public JXApplication
{
public:

	GPMApp(int* argc, char* argv[], bool* displayAbout, JString* prevVersStr);

	virtual ~GPMApp();

	void	DisplayAbout(const JString& prevVersStr = JString::empty);

	static const JUtf8Byte*	GetAppSignature();
	static void				InitStrings();

protected:

	virtual void	CleanUpBeforeSuddenDeath(const JXDocumentManager::SafetySaveReason reason);
};

#endif
