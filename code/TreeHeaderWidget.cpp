/******************************************************************************
 TreeHeaderWidget.cpp

	BASE CLASS = JXColHeaderWidget

	Copyright (C) 2006 by John Lindal.

 *****************************************************************************/

#include "TreeHeaderWidget.h"
#include "ProcessList.h"
#include "globals.h"
#include <jx-af/jx/jXPainterUtil.h>
#include <jx-af/jx/jXConstants.h>
#include <jx-af/jcore/JPainter.h>
#include <jx-af/jcore/JFontManager.h>
#include <jx-af/jx/JXColorManager.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

TreeHeaderWidget::TreeHeaderWidget
	(
	JXTable*			table,
	ProcessList*		list,
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
	JXColHeaderWidget(table, scrollbarSet, enclosure, hSizing,vSizing, x,y, w,h),
	itsList(list)
{
	SetColTitle(1, JString::empty);
	SetColTitle(2, JGetString("CommandColumnTitle::ListHeaderWidget"));
	SetColTitle(3, JString::empty);
	SetColTitle(4, JGetString("PIDColumnTitle::ListHeaderWidget"));
	SetColTitle(5, JGetString("UserColumnTitle::ListHeaderWidget"));
	SetColTitle(6, JGetString("NiceColumnTitle::ListHeaderWidget"));
	SetColTitle(7, JGetString("SizeColumnTitle::ListHeaderWidget"));
	SetColTitle(8, JGetString("CPUColumnTitle::ListHeaderWidget"));
	SetColTitle(9, JGetString("MemoryColumnTite::ListHeaderWidget"));
	SetColTitle(10, JGetString("TimeColumnTitle::ListHeaderWidget"));
}

/******************************************************************************
 Destructor

 *****************************************************************************/

TreeHeaderWidget::~TreeHeaderWidget()
{
}

/******************************************************************************
 TableDrawCell (virtual protected)

 ******************************************************************************/

void
TreeHeaderWidget::TableDrawCell
	(
	JPainter&		p,
	const JPoint&	cell,
	const JRect&	rect
	)
{
	JXDrawUpFrame(p, rect, kJXDefaultBorderWidth);

	JString str;
	if (!GetColTitle(cell.x, &str))
	{
		str = JString((JUInt64) cell.x);
	}

	JSize underLines = 0;
	if (itsList->TreeColIsSelected(cell.x))
	{
		underLines = 1;
	}

	const JFont font = JFontManager::GetFont(
		JFontManager::GetDefaultFontName(), JFontManager::GetDefaultRowColHeaderFontSize(),
		JFontStyle(true, false, underLines, false, JColorManager::GetBlackColor()));
	p.SetFont(font);
	p.String(rect, str, JPainter::HAlign::kCenter, JPainter::VAlign::kCenter);
}

/******************************************************************************
 HandleMouseDown (virtual protected)

 ******************************************************************************/

void
TreeHeaderWidget::HandleMouseDown
	(
	const JPoint&			pt,
	const JXMouseButton		button,
	const JSize				clickCount,
	const JXButtonStates&	buttonStates,
	const JXKeyModifiers&	modifiers
	)
{
	JPoint cell;
	if (GetCell(pt, &cell))
	{
		itsList->TreeColSelected(cell.x);
		TableRefresh();
	}
}
