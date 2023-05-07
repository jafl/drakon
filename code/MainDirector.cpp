/******************************************************************************
 MainDirector.cpp

	BASE CLASS = public JXWindowDirector

	Copyright (C) 2001 by Glenn W. Bach.

 *****************************************************************************/

#include "MainDirector.h"
#include "SystemStats.h"
#include "ProcessTable.h"
#include "ListHeaderWidget.h"
#include "ProcessTreeList.h"
#include "TreeHeaderWidget.h"
#include "ProcessList.h"

#include "globals.h"
#include "actionDefs.h"

#include <jx-af/jx/JXApplication.h>
#include <jx-af/jx/JXHelpManager.h>
#include <jx-af/jx/JXMenuBar.h>
#include <jx-af/jx/JXToolBar.h>
#include <jx-af/jx/JXTabGroup.h>
#include <jx-af/jx/JXScrollbarSet.h>
#include <jx-af/jx/JXTextMenu.h>
#include <jx-af/jx/JXStaticText.h>
#include <jx-af/jx/JXFunctionTask.h>
#include <jx-af/jx/JXImage.h>
#include <jx-af/jx/JXColorManager.h>
#include <jx-af/jx/JXWindow.h>

#include <jx-af/jcore/JNamedTreeList.h>
#include <jx-af/jcore/JSimpleProcess.h>
#include <jx-af/jcore/jAssert.h>

#include "pause.xpm"
#include "cont.xpm"
#include "slow.xpm"
#include "gpm_stop.xpm"
#include "gpm_all_processes.xpm"
#include <jx-af/image/jx/jx_edit_clear.xpm>

const JCoordinate kStatusHeight = 30;
const JCoordinate kStatusMargin = 5;

const JFileVersion kCurrentPrefsVersion	= 6;

	// version  6: removed showFullCommand; added tree sorting column
	// version  5: added selected tab index
	// version  4: added list sorting column
	// version  3: added showFullCommand
	// version  2: added window geometry
	// version  1: add showUserOnly

const Time kTimerDelay = 3000;

enum
{
	kListTabIndex = 1,
	kTreeTabIndex
};

// File menu

static const JUtf8Byte* kFileMenuStr =
	"Quit %k Meta-Q %i" kJXQuitAction;

enum
{
	kQuitCmd = 1
};

// Process menu

static const JUtf8Byte* kProcessMenuStr =
	"    Show processes from all users %b %i" kShowAllAction
	"%l| End process                      %i" kStopAction
	"  | Kill process                     %i" kKillAction
	"%l| Pause process                    %i" kPauseAction
	"  | Continue process                 %i" kContinueAction
	"%l| Re-nice process                  %i" kReNiceAction;
//	"  | Send signal to process";

enum
{
	kShowAllCmd = 1,
	kEndCmd,
	kKillCmd,
	kPauseCmd,
	kContinueCmd,
	kReNiceCmd
};

// Prefs menu

static const JUtf8Byte* kPrefsMenuStr =
	"Toolbar buttons...  %i" kToolbarButtonsAction;

enum
{
	kEditToolBarCmd = 1
};

// Help menu

static const JUtf8Byte* kHelpMenuStr =
	"    About"
	"%l| Table of Contents       %i" kJXHelpTOCAction
	"  | Overview"
	"  | This window       %k F1 %i" kJXHelpSpecificAction
	"%l| Changes"
	"  | Credits";

enum
{
	kAboutCmd = 1,
	kTOCCmd,
	kOverviewCmd,
	kThisWindowCmd,
	kChangesCmd,
	kCreditsCmd
};

/******************************************************************************
 Constructor

 *****************************************************************************/

MainDirector::MainDirector
	(
	JXDirector* supervisor
	)
	:
	JXWindowDirector(supervisor),
	JPrefObject(GetPrefsManager(), kMainDirectorID),
	itsTimerTask(nullptr)
{
	itsProcessList = jnew ProcessList();
	assert( itsProcessList != nullptr );

	BuildWindow();

	itsTimerTask = jnew JXFunctionTask(kTimerDelay, std::bind(&ProcessList::Update, itsProcessList));
	assert( itsTimerTask != nullptr );
	itsTimerTask->Start();

	JPrefObject::ReadPrefs();
}

