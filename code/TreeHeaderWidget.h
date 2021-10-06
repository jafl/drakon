/******************************************************************************
 TreeHeaderWidget.h

	Copyright (C) 2006 by John Lindal.

 *****************************************************************************/

#ifndef _H_TreeHeaderWidget
#define _H_TreeHeaderWidget

#include <jx-af/jx/JXColHeaderWidget.h>

class ProcessList;

class TreeHeaderWidget : public JXColHeaderWidget
{
public:

	TreeHeaderWidget(JXTable* table, ProcessList* list,
						JXScrollbarSet* scrollbarSet, JXContainer* enclosure,
						const HSizingOption hSizing, const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h);

	virtual ~TreeHeaderWidget();

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
