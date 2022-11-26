/******************************************************************************
 ProcessTable.cpp

	BASE CLASS = JXTable

	Copyright (C) 2001 by Glenn W. Bach.

 *****************************************************************************/

#include "ProcessTable.h"
#include "ProcessList.h"

#include "globals.h"

#include <jx-af/jx/JXDisplay.h>
#include <jx-af/jx/JXColorManager.h>
#include <jx-af/jx/JXMenuBar.h>
#include <jx-af/jx/JXTextMenu.h>
#include <jx-af/jx/JXToolBar.h>
#include <jx-af/jx/JXTEBase.h>
#include <jx-af/jx/JXImage.h>
#include <jx-af/jx/JXImageCache.h>

#include <jx-af/jcore/JPainter.h>
#include <jx-af/jcore/JTableSelection.h>

#include <jx-af/jcore/JSimpleProcess.h>
#include <jx-af/jcore/jASCIIConstants.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <jx-af/jcore/jAssert.h>

#include <jx-af/image/jx/jx_edit_clear.xpm>
#include "pause.xpm"
#include "cont.xpm"
#include "slow.xpm"
#include "gpm_stop.xpm"

const JCoordinate kDefColWidth	= 100;
const JCoordinate kDefRowHeight	= 20;
const JCoordinate kHMarginWidth	= 5;

static const JUtf8Byte* kContextMenuStr =
	"    End process"
	"  | Kill process"
	"%l| Pause process"
	"  | Continue process"
	"%l| Re-nice process";
//	"  | Send signal to process";

enum
{
	kContextEndCmd = 1,
	kContextKillCmd,
	kContextPauseCmd,
	kContextContinueCmd,
	kContextReNiceCmd
};

/******************************************************************************
 Constructor

 *****************************************************************************/

ProcessTable::ProcessTable
	(
	ProcessList*		list,
	JXTEBase*			fullCmdDisplay,
	JXScrollbarSet*		scrollbarSet,
	JXContainer*		enclosure,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate	w,
	const JCoordinate	h
	)
	:
	JXTable(kDefRowHeight, kDefColWidth, scrollbarSet,
			enclosure, hSizing, vSizing, x,y, w,h),
	itsList(list),
	itsContextMenu(nullptr),
	itsSelectedEntry(nullptr),
	itsFullCmdDisplay(fullCmdDisplay),
	itsZombieImage(nullptr)
{
	AppendCols(ProcessList::kListCount);
	SetColWidth(ProcessList::kListState,  20);
	SetColWidth(ProcessList::kListPID,    50);
	SetColWidth(ProcessList::kListUser,   75);
	SetColWidth(ProcessList::kListNice,   40);
	SetColWidth(ProcessList::kListSize,   60);
	SetColWidth(ProcessList::kListCPU,    50);
	SetColWidth(ProcessList::kListMemory, 70);
	SetColWidth(ProcessList::kListTime,   60);

	SetRowBorderInfo(0, JColorManager::GetBlackColor());
	SetColBorderInfo(0, JColorManager::GetBlackColor());

	itsZombieImage = GetDisplay()->GetImageCache()->GetImage(jx_edit_clear);

	itsContextMenu = CreateContextMenu(this);
	ListenTo(itsContextMenu);

	ListenTo(itsList);
}

/******************************************************************************
 Destructor

 *****************************************************************************/

ProcessTable::~ProcessTable()
{
}

/******************************************************************************
 CreateContextMenu (static)

 ******************************************************************************/