/******************************************************************************
 Destructor

 *****************************************************************************/

MainDirector::~MainDirector()
{
	JPrefObject::WritePrefs();
	jdelete itsProcessList;
	jdelete itsTimerTask;
}

/******************************************************************************
 BuildWindow

 ******************************************************************************/

#include "gpm_main_window_icon.xpm"
#include <jx-af/image/jx/jx_help_specific.xpm>
#include <jx-af/image/jx/jx_help_toc.xpm>

void
MainDirector::BuildWindow()
{
// begin JXLayout

	auto* window = jnew JXWindow(this, 530,350, JString::empty);
	assert( window != nullptr );

	auto* menuBar =
		jnew JXMenuBar(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 530,30);
	assert( menuBar != nullptr );

	itsToolBar =
		jnew JXToolBar(GetPrefsManager(), kMainToolBarID, menuBar, window,
					JXWidget::kHElastic, JXWidget::kVElastic, 0,30, 530,300);
	assert( itsToolBar != nullptr );

	itsFullCmdDisplay =
		jnew JXStaticText(JString::empty, false, true, false, nullptr, window,
					JXWidget::kHElastic, JXWidget::kFixedBottom, 0,330, 530,20);
	assert( itsFullCmdDisplay != nullptr );

// end JXLayout

	window->SetTitle(JGetString("WindowTitle::MainDirector"));
	window->SetCloseAction(JXWindow::kQuitApp);
	window->SetMinSize(530, 250);
	window->SetWMClass(GetWMClassInstance(), GetMainWindowClass());

	auto* image = jnew JXImage(GetDisplay(), gpm_main_window_icon);
	assert( image != nullptr );
	window->SetIcon(image);

	// system stats

	itsSystemStats =
		jnew SystemStats(itsProcessList, itsToolBar->GetWidgetEnclosure(),
					   JXWidget::kHElastic, JXWidget::kFixedTop,
					   0,kStatusMargin, 100,kStatusHeight);
	assert( itsSystemStats != nullptr );
	itsSystemStats->FitToEnclosure(true, false);

	// tab group

	itsTabGroup =
		jnew JXTabGroup(itsToolBar->GetWidgetEnclosure(),
					   JXWidget::kHElastic, JXWidget::kVElastic,
					   0,0, 100,100);
	assert( itsTabGroup != nullptr );
	itsTabGroup->FitToEnclosure();
	ListenTo(itsTabGroup->GetCardEnclosure(), std::function([this](const JXCardFile::CardIndexChanged&)
	{
		JIndex index;
		const bool ok = itsTabGroup->GetCurrentTabIndex(&index);
		assert( ok );

		const ProcessEntry* entry;
		if (index == kListTabIndex && itsProcessTree->GetSelectedProcess(&entry))
		{
			itsProcessTable->SelectProcess(*entry);
		}
		else if (index == kTreeTabIndex && itsProcessTable->GetSelectedProcess(&entry))
		{
			itsProcessTree->SelectProcess(*entry);
		}
	}));

	const JCoordinate statusHeight = kStatusHeight + 2*kStatusMargin;
	itsTabGroup->AdjustSize(0, -statusHeight);
	itsTabGroup->Move(0, statusHeight);

	JXContainer* listTab = itsTabGroup->AppendTab(JGetString("ListTabTitle::MainDirector"));
	JXContainer* treeTab = itsTabGroup->AppendTab(JGetString("TreeTabTitle::MainDirector"));

	// list view

	auto* scrollbarSet =
		jnew JXScrollbarSet(listTab, JXWidget::kHElastic, JXWidget::kVElastic,
						   0,0, 100,100);
	assert( scrollbarSet != nullptr );
	scrollbarSet->FitToEnclosure();

	const JCoordinate kHeaderHeight	= 25;
	const JCoordinate tableHeight   = scrollbarSet->GetScrollEnclosure()->GetBoundsHeight() - kHeaderHeight;

	itsProcessTable =
		jnew ProcessTable(itsProcessList, itsFullCmdDisplay,
			scrollbarSet, scrollbarSet->GetScrollEnclosure(),
			JXWidget::kHElastic, JXWidget::kVElastic,
			0,kHeaderHeight, 100,tableHeight);
	assert( itsProcessTable != nullptr );
	itsProcessTable->FitToEnclosure(true, false);

	auto* tableHeader =
		jnew ListHeaderWidget(itsProcessTable, itsProcessList,
			scrollbarSet, scrollbarSet->GetScrollEnclosure(),
			JXWidget::kHElastic, JXWidget::kFixedTop,
			0,0, 100,kHeaderHeight);
	assert( tableHeader != nullptr );
	tableHeader->FitToEnclosure(true, false);

	// tree view

	scrollbarSet =
		jnew JXScrollbarSet(treeTab, JXWidget::kHElastic, JXWidget::kVElastic,
						   0,0, 100,100);
	assert( scrollbarSet != nullptr );
	scrollbarSet->FitToEnclosure();

	auto* treeList = jnew JNamedTreeList(itsProcessList->GetProcessTree());
	assert( treeList != nullptr );

	itsProcessTree =
		jnew ProcessTreeList(itsProcessList, treeList, itsFullCmdDisplay,
			scrollbarSet, scrollbarSet->GetScrollEnclosure(),
			JXWidget::kHElastic, JXWidget::kVElastic,
			0,kHeaderHeight, 100,tableHeight);
	assert( itsProcessTree != nullptr );
	itsProcessTree->FitToEnclosure(true, false);

	auto* treeHeader =
		jnew TreeHeaderWidget(itsProcessTree, itsProcessList,
			scrollbarSet, scrollbarSet->GetScrollEnclosure(),
			JXWidget::kHElastic, JXWidget::kFixedTop,
			0,0, 100,kHeaderHeight);
	assert( treeHeader != nullptr );
	treeHeader->FitToEnclosure(true, false);

	itsProcessTable->SetDefaultRowHeight(itsProcessTree->GetDefaultRowHeight());

	// focus hocus pocus

	window->UnregisterFocusWidget(itsFullCmdDisplay);
	window->RegisterFocusWidget(itsFullCmdDisplay);

	// menus

	itsFileMenu = menuBar->AppendTextMenu(JGetString("FileMenuTitle::JXGlobal"));
	itsFileMenu->SetMenuItems(kFileMenuStr);
	itsFileMenu->SetUpdateAction(JXMenu::kDisableNone);
	itsFileMenu->AttachHandlers(this,
		std::bind(&MainDirector::UpdateFileMenu, this),
		std::bind(&MainDirector::HandleFileMenu, this, std::placeholders::_1));

	itsProcessMenu = menuBar->AppendTextMenu(JGetString("ProcessMenuTitle::MainDirector"));
	itsProcessMenu->SetMenuItems(kProcessMenuStr, "ProcessTable");
	itsProcessMenu->AttachHandlers(this,
		std::bind(&MainDirector::UpdateProcessMenu, this),
		std::bind(&MainDirector::HandleProcessMenu, this, std::placeholders::_1));

	itsProcessMenu->SetItemImage(kShowAllCmd, JXPM(gpm_all_processes));
	itsProcessMenu->SetItemImage(kEndCmd, JXPM(gpm_stop));
	itsProcessMenu->SetItemImage(kKillCmd, JXPM(jx_edit_clear));
	itsProcessMenu->SetItemImage(kPauseCmd, JXPM(gpm_pause));
	itsProcessMenu->SetItemImage(kContinueCmd, JXPM(gpm_cont));
	itsProcessMenu->SetItemImage(kReNiceCmd, JXPM(gpm_slow));

	itsPrefsMenu = menuBar->AppendTextMenu(JGetString("PrefsMenuTitle::JXGlobal"));
	itsPrefsMenu->SetMenuItems(kPrefsMenuStr);
	itsPrefsMenu->SetUpdateAction(JXMenu::kDisableNone);
	itsPrefsMenu->AttachHandlers(this,
		std::bind(&MainDirector::UpdatePrefsMenu, this),
		std::bind(&MainDirector::HandlePrefsMenu, this, std::placeholders::_1));

	itsHelpMenu = menuBar->AppendTextMenu(JGetString("HelpMenuTitle::JXGlobal"));
	itsHelpMenu->SetMenuItems(kHelpMenuStr);
	itsHelpMenu->SetUpdateAction(JXMenu::kDisableNone);
	itsHelpMenu->AttachHandlers(this,
		std::bind(&MainDirector::UpdateHelpMenu, this),
		std::bind(&MainDirector::HandleHelpMenu, this, std::placeholders::_1));

	itsHelpMenu->SetItemImage(kTOCCmd,        jx_help_toc);
	itsHelpMenu->SetItemImage(kThisWindowCmd, jx_help_specific);

	// must be done after creating widgets

	itsToolBar->LoadPrefs();
	if (itsToolBar->IsEmpty())
	{
		itsToolBar->AppendButton(itsFileMenu, kQuitCmd);
		itsToolBar->NewGroup();
		itsToolBar->AppendButton(itsProcessMenu, kShowAllCmd);
		itsToolBar->NewGroup();
		itsToolBar->AppendButton(itsProcessMenu, kEndCmd);
		itsToolBar->AppendButton(itsProcessMenu, kKillCmd);
		itsToolBar->NewGroup();
		itsToolBar->AppendButton(itsProcessMenu, kPauseCmd);
		itsToolBar->AppendButton(itsProcessMenu, kContinueCmd);
		itsToolBar->NewGroup();
		itsToolBar->AppendButton(itsProcessMenu, kReNiceCmd);
		itsToolBar->NewGroup();
		itsToolBar->AppendButton(itsHelpMenu, kTOCCmd);
		itsToolBar->AppendButton(itsHelpMenu, kThisWindowCmd);
	}
}

