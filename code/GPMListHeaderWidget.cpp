/******************************************************************************
 GPMListHeaderWidget.cpp

	BASE CLASS = JXColHeaderWidget

	Copyright (C) 2000 by Glenn W. Bach.

 *****************************************************************************/

#include "GPMListHeaderWidget.h"
#include "GPMProcessList.h"
#include "gpmGlobals.h"
#include <jx-af/jx/jXPainterUtil.h>
#include <jx-af/jx/jXConstants.h>
#include <jx-af/jcore/JPainter.h>
#include <jx-af/jcore/JFontManager.h>
#include <jx-af/jx/JXColorManager.h>
#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 ******************************************************************************/

GPMListHeaderWidget::GPMListHeaderWidget
	(
	JXTable*			table,
	GPMProcessList*		list,
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
	SetColTitle(2, JGetString("PIDColumnTitle::GPMListHeaderWidget"));
	SetColTitle(3, JGetString("UserColumnTitle::GPMListHeaderWidget"));
	SetColTitle(4, JGetString("NiceColumnTitle::GPMListHeaderWidget"));
	SetColTitle(5, JGetString("SizeColumnTitle::GPMListHeaderWidget"));
	SetColTitle(6, JGetString("CPUColumnTitle::GPMListHeaderWidget"));
	SetColTitle(7, JGetString("MemoryColumnTite::GPMListHeaderWidget"));
	SetColTitle(8, JGetString("TimeColumnTitle::GPMListHeaderWidget"));
	SetColTitle(9, JGetString("CommandColumnTitle::GPMListHeaderWidget"));
}

/******************************************************************************
 Destructor

 *****************************************************************************/

GPMListHeaderWidget::~GPMListHeaderWidget()
{
}

/******************************************************************************
 TableDrawCell (virtual protected)

 ******************************************************************************/

void
GPMListHeaderWidget::TableDrawCell
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
	if (itsList->ListColIsSelected(cell.x))
	{
		underLines = 1;
	}

	const JFont font = JFontManager::GetFont(
		JFontManager::GetDefaultFontName(), JFontManager::GetDefaultRowColHeaderFontSize(),
		JFontStyle(true, false, underLines, false, JColorManager::GetBlackColor()));
	p.SetFont(font);
	p.String(rect, str, JPainter::kHAlignCenter, JPainter::kVAlignCenter);
}

/******************************************************************************
 HandleMouseDown (virtual protected)

 ******************************************************************************/

void
GPMListHeaderWidget::HandleMouseDown
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
