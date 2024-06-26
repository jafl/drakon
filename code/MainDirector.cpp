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

	BuildWindow();

	itsTimerTask = jnew JXFunctionTask(kTimerDelay,
		std::bind(&ProcessList::Update, itsProcessList),
		"ProcessList::Update");
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
#include "MainDirector-File.h"
#include "MainDirector-Process.h"
#include "MainDirector-Preferences.h"
#include "MainDirector-Help.h"

void
MainDirector::BuildWindow()
{
	auto* treeList = jnew JNamedTreeList(itsProcessList->GetProcessTree());

// begin JXLayout

	auto* window = jnew JXWindow(this, 530,350, JGetString("WindowTitle::MainDirector::JXLayout"));
	window->SetMinSize(200, 200);
	window->SetWMClass(JXGetApplication()->GetWMName().GetBytes(), "Drakon_Main_Window");

	auto* menuBar =
		jnew JXMenuBar(window,
					JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 530,30);
	assert( menuBar != nullptr );

	itsToolBar =
		jnew JXToolBar(GetPrefsManager(), kMainToolBarID, menuBar, window,
					JXWidget::kHElastic, JXWidget::kVElastic, 0,30, 530,300);

	itsSystemStats =
		jnew SystemStats(itsProcessList, itsToolBar->GetWidgetEnclosure(),
					JXWidget::kHElastic, JXWidget::kFixedTop, 0,5, 530,30);

	itsTabGroup =
		jnew JXTabGroup(itsToolBar->GetWidgetEnclosure(),
					JXWidget::kHElastic, JXWidget::kVElastic, 0,40, 530,260);
	itsTabGroup->AppendTab(JGetString("itsTabGroup::tab1::MainDirector::JXLayout"));
	itsTabGroup->AppendTab(JGetString("itsTabGroup::tab2::MainDirector::JXLayout"));

	auto* treeScrollbarSet =
		jnew JXScrollbarSet(itsTabGroup->GetTabEnclosure(2),
					JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 526,229);
	assert( treeScrollbarSet != nullptr );

	auto* listScrollbarSet =
		jnew JXScrollbarSet(itsTabGroup->GetTabEnclosure(1),
					JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 526,229);
	assert( listScrollbarSet != nullptr );

	itsFullCmdDisplay =
		jnew JXStaticText(JString::empty, false, true, false, nullptr, window,
					JXWidget::kHElastic, JXWidget::kFixedBottom, 0,330, 530,20);

	itsProcessTree =
		jnew ProcessTreeList(itsProcessList, treeList, itsFullCmdDisplay, treeScrollbarSet, treeScrollbarSet->GetScrollEnclosure(),
					JXWidget::kHElastic, JXWidget::kVElastic, 0,25, 526,204);

	itsProcessTable =
		jnew ProcessTable(itsProcessList, itsFullCmdDisplay, listScrollbarSet, listScrollbarSet->GetScrollEnclosure(),
					JXWidget::kHElastic, JXWidget::kVElastic, 0,25, 526,204);

	auto* treeHeader =
		jnew TreeHeaderWidget(itsProcessTree, itsProcessList, treeScrollbarSet, treeScrollbarSet->GetScrollEnclosure(),
					JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 526,25);
	assert( treeHeader != nullptr );

	auto* tableHeader =
		jnew ListHeaderWidget(itsProcessTable, itsProcessList, listScrollbarSet, listScrollbarSet->GetScrollEnclosure(),
					JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 526,25);
	assert( tableHeader != nullptr );

// end JXLayout

	window->SetCloseAction(JXWindow::kQuitApp);
	window->SetIcon(jnew JXImage(GetDisplay(), gpm_main_window_icon));

	// tab group

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

	itsProcessTable->SetDefaultRowHeight(itsProcessTree->GetDefaultRowHeight());

	// focus hocus pocus

	window->UnregisterFocusWidget(itsFullCmdDisplay);
	window->RegisterFocusWidget(itsFullCmdDisplay);

	// menus

	itsFileMenu = menuBar->AppendTextMenu(JGetString("MenuTitle::MainDirector_File"));
	itsFileMenu->SetMenuItems(kFileMenuStr);
	itsFileMenu->SetUpdateAction(JXMenu::kDisableNone);
	itsFileMenu->AttachHandlers(this,
		&MainDirector::UpdateFileMenu,
		&MainDirector::HandleFileMenu);
	ConfigureFileMenu(itsFileMenu);

	itsProcessMenu = menuBar->AppendTextMenu(JGetString("MenuTitle::MainDirector_Process"));
	itsProcessMenu->SetMenuItems(kProcessMenuStr);
	itsProcessMenu->AttachHandlers(this,
		&MainDirector::UpdateProcessMenu,
		&MainDirector::HandleProcessMenu);
	ConfigureProcessMenu(itsProcessMenu);

	itsPrefsMenu = menuBar->AppendTextMenu(JGetString("MenuTitle::MainDirector_Preferences"));
	itsPrefsMenu->SetMenuItems(kPreferencesMenuStr);
	itsPrefsMenu->SetUpdateAction(JXMenu::kDisableNone);
	itsPrefsMenu->AttachHandler(this, &MainDirector::HandlePrefsMenu);
	ConfigurePreferencesMenu(itsPrefsMenu);

	itsHelpMenu = menuBar->AppendTextMenu(JGetString("MenuTitle::MainDirector_Help"));
	itsHelpMenu->SetMenuItems(kHelpMenuStr);
	itsHelpMenu->SetUpdateAction(JXMenu::kDisableNone);
	itsHelpMenu->AttachHandler(this, &MainDirector::HandleHelpMenu);
	ConfigureHelpMenu(itsHelpMenu);

	// must be done after creating widgets

	auto f = std::function(UpgradeToolBarID);
	itsToolBar->LoadPrefs(&f);
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
 UpgradeToolBarID (static private)

 ******************************************************************************/

void
MainDirector::UpgradeToolBarID
	(
	JString* s
	)
{
	if (s->StartsWith("GPM"))
	{
		JStringIterator iter(s);
		iter.SkipNext(3);
		iter.RemoveAllPrev();
		*s += "::MainDirector";
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
			cmd += JString(sigValue);
			cmd += " ";
			cmd += JString(pid);
			cmd += "'";
			JSimpleProcess::Create(cmd, true);
		}
	}
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
