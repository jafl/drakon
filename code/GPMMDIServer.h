/******************************************************************************
 GPMMDIServer.h

	Copyright (C) 2001 by Glenn W. Bach.

 *****************************************************************************/

#ifndef _H_GPMMDIServer
#define _H_GPMMDIServer

#include <jx-af/jx/JXMDIServer.h>

class GPMMainDirector;

class GPMMDIServer : public JXMDIServer
{
public:

	GPMMDIServer();

	virtual ~GPMMDIServer();

	static void	PrintCommandLineHelp();

	void	SetMainDirector(GPMMainDirector* dir);

protected:

	virtual void	HandleMDIRequest(const JString& dir,
									 const JPtrArray<JString>& argList) override;

private:

	GPMMainDirector*	itsMainDirector;
};


/******************************************************************************
 SetMainDirector

 *****************************************************************************/

inline void
GPMMDIServer::SetMainDirector
	(
	GPMMainDirector* dir
	)
{
	itsMainDirector = dir;
}

#endif
