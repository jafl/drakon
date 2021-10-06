/******************************************************************************
 MDIServer.h

	Copyright (C) 2001 by Glenn W. Bach.

 *****************************************************************************/

#ifndef _H_MDIServer
#define _H_MDIServer

#include <jx-af/jx/JXMDIServer.h>

class MainDirector;

class MDIServer : public JXMDIServer
{
public:

	MDIServer();

	virtual ~MDIServer();

	static void	PrintCommandLineHelp();

	void	SetMainDirector(MainDirector* dir);

protected:

	void	HandleMDIRequest(const JString& dir,
									 const JPtrArray<JString>& argList) override;

private:

	MainDirector*	itsMainDirector;
};


/******************************************************************************
 SetMainDirector

 *****************************************************************************/

inline void
MDIServer::SetMainDirector
	(
	MainDirector* dir
	)
{
	itsMainDirector = dir;
}

#endif