/******************************************************************************
 UpdateFileMenu

 ******************************************************************************/

void
MainDirector::UpdateFileMenu()
{
}

/******************************************************************************
 HandleFileMenu

 ******************************************************************************/

void
MainDirector::HandleFileMenu
	(
	const JIndex index
	)
{
	if (index == kQuitCmd)
	{
		GetApplication()->Quit();
	}
}

/******************************************************************************
 UpdateProcessMenu (private)

 ******************************************************************************/

void
MainDirector::UpdateProcessMenu()
{
	itsProcessMenu->EnableItem(kShowAllCmd);
	if (!itsProcessList->WillShowUserOnly())
	{
		itsProcessMenu->CheckItem(kShowAllCmd);
	}

	JIndex tabIndex;
	const bool ok = itsTabGroup->GetCurrentTabIndex(&tabIndex);
	assert( ok );

	const ProcessEntry* entry;
	if (((tabIndex == kListTabIndex && itsProcessTable->GetSelectedProcess(&entry)) ||
		 (tabIndex == kTreeTabIndex && itsProcessTree->GetSelectedProcess(&entry))) &&
		 entry->GetState() != ProcessEntry::kZombie)
	{
		const bool notSelf = entry->GetPID() != getpid();
		itsProcessMenu->EnableItem(kEndCmd);
		itsProcessMenu->EnableItem(kKillCmd);
		itsProcessMenu->SetItemEnabled(kPauseCmd, notSelf);
		itsProcessMenu->SetItemEnabled(kContinueCmd, notSelf);
		itsProcessMenu->EnableItem(kReNiceCmd);
	}
}

