/******************************************************************************
 ProcessTreeList.h

	Copyright (C) 2006 by John Lindal.

 *****************************************************************************/

#ifndef _H_ProcessTreeList
#define _H_ProcessTreeList

#include <jx-af/jx/JXNamedTreeListWidget.h>

class ProcessList;
class ProcessEntry;

class JXTextMenu;
class JXToolBar;
class JXTEBase;
class JXImage;

class ProcessTreeList : public JXNamedTreeListWidget
{
public:

	ProcessTreeList(ProcessList* list, JNamedTreeList* treeList,
						JXTEBase* fullCmdDisplay,
						JXScrollbarSet* scrollbarSet, JXContainer* enclosure,
						const HSizingOption hSizing, const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h);

	virtual ~ProcessTreeList();

	bool	GetSelectedProcess(const ProcessEntry** entry) const;
	void	SelectProcess(const ProcessEntry& entry);

protected:

	void	TableDrawCell(JPainter& p, const JPoint& cell, const JRect& rect) override;

	void	HandleMouseDown(const JPoint& pt, const JXMouseButton button,
									const JSize clickCount,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers) override;

	void	Receive(JBroadcaster* sender, const Message& message) override;

private:

	ProcessList*			itsList;			// not owned
	JXTextMenu*				itsContextMenu;
	const ProcessEntry* 	itsSelectedEntry;
	JXTEBase*				itsFullCmdDisplay;
	JXImage*				itsZombieImage;		// not owned

private:

	void	UpdateContextMenu();
	void	HandleContextMenu(const JIndex index);
};

#endif
