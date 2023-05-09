/******************************************************************************
 ProcessTreeList.cpp

	BASE CLASS = JXNamedTreeListWidget

	Copyright (C) 2006 by John Lindal.

 *****************************************************************************/

#include "ProcessTreeList.h"
#include "ProcessTable.h"
#include "ProcessList.h"
#include "globals.h"

#include <jx-af/jx/JXDisplay.h>
#include <jx-af/jx/JXTextMenu.h>
#include <jx-af/jx/JXTEBase.h>
#include <jx-af/jx/JXImage.h>
#include <jx-af/jx/JXImageCache.h>
#include <jx-af/jx/JXColorManager.h>

#include <jx-af/jcore/JTableSelection.h>
#include <jx-af/jcore/JTreeList.h>
#include <jx-af/jcore/JPainter.h>
#include <jx-af/jcore/jAssert.h>

#include <jx-af/image/jx/jx_edit_clear.xpm>

const JCoordinate kHMarginWidth	= 5;

/******************************************************************************
 Constructor

 *****************************************************************************/

ProcessTreeList::ProcessTreeList
	(
	ProcessList*		list,
	JNamedTreeList*		treeList,
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
	JXNamedTreeListWidget(treeList, scrollbarSet, enclosure, hSizing,vSizing, x,y, w,h),
	itsList(list),
	itsContextMenu(nullptr),
	itsSelectedEntry(nullptr),
	itsFullCmdDisplay(fullCmdDisplay),
	itsZombieImage(nullptr)
{
	AppendCols(ProcessList::kTreeCount - 2);
	SetColWidth(ProcessList::kTreeState,  20);
	SetColWidth(ProcessList::kTreePID,    50);
	SetColWidth(ProcessList::kTreeUser,   75);
	SetColWidth(ProcessList::kTreeNice,   40);
	SetColWidth(ProcessList::kTreeSize,   60);
	SetColWidth(ProcessList::kTreeCPU,    50);
	SetColWidth(ProcessList::kTreeMemory, 60);
	SetColWidth(ProcessList::kTreeTime,   60);

	itsZombieImage = GetDisplay()->GetImageCache()->GetImage(jx_edit_clear);

	itsContextMenu = ProcessTable::CreateContextMenu(this);
	itsContextMenu->AttachHandlers(this,
		&ProcessTreeList::UpdateContextMenu,
		&ProcessTreeList::HandleContextMenu);

	ListenTo(itsList);
}

/******************************************************************************
 Destructor

 *****************************************************************************/

ProcessTreeList::~ProcessTreeList()
{
}

/******************************************************************************
 Receive (virtual protected)

 ******************************************************************************/

void
ProcessTreeList::Receive
	(
	JBroadcaster*	sender,
	const Message&	message
	)
{
	if (sender == itsList && message.Is(ProcessList::kPrepareForUpdate))
	{
		if (itsSelectedEntry != nullptr)
		{
			StopListening(itsSelectedEntry);
			itsSelectedEntry = nullptr;
		}

		if (GetSelectedProcess(&itsSelectedEntry))
		{
			ClearWhenGoingAway(itsSelectedEntry, &itsSelectedEntry);
		}
	}

	else if (sender == itsList && message.Is(ProcessList::kListChanged))
	{
		JTableSelection& s = GetTableSelection();
		s.ClearSelection();

		JIndex index;
		if (itsSelectedEntry != nullptr &&
			GetTreeList()->FindNode(itsSelectedEntry, &index))
		{
			s.SelectRow(index);

			StopListening(itsSelectedEntry);
			itsSelectedEntry = nullptr;
		}

		Refresh();
	}

	else
	{
		if (sender == &GetTableSelection() && message.Is(JTableData::kRectChanged))
		{
			const ProcessEntry* entry;
			if (IsVisible() && GetSelectedProcess(&entry))
			{
				itsFullCmdDisplay->GetText()->SetText(entry->GetFullCommand());
			}
		}

		JXNamedTreeListWidget::Receive(sender, message);
	}
}

/******************************************************************************
 TableDrawCell (virtual protected)

 ******************************************************************************/

void
ProcessTreeList::TableDrawCell
	(
	JPainter&		p,
	const JPoint&	cell,
	const JRect&	rect
	)
{
	ProcessTable::DrawRowBackground(p, cell, rect, JColorManager::GetGrayColor(95));

	if (cell.x == ProcessList::kTreeOpenClose ||
		cell.x == ProcessList::kTreeCommand)
	{
		JXNamedTreeListWidget::TableDrawCell(p, cell, rect);
		return;
	}

	HilightIfSelected(p, cell, rect);

	const JTreeNode* node        = GetTreeList()->GetNode(cell.y);
	const ProcessEntry& entry = * dynamic_cast<const ProcessEntry*>(node);

	JString str;
	JPainter::HAlign halign = JPainter::HAlign::kRight;
	if (cell.x == ProcessList::kTreeState)
	{
		ProcessTable::DrawProcessState(entry, p, rect, *itsZombieImage);
	}
	else if (cell.x == ProcessList::kTreePID)
	{
		str	= JString((JUInt64) entry.GetPID());
	}
	else if (cell.x == ProcessList::kTreeUser)
	{
		str    = entry.GetUser();
		halign = JPainter::HAlign::kLeft;
	}
/*	else if (cell.x == ProcessList::kTreePPID)
	{
		str	= JString((JUInt64) entry.GetPPID());
	}
	else if (cell.x == ProcessList::kTreePriority)
	{
		str	= JString((JUInt64) entry.GetPriority());
	}
*/	else if (cell.x == ProcessList::kTreeNice)
	{
		str	= JString((JUInt64) entry.GetNice());
	}
	else if (cell.x == ProcessList::kTreeSize)
	{
		str	= JString((JUInt64) entry.GetSize());
	}
/*	else if (cell.x == ProcessList::kTreeResident)
	{
		str	= JString((JUInt64) entry.GetResident());
	}
	else if (cell.x == ProcessList::kTreeShare)
	{
		str	= JString((JUInt64) entry.GetShare());
	}
*/	else if (cell.x == ProcessList::kTreeCPU)
	{
		str	= JString(entry.GetPercentCPU(), 1);
	}
	else if (cell.x == ProcessList::kTreeMemory)
	{
		str	= JString(entry.GetPercentMemory(), 1);
	}
	else if (cell.x == ProcessList::kTreeTime)
	{
		str	= JString((JUInt64) entry.GetTime());
	}

	JRect r  = rect;
	r.left  += kHMarginWidth;
	r.right -= kHMarginWidth;
	p.JPainter::String(r, str, halign, JPainter::VAlign::kCenter);
}

/******************************************************************************
 HandleMouseDown (virtual protected)

 ******************************************************************************/

void
ProcessTreeList::HandleMouseDown
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

	JPoint cell;
	if (!GetCell(pt, &cell))
	{
		return;
	}

	s.SelectRow(cell.y);

	if (cell.x == ProcessList::kTreeState)
	{
		const ProcessEntry* entry;
		const bool ok = GetSelectedProcess(&entry);
		assert( ok );

		ProcessTable::ToggleProcessState(*entry);
		itsList->Update();
	}
	else if (cell.x != ProcessList::kTreeOpenClose &&
			 button == kJXRightButton)
	{
		itsContextMenu->PopUp(this, pt, buttonStates, modifiers);
	}
	else
	{
		JXNamedTreeListWidget::HandleMouseDown(pt, button, clickCount, buttonStates, modifiers);
	}
}