/******************************************************************************
 HandleProcessMenu (private)

 ******************************************************************************/

void
MainDirector::HandleProcessMenu
	(
	const JIndex index
	)
{
	if (index == kShowAllCmd)
	{
		itsProcessList->ShouldShowUserOnly(!itsProcessList->WillShowUserOnly());
		return;
	}

	JIndex tabIndex;
	const bool ok = itsTabGroup->GetCurrentTabIndex(&tabIndex);
	assert( ok );

	const ProcessEntry* entry;
	if ((tabIndex == kListTabIndex && itsProcessTable->GetSelectedProcess(&entry)) ||
		(tabIndex == kTreeTabIndex && itsProcessTree->GetSelectedProcess(&entry)))
	{
		if (entry->GetState() == ProcessEntry::kZombie)
		{
			return;
		}

		if (index == kReNiceCmd)
		{
			JSetProcessPriority(entry->GetPID(), 19);
			return;
		}

		JIndex sigValue = 0;
		if (index == kEndCmd)
		{
			sigValue = SIGTERM;
		}
		else if (index == kKillCmd)
		{
			sigValue = SIGKILL;
		}
		else if (index == kPauseCmd)
		{
			sigValue = SIGSTOP;
		}
		else if (index == kContinueCmd)
		{
			sigValue = SIGCONT;
		}

		const pid_t pid = entry->GetPID();
		if (sigValue == 0 || pid == 0)
		{
			return;
		}

		const uid_t uid = getuid();
		if (uid == 0 || entry->GetUID() == uid)
		{
			JSendSignalToProcess(pid, sigValue);
			itsProcessList->Update();
		}
		else
		{
			JString cmd("xterm -title 'Drakon sudo' -e /bin/sh -c 'sudo -k ; sudo kill -");
			cmd += JString((JUInt64) sigValue);
			cmd += " ";
			cmd += JString((JUInt64) pid);
			cmd += "'";
			JSimpleProcess::Create(cmd, true);
		}
	}
}

