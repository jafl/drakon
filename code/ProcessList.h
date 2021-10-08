/******************************************************************************
 ProcessList.h

	Copyright (C) 2000 by Glenn W. Bach.

 *****************************************************************************/

#ifndef _H_ProcessList
#define _H_ProcessList

#include <jx-af/jcore/JContainer.h>
#include <jx-af/jcore/JPtrArray-JString.h>
#include "ProcessEntry.h"

class JTree;
class JTreeNode;
class JDirInfo;

class ProcessList : public JContainer
{
public:

	// don't forget to update ListHeaderWidget

	enum ListColType
	{
		kListState = 1,
		kListPID,
		kListUser,
//		kListPPID,
//		kListPriority,
		kListNice,
		kListSize,
//		kListResident,
//		kListShare,
		kListCPU,
		kListMemory,
		kListTime,
		kListCommand,

		kListCount = kListCommand
	};

	// don't forget to update TreeHeaderWidget

	enum TreeColType
	{
		kTreeOpenClose = 1,
		kTreeCommand,
		kTreeState,
		kTreePID,
		kTreeUser,
//		kTreePPID,
//		kTreePriority,
		kTreeNice,
		kTreeSize,
//		kTreeResident,
//		kTreeShare,
		kTreeCPU,
		kTreeMemory,
		kTreeTime,

		kTreeCount = kTreeTime
	};

public:

	ProcessList();
	~ProcessList() override;

	void	Update();

	const ProcessEntry*	GetProcessEntry(const JIndex index) const;
	bool				GetEntryIndex(const ProcessEntry* entry, JIndex *index);
	bool				FindProcessEntry(const pid_t pid, ProcessEntry** entry) const;
	bool				ClosestMatch(const JString& prefix, ProcessEntry** entry) const;

	const JPtrArray<ProcessEntry>&	GetHiddenProcesses() const;

	bool		ListColIsSelected(const JIndex index) const;
	ListColType	GetSelectedListCol() const;
	void		ListColSelected(const JIndex index);

	bool		TreeColIsSelected(const JIndex index) const;
	TreeColType	GetSelectedTreeCol() const;
	void		TreeColSelected(const JIndex index);

	JTree*		GetProcessTree();

	bool	WillShowUserOnly() const;
	void	ShouldShowUserOnly(const bool show);

private:

	JPtrArray<ProcessEntry>*	itsVisibleEntries;
	JPtrArray<ProcessEntry>*	itsAlphaEntries;
	JPtrArray<ProcessEntry>*	itsHiddenEntries;
	JTree*						itsTree;
	JTreeNode*					itsRootNode;
	JFloat						itsElapsedTime;
	JFloat						itsLastTime;
	ListColType					itsListColType;
	TreeColType					itsTreeColType;
	bool						itsIsShowingUserOnly;
	const JIndex				itsUID;

	#ifdef _J_HAS_PROC
	JDirInfo*					itsDirInfo;
	#endif

private:

	// not allowed

	ProcessList(const ProcessList&) = delete;
	ProcessList& operator=(const ProcessList&) = delete;

public:

	static const JUtf8Byte* kListChanged;
	static const JUtf8Byte* kPrepareForUpdate;

	class ListChanged : public JBroadcaster::Message
		{
		public:

			ListChanged()
				:
				JBroadcaster::Message(kListChanged)
				{ };
		};

	class PrepareForUpdate : public JBroadcaster::Message
		{
		public:

			PrepareForUpdate()
				:
				JBroadcaster::Message(kPrepareForUpdate)
				{ };
		};
};


/******************************************************************************
 ListColIsSelected

 ******************************************************************************/

inline bool
ProcessList::ListColIsSelected
	(
	const JIndex index
	)
	const
{
	return index == (JIndex) itsListColType;
}

/******************************************************************************
 GetSelectedListCol

 ******************************************************************************/

inline ProcessList::ListColType
ProcessList::GetSelectedListCol()
	const
{
	return itsListColType;
}

/******************************************************************************
 TreeColIsSelected

 ******************************************************************************/

inline bool
ProcessList::TreeColIsSelected
	(
	const JIndex index
	)
	const
{
	return index == (JIndex) itsTreeColType;
}

/******************************************************************************
 GetSelectedTreeCol

 ******************************************************************************/

inline ProcessList::TreeColType
ProcessList::GetSelectedTreeCol()
	const
{
	return itsTreeColType;
}

/******************************************************************************
 GetProcessTree

 ******************************************************************************/

inline JTree*
ProcessList::GetProcessTree()
{
	return itsTree;
}

/******************************************************************************
 WillShowUserOnly

 ******************************************************************************/

inline bool
ProcessList::WillShowUserOnly()
	const
{
	return itsIsShowingUserOnly;
}

/******************************************************************************
 GetProcessEntry

 ******************************************************************************/

inline const ProcessEntry*
ProcessList::GetProcessEntry
	(
	const JIndex index
	)
	const
{
	return itsVisibleEntries->GetElement(index);
}

/******************************************************************************
 GetEntryIndex

 ******************************************************************************/

inline bool
ProcessList::GetEntryIndex
	(
	const ProcessEntry*	entry,
	JIndex*					index
	)
{
	return itsVisibleEntries->SearchSorted(const_cast<ProcessEntry*>(entry), JListT::kAnyMatch, index);
}

/******************************************************************************
 GetHiddenProcesses

 ******************************************************************************/

inline const JPtrArray<ProcessEntry>&
ProcessList::GetHiddenProcesses()
	const
{
	return *itsHiddenEntries;
}

#endif
