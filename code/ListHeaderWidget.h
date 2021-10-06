/******************************************************************************
 ListHeaderWidget.h

	Copyright (C) 2000 by Glenn W. Bach.

 *****************************************************************************/

#ifndef _H_ListHeaderWidget
#define _H_ListHeaderWidget

#include <jx-af/jx/JXColHeaderWidget.h>

class ProcessList;

class ListHeaderWidget : public JXColHeaderWidget
{
public:

	ListHeaderWidget(JXTable* table, ProcessList* list,
						JXScrollbarSet* scrollbarSet, JXContainer* enclosure,
						const HSizingOption hSizing, const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h);

	virtual ~ListHeaderWidget();

protected:

	void	TableDrawCell(JPainter& p, const JPoint& cell, const JRect& rect) override;

	void	HandleMouseDown(const JPoint& pt, const JXMouseButton button,
									const JSize clickCount,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers) override;

private:

	ProcessList*	itsList;
};

#endif