JXTextMenu*
ProcessTable::CreateContextMenu
	(
	JXContainer* enclosure
	)
{
	auto* menu = jnew JXTextMenu(JString::empty, enclosure, kFixedLeft, kFixedTop, 0,0, 10,10);
	assert( menu != nullptr );
	menu->SetMenuItems(kContextMenuStr);
	menu->SetToHiddenPopupMenu(true);

	menu->SetItemImage(kContextEndCmd,      JXPM(gpm_stop));
	menu->SetItemImage(kContextKillCmd,     JXPM(jx_edit_clear));
	menu->SetItemImage(kContextPauseCmd,    JXPM(gpm_pause));
	menu->SetItemImage(kContextContinueCmd, JXPM(gpm_cont));
	menu->SetItemImage(kContextReNiceCmd,   JXPM(gpm_slow));

	return menu;
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
ProcessTable::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsList && message.Is(ProcessList::kPrepareForUpdate))
	{
		StopListening(itsSelectedEntry);
		if (GetSelectedProcess(&itsSelectedEntry))
		{
			ClearWhenGoingAway(itsSelectedEntry, &itsSelectedEntry);
		}
	}

	else if (sender == itsList && message.Is(ProcessList::kListChanged))
	{
		JTableSelection& s = GetTableSelection();
		s.ClearSelection();

		const JSize count	= GetRowCount();
		const JSize lCount	= itsList->GetElementCount();
		if (lCount > count)
		{
			AppendRows(lCount - count);
		}
		else if (count > lCount)
		{
			RemoveNextRows(1, count - lCount);
		}

		JIndex index;
		if (itsSelectedEntry != nullptr)
		{
			if (itsList->GetEntryIndex(itsSelectedEntry, &index))
			{
				s.SelectRow(index);
			}

			StopListening(itsSelectedEntry);
			itsSelectedEntry = nullptr;
		}

		Refresh();
	}

	else if (sender == itsContextMenu && message.Is(JXMenu::kNeedsUpdate))
	{
		UpdateContextMenu();
	}
	else if (sender == itsContextMenu && message.Is(JXMenu::kItemSelected))
	{
		 const auto* selection =
			dynamic_cast<const JXMenu::ItemSelected*>(&message);
		assert( selection != nullptr );
		HandleContextMenu(selection->GetIndex());
	}

	else
	{
		if (sender == &(GetTableSelection()) && message.Is(JTableData::kRectChanged))
		{
			const ProcessEntry* entry;
			if (IsVisible() && GetSelectedProcess(&entry))
			{
				itsFullCmdDisplay->GetText()->SetText(entry->GetFullCommand());
			}
		}

		JXTable::Receive(sender, message);
	}
}

/******************************************************************************
 TableDrawCell (virtual protected)

 ******************************************************************************/

void
ProcessTable::TableDrawCell
	(
	JPainter&		p,
	const JPoint&	cell,
	const JRect&	rect
	)
{
	DrawRowBackground(p, cell, rect, JColorManager::GetGrayColor(95));

	HilightIfSelected(p, cell, rect);

	const ProcessEntry& entry = *(itsList->GetProcessEntry(cell.y));

	JString str;
	JPainter::HAlignment halign = JPainter::kHAlignRight;
	if (cell.x == ProcessList::kListState)
	{
		DrawProcessState(entry, p, rect, *itsZombieImage);
		return;
	}
	else if (cell.x == ProcessList::kListPID)
	{
		str	= JString((JUInt64) entry.GetPID());
	}
	else if (cell.x == ProcessList::kListUser)
	{
		str		= entry.GetUser();
		halign	= JPainter::kHAlignLeft;
	}
/*	else if (cell.x == ProcessList::kListPPID)
	{
		str	= JString((JUInt64) entry.GetPPID());
	}
	else if (cell.x == ProcessList::kListPriority)
	{
		str	= JString((JUInt64) entry.GetPriority());
	}
*/	else if (cell.x == ProcessList::kListNice)
	{
		str	= JString((JUInt64) entry.GetNice());
	}
	else if (cell.x == ProcessList::kListSize)
	{
		str	= JString((JUInt64) entry.GetSize());
	}
/*	else if (cell.x == ProcessList::kListResident)
	{
		str	= JString((JUInt64) entry.GetResident());
	}
	else if (cell.x == ProcessList::kListShare)
	{
		str	= JString((JUInt64) entry.GetShare());
	}
*/	else if (cell.x == ProcessList::kListCPU)
	{
		str	= JString(entry.GetPercentCPU(), 1);
	}
	else if (cell.x == ProcessList::kListMemory)
	{
		str	= JString(entry.GetPercentMemory(), 1);
	}
	else if (cell.x == ProcessList::kListTime)
	{
		str	= JString((JUInt64) entry.GetTime());
	}
	else if (cell.x == ProcessList::kListCommand)
	{
		str	= entry.GetCommand();
		halign	= JPainter::kHAlignLeft;
	}

	JRect r = rect;
	r.Shrink(kHMarginWidth, 0);
	p.JPainter::String(r, str, halign, JPainter::kVAlignCenter);
}

