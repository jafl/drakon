/******************************************************************************
 ListHeaderWidget.cpp

	BASE CLASS = JXColHeaderWidget

	Copyright (C) 2000 by Glenn W. Bach.

 *****************************************************************************/

#include "ListHeaderWidget.h"
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

ListHeaderWidget::ListHeaderWidget
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
	SetColTitle(2, JGetString("PIDColumnTitle::ListHeaderWidget"));
	SetColTitle(3, JGetString("UserColumnTitle::ListHeaderWidget"));
	SetColTitle(4, JGetString("NiceColumnTitle::ListHeaderWidget"));
	SetColTitle(5, JGetString("SizeColumnTitle::ListHeaderWidget"));
	SetColTitle(6, JGetString("CPUColumnTitle::ListHeaderWidget"));
	SetColTitle(7, JGetString("MemoryColumnTite::ListHeaderWidget"));
	SetColTitle(8, JGetString("TimeColumnTitle::ListHeaderWidget"));
	SetColTitle(9, JGetString("CommandColumnTitle::ListHeaderWidget"));
}

/******************************************************************************
 Destructor

 *****************************************************************************/

ListHeaderWidget::~ListHeaderWidget()
{
}

/******************************************************************************
 TableDrawCell (virtual protected)

 ******************************************************************************/

void
ListHeaderWidget::TableDrawCell
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
		str = JString(cell.x);
	}

	JSize underLines = 0;
	if (itsList->ListColIsSelected(cell.x))
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
ListHeaderWidget::HandleMouseDown
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
		itsList->ListColSelected(cell.x);
		TableRefresh();
	}
}
