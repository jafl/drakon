/******************************************************************************
 MainDirector.h

	Copyright (C) 2001 by Glenn W. Bach.

 *****************************************************************************/

#ifndef _H_MainDirector
#define _H_MainDirector

#include <jx-af/jx/JXWindowDirector.h>
#include <jx-af/jcore/JPrefObject.h>

class JXToolBar;
class JXTabGroup;
class JXStaticText;
class JXTextMenu;
class JXFunctionTask;

class SystemStats;
class ProcessList;
class ProcessTable;
class ProcessTreeList;

class MainDirector : public JXWindowDirector, public JPrefObject
{
public:

public:

	MainDirector(JXDirector* supervisor);
	~MainDirector() override;

protected:

	void	ReadPrefs(std::istream& input) override;
	void	WritePrefs(std::ostream& output) const override;

private:

	JXTextMenu*	itsFileMenu;
	JXTextMenu*	itsProcessMenu;
	JXTextMenu*	itsPrefsMenu;
	JXTextMenu*	itsHelpMenu;

	ProcessList*	itsProcessList;
	JXFunctionTask*	itsTimerTask;

	SystemStats*	itsSystemStats;

	JXTabGroup*			itsTabGroup;
	ProcessTable*		itsProcessTable;
	ProcessTreeList*	itsProcessTree;

// begin JXLayout

	JXToolBar*    itsToolBar;
	JXStaticText* itsFullCmdDisplay;

// end JXLayout

private:

	void	BuildWindow();

	void	UpdateFileMenu();
	void	HandleFileMenu(const JIndex index);

	void	UpdateProcessMenu();
	void	HandleProcessMenu(const JIndex index);

	void	HandlePrefsMenu(const JIndex index);

	void	HandleHelpMenu(const JIndex index);

	void	ReadPrefs();
	void	WritePrefs();
};

#endif