/******************************************************************************
 DrawRowBackground (static)

 ******************************************************************************/

void
ProcessTable::DrawRowBackground
	(
	JPainter&			p,
	const JPoint&		cell,
	const JRect&		rect,
	const JColorID	color
	)
{
	if (cell.y % 2 == 1)
	{
		p.SetPenColor(color);
		p.SetFilling(true);
		p.Rect(rect);
		p.SetFilling(false);
	}
}

/******************************************************************************
 DrawProcessState (static)

 ******************************************************************************/

void
ProcessTable::DrawProcessState
	(
	const ProcessEntry&	entry,
	JPainter&				p,
	const JRect&			rect,
	const JXImage&			zombieImage
	)
{
	if (entry.GetState() == ProcessEntry::kZombie)
	{
		p.Image(zombieImage, zombieImage.GetBounds(), rect);
	}
	else
	{
		JRect r(rect.ycenter()-3, rect.xcenter()-3,
				rect.ycenter()+4, rect.xcenter()+4);
		p.SetPenColor(entry.GetState() == ProcessEntry::kStopped ?
					  JColorManager::GetRedColor() : JColorManager::GetGreenColor());
		p.SetFilling(true);
		p.Ellipse(r);
		p.SetPenColor(JColorManager::GetBlackColor());
		p.SetFilling(false);
		p.Ellipse(r);
	}
}

/******************************************************************************
 HandleMouseDown (virtual protected)

 ******************************************************************************/

void
ProcessTable::HandleMouseDown
	(
	const JPoint&			pt,
	const JXMouseButton		button,
	const JSize				clickCount,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	if (ScrollForWheel(button, modifiers))
	{
		return;
	}

	JTableSelection& s	= GetTableSelection();
	s.ClearSelection();
	itsKeyBuffer.Clear();

	JPoint cell;
	if (!GetCell(pt, &cell))
	{
		return;
	}

	s.SelectRow(cell.y);

	if (cell.x == ProcessList::kListState)
	{
		const ProcessEntry* entry = itsList->GetProcessEntry(cell.y);
		ToggleProcessState(*entry);
		itsList->Update();
	}
	else if (button == kJXRightButton)
	{
		itsContextMenu->PopUp(this, pt, buttonStates, modifiers);
	}
}

/******************************************************************************
 ToggleProcessState (static)

 ******************************************************************************/

void
ProcessTable::ToggleProcessState
	(
	const ProcessEntry& entry
	)
{
	const pid_t pid = entry.GetPID();
	const uid_t uid = getuid();
	if (uid == 0 || entry.GetUID() != uid)
	{
		// too dangerous to allow toggle
	}
	else if (pid == getpid() || pid == 0)
	{
		// do not allow pause
	}
	else if (entry.GetState() == ProcessEntry::kZombie)
	{
		// cannot do anything
	}
	else if (entry.GetState() == ProcessEntry::kStopped)
	{
		JSendSignalToProcess(pid, SIGCONT);
	}
	else
	{
		JSendSignalToProcess(pid, SIGSTOP);
	}
}

/******************************************************************************
 HandleFocusEvent (virtual protected)

 ******************************************************************************/

void
ProcessTable::HandleFocusEvent()
{
	JXTable::HandleFocusEvent();
	itsKeyBuffer.Clear();
}

/******************************************************************************
 HandleKeyPress (virtual public)

 ******************************************************************************/

void
ProcessTable::HandleKeyPress
	(
	const JUtf8Character&	c,
	const int				keySym,
	const JXKeyModifiers&	modifiers
	)
{
	if (c == ' ' || c == kJEscapeKey)
	{
		itsKeyBuffer.Clear();
		GetTableSelection().ClearSelection();
	}

	// incremental search

	else if (c.IsPrint() && !modifiers.control() && !modifiers.meta())
	{
		itsKeyBuffer.Append(c);

		ProcessEntry* entry;
		JIndex index;
		if (itsList->ClosestMatch(itsKeyBuffer, &entry) &&
			itsList->GetEntryIndex(entry, &index))
		{
			(GetTableSelection()).ClearSelection();
			(GetTableSelection()).SelectRow(index);
			TableScrollToCell(JPoint(1, index));
		}
		else
		{
			(GetTableSelection()).ClearSelection();
		}
	}

	else
	{
		if (c.IsPrint())
		{
			itsKeyBuffer.Clear();
		}
		JXTable::HandleKeyPress(c, keySym, modifiers);
	}
}