/******************************************************************************
 UpdateContextMenu (private)

 ******************************************************************************/

void
ProcessTreeList::UpdateContextMenu()
{
	const ProcessEntry* entry;
	if (GetSelectedProcess(&entry))
	{
		ProcessTable::UpdateContextMenu(itsContextMenu, *entry);
	}
}

/******************************************************************************
 HandleContextMenu (private)

 ******************************************************************************/

void
ProcessTreeList::HandleContextMenu
	(
	const JIndex index
	)
{
	const ProcessEntry* entry;
	if (GetSelectedProcess(&entry))
	{
		ProcessTable::HandleContextMenu(index, *entry, itsList);
	}
}

/******************************************************************************
 GetSelectedProcess

 ******************************************************************************/

bool
ProcessTreeList::GetSelectedProcess
	(
	const ProcessEntry** entry
	)
	const
{
	JPtrArray<JTreeNode> list(JPtrArrayT::kForgetAll);
	GetSelectedNodes(&list);

	if (!list.IsEmpty())
	{
		*entry = dynamic_cast<ProcessEntry*>(list.GetFirstElement());
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
ProcessTreeList::SelectProcess
	(
	const ProcessEntry& entry
	)
{
	JTableSelection& s = GetTableSelection();
	s.ClearSelection();

	GetTreeList()->MakeVisible(&entry);

	JIndex index;
	if (GetTreeList()->FindNode(&entry, &index))
	{
		s.SelectRow(index);
		ScrollToNode(&entry);
	}
}