/******************************************************************************
 UpdatePrefsMenu

 ******************************************************************************/

void
MainDirector::UpdatePrefsMenu()
{
}

/******************************************************************************
 HandlePrefsMenu

 ******************************************************************************/

void
MainDirector::HandlePrefsMenu
	(
	const JIndex index
	)
{
	if (index == kEditToolBarCmd)
	{
		itsToolBar->Edit();
	}
}

/******************************************************************************
 UpdateHelpMenu

 ******************************************************************************/

void
MainDirector::UpdateHelpMenu()
{
}

/******************************************************************************
 HandleHelpMenu

 ******************************************************************************/

void
MainDirector::HandleHelpMenu
	(
	const JIndex index
	)
{
	if (index == kAboutCmd)
	{
		GetApplication()->DisplayAbout();
	}

	else if (index == kTOCCmd)
	{
		JXGetHelpManager()->ShowTOC();
	}
	else if (index == kOverviewCmd)
	{
		JXGetHelpManager()->ShowSection("OverviewHelp");
	}
	else if (index == kThisWindowCmd)
	{
		JXGetHelpManager()->ShowSection("MainHelp");
	}

	else if (index == kChangesCmd)
	{
		JXGetHelpManager()->ShowChangeLog();
	}
	else if (index == kCreditsCmd)
	{
		JXGetHelpManager()->ShowCredits();
	}
}

/******************************************************************************
 ReadPrefs (virtual protected)

 ******************************************************************************/

void
MainDirector::ReadPrefs
	(
	std::istream& input
	)
{
	JFileVersion vers;
	input >> vers;
	if (vers > kCurrentPrefsVersion)
	{
		return;
	}

	if (vers >= 1)
	{
		bool show;
		input >> JBoolFromString(show);
		itsProcessList->ShouldShowUserOnly(show);
	}

	if (vers >= 2)
	{
		GetWindow()->ReadGeometry(input);
		GetWindow()->Deiconify();
	}

	if (3 <= vers && vers < 6)
	{
		bool full;
		input >> JBoolFromString(full);
	}

	if (vers >= 4)
	{
		int type;
		input >> type;
		itsProcessList->ListColSelected(type);
	}

	if (vers >= 6)
	{
		int type;
		input >> type;
		itsProcessList->TreeColSelected(type);
	}

	if (vers >= 5)
	{
		JIndex tabIndex;
		input >> tabIndex;
		itsTabGroup->ShowTab(tabIndex);
	}
}

/******************************************************************************
 WritePrefs (virtual protected)

 ******************************************************************************/

void
MainDirector::WritePrefs
	(
	std::ostream& output
	)
	const
{
	output << kCurrentPrefsVersion;
	output << ' ' << JBoolToString(itsProcessList->WillShowUserOnly());

	GetWindow()->WriteGeometry(output);

	output << ' ' << (int) itsProcessList->GetSelectedListCol();
	output << ' ' << (int) itsProcessList->GetSelectedTreeCol();

	JIndex tabIndex;
	const bool ok = itsTabGroup->GetCurrentTabIndex(&tabIndex);
	assert( ok );
	output << ' ' << tabIndex;
}