/******************************************************************************
 UpdateContextMenu (private)

 ******************************************************************************/

void
ProcessTable::UpdateContextMenu()
{
	const ProcessEntry* entry;
	if (GetSelectedProcess(&entry))
	{
		UpdateContextMenu(itsContextMenu, *entry);
	}
}

// static

void
ProcessTable::UpdateContextMenu
	(
	JXTextMenu*				menu,
	const ProcessEntry&	entry
	)
{
	if (entry.GetState() != ProcessEntry::kZombie)
	{
		const bool notSelf = entry.GetPID() != getpid();
		menu->EnableItem(kContextEndCmd);
		menu->EnableItem(kContextKillCmd);
		menu->SetItemEnabled(kContextPauseCmd, notSelf);
		menu->SetItemEnabled(kContextContinueCmd, notSelf);
		menu->EnableItem(kContextReNiceCmd);
	}
}

/******************************************************************************
 HandleContextMenu (private)

 ******************************************************************************/

void
ProcessTable::HandleContextMenu
	(
	const JIndex index
	)
{
	const ProcessEntry* entry;
	if (GetSelectedProcess(&entry))
	{
		HandleContextMenu(index, *entry, itsList);
	}
}

// static

void
ProcessTable::HandleContextMenu
	(
	const JIndex			menuIndex,
	const ProcessEntry&	entry,
	ProcessList*			list
	)
{
	if (entry.GetState() == ProcessEntry::kZombie)
	{
		return;
	}

	if (menuIndex == kContextReNiceCmd)
	{
		JSetProcessPriority(entry.GetPID(), 19);
		list->Update();
		return;
	}

	JIndex sigValue = 0;
	if (menuIndex == kContextEndCmd)
	{
		sigValue = SIGTERM;
	}
	else if (menuIndex == kContextKillCmd)
	{
		sigValue = SIGKILL;
	}
	else if (menuIndex == kContextPauseCmd)
	{
		sigValue = SIGSTOP;
	}
	else if (menuIndex == kContextContinueCmd)
	{
		sigValue = SIGCONT;
	}

	const pid_t pid = entry.GetPID();
	if (sigValue == 0 || pid == 0)
	{
		return;
	}

	const uid_t uid = getuid();
	if (uid == 0 || entry.GetUID() == uid)
	{
		JSendSignalToProcess(pid, sigValue);
		list->Update();
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

/******************************************************************************
 GetSelectedProcess

 ******************************************************************************/

bool
ProcessTable::GetSelectedProcess
	(
	const ProcessEntry** entry
	)
	const
{
	JPoint cell;
	if (GetTableSelection().GetFirstSelectedCell(&cell))
	{
		*entry = itsList->GetProcessEntry(cell.y);
		return true;
	}
	else
	{
		*entry = nullptr;
		return false;
	}
}

/******************************************************************************
 SelectProcess

 ******************************************************************************/

void
ProcessTable::SelectProcess
	(
	const ProcessEntry& entry
	)
{
	JTableSelection& s = GetTableSelection();
	s.ClearSelection();
	itsKeyBuffer.Clear();

	JIndex index;
	if (itsList->GetEntryIndex(&entry, &index))
	{
		s.SelectRow(index);
		TableScrollToCell(JPoint(1, index));
	}
}

/******************************************************************************
 ApertureResized (virtual protected)

 ******************************************************************************/

void
ProcessTable::ApertureResized
	(
	const JCoordinate dw,
	const JCoordinate dh
	)
{
	JXTable::ApertureResized(dw,dh);
	AdjustColWidths();
}

/******************************************************************************
 AdjustColWidths (private)

 ******************************************************************************/

void
ProcessTable::AdjustColWidths()
{
	const JCoordinate availWidth =
		GetApertureWidth() - (GetBoundsWidth() - GetColWidth(ProcessList::kListCommand));
	SetColWidth(ProcessList::kListCommand, JMax(kDefColWidth, availWidth));
}
