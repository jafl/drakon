/******************************************************************************
 ProcessTable.h

	Copyright (C) 2001 by Glenn W. Bach.

 *****************************************************************************/

#ifndef _H_ProcessTable
#define _H_ProcessTable

#include <jx-af/jx/JXTable.h>

class ProcessList;
class ProcessEntry;

class JXTextMenu;
class JXToolBar;
class JXTEBase;
class JXImage;

class ProcessTable : public JXTable
{
public:

	ProcessTable(ProcessList* list, JXTEBase* fullCmdDisplay,
					JXScrollbarSet* scrollbarSet, JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);

	virtual ~ProcessTable();

	bool	GetSelectedProcess(const ProcessEntry** entry) const;
	void	SelectProcess(const ProcessEntry& entry);

	virtual void	HandleKeyPress(const JUtf8Character& c,
								   const int keySym, const JXKeyModifiers& modifiers) override;

	static void	DrawRowBackground(JPainter& p, const JPoint& cell, const JRect& rect,
								  const JColorID color);
	static void	DrawProcessState(const ProcessEntry& entry,
								 JPainter& p, const JRect& rect,
								 const JXImage& zombieImage);

	static JXTextMenu*	CreateContextMenu(JXContainer* enclosure);
	static void			UpdateContextMenu(JXTextMenu* menu, const ProcessEntry& entry);
	static void			HandleContextMenu(const JIndex menuIndex, const ProcessEntry& entry,
										  ProcessList* list);
	static void			ToggleProcessState(const ProcessEntry& entry);

protected:

	virtual void	ApertureResized(const JCoordinate dw, const JCoordinate dh) override;
	virtual void	TableDrawCell(JPainter& p, const JPoint& cell, const JRect& rect) override;

	virtual void	HandleFocusEvent() override;
	virtual void	HandleMouseDown(const JPoint& pt, const JXMouseButton button,
									const JSize clickCount,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers) override;

	virtual void	Receive(JBroadcaster* sender, const Message& message) override;

private:

	ProcessList*			itsList;			// not owned
	JString					itsKeyBuffer;
	JXTextMenu*				itsContextMenu;
	const ProcessEntry* 	itsSelectedEntry;	// nullptr unless updating
	JXTEBase*				itsFullCmdDisplay;
	JXImage*				itsZombieImage;		// not owned

private:

	void	UpdateContextMenu();
	void	HandleContextMenu(const JIndex index);

	void	AdjustColWidths();
};

#endif
