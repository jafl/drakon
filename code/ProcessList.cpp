/******************************************************************************
 ProcessList.cpp

	BASE CLASS = JContainer

	Copyright (C) 2000 by Glenn W. Bach.

 *****************************************************************************/

#include "ProcessList.h"
#include "ProcessEntry.h"
#include <jx-af/jcore/JTree.h>
#include <jx-af/jcore/JDirInfo.h>
#include <jx-af/jcore/jStreamUtil.h>
#include <jx-af/jcore/jTime.h>
#include <jx-af/jcore/jSysUtil.h>
#include <jx-af/jcore/jFStreamUtil.h>
#include <sys/time.h>
#include <unistd.h>
#include <jx-af/jcore/jAssert.h>

const JUtf8Byte* ProcessList::kListChanged      = "ListChanged::ProcessList";
const JUtf8Byte* ProcessList::kPrepareForUpdate = "PrepareForUpdate::ProcessList";

/******************************************************************************
 Constructor

 *****************************************************************************/

ProcessList::ProcessList()
	:
	itsElapsedTime(0),
	itsLastTime(0),
	itsIsShowingUserOnly(true),
	itsUID(getuid()),
	itsDirInfo(nullptr)
{
	itsVisibleEntries = jnew JPtrArray<ProcessEntry>(JPtrArrayT::kForgetAll);
	itsVisibleEntries->SetCompareFunction(ProcessEntry::CompareListPID);
	InstallCollection(itsVisibleEntries);
	itsListColType = kListPID;
	itsTreeColType = kTreeCommand;

	itsAlphaEntries = jnew JPtrArray<ProcessEntry>(JPtrArrayT::kForgetAll);
	itsAlphaEntries->SetCompareFunction(ProcessEntry::CompareListCommandForIncrSearch);

	itsHiddenEntries = jnew JPtrArray<ProcessEntry>(JPtrArrayT::kDeleteAll);
	itsHiddenEntries->SetCompareFunction(ProcessEntry::CompareListPID);

	itsRootNode = jnew JTreeNode(nullptr);
	itsRootNode->SetChildCompareFunction(ProcessEntry::CompareTreeCommand,
										 JListT::kSortAscending, true);

	itsTree = jnew JTree(itsRootNode);

	Update();
}

/******************************************************************************
 Destructor

 *****************************************************************************/

ProcessList::~ProcessList()
{
	jdelete itsHiddenEntries;
	jdelete itsVisibleEntries;
	jdelete itsAlphaEntries;
	jdelete itsTree;
	jdelete itsDirInfo;
}

/******************************************************************************
 ShouldShowUserOnly

 ******************************************************************************/

void
ProcessList::ShouldShowUserOnly
	(
	const bool show
	)
{
	if (itsIsShowingUserOnly != show)
	{
		itsIsShowingUserOnly = show;
		Update();
	}
}

/******************************************************************************
 Update

 ******************************************************************************/

void
ProcessList::Update()
{
	Broadcast(PrepareForUpdate());
	
	struct timeval currentTime;
	struct timezone tZone;
	gettimeofday(&currentTime, &tZone);

	const JFloat newTime = currentTime.tv_sec + (currentTime.tv_usec / 1000000.0);
	itsElapsedTime       = itsLastTime == 0 ? 0 : newTime - itsLastTime;
	itsLastTime          = newTime;

	JPtrArray<ProcessEntry> newEntries(JPtrArrayT::kForgetAll);
	newEntries.SetCompareFunction(ProcessEntry::CompareListPID);

	if (itsDirInfo == nullptr)
	{
		bool ok	= JDirInfo::Create(JString("/proc", false), &itsDirInfo);
		assert(ok);
	}
	else
	{
		itsDirInfo->ForceUpdate();
	}

	for (const auto* entry : *itsDirInfo)
	{
		if (entry->GetName().IsInteger())
		{
			auto* pentry = jnew ProcessEntry(itsTree, *entry);
			newEntries.InsertSorted(pentry);
		}
	}

	if (!itsIsShowingUserOnly)
	{
		itsHiddenEntries->CleanOut();
	}

	// remove dead processes from the hidden list

	JSize count = itsHiddenEntries->GetItemCount();
	for (JIndex i=count; i>=1; i--)
	{
		auto* pentry = itsHiddenEntries->GetItem(i);
		JIndex findex;
		if (!newEntries.SearchSorted(pentry, JListT::kAnyMatch, &findex))
		{
			itsHiddenEntries->DeleteItem(i);
		}
	}

	// hide processes from other users, if necessary

	if (itsIsShowingUserOnly)
	{
		count = newEntries.GetItemCount();
		for (JIndex i=count; i>=1; i--)
		{
			auto* pentry = newEntries.GetItem(i);
			if (pentry->GetUID() != itsUID)
			{
				newEntries.RemoveItem(i);

				JIndex findex;
				if (itsHiddenEntries->SearchSorted(pentry, JListT::kAnyMatch, &findex))
				{
					jdelete pentry;
				}
				else
				{
					itsHiddenEntries->InsertSorted(pentry);
				}
			}
		}

		for (auto* pentry : *itsHiddenEntries)
		{
			pentry->Update(itsElapsedTime);
		}
	}

	// remove dead processes from the visible list

	count = itsVisibleEntries->GetItemCount();
	for (JIndex i=count; i>=1; i--)
	{
		auto* pentry = itsVisibleEntries->GetItem(i);
		JIndex findex;
		if (newEntries.SearchSorted(pentry, JListT::kAnyMatch, &findex))
		{
			newEntries.DeleteItem(findex);
		}
		else
		{
			while (pentry->HasChildren())
			{
				itsRootNode->InsertSorted(pentry->GetChild(1));
			}

			itsAlphaEntries->Remove(pentry);
			itsVisibleEntries->DeleteItem(i);
		}
	}

	// update information on all pre-existing processes

	for (auto* pentry : *itsVisibleEntries)
	{
		pentry->Update(itsElapsedTime);
	}

	itsVisibleEntries->Sort();
	itsRootNode->SortChildren(true);

	// add new processes to the list

	std::function<std::weak_ordering(JTreeNode * const &, JTreeNode * const &)>* treeCompareFn;
	JListT::SortOrder treeSortOrder;
	itsRootNode->GetChildCompareFunction(&treeCompareFn, &treeSortOrder);

	for (auto* pentry : newEntries)
	{
		pentry->Update(itsElapsedTime);
		itsVisibleEntries->InsertSorted(pentry);
		itsAlphaEntries->InsertSorted(pentry);

		pentry->SetChildCompareFunction(*treeCompareFn, treeSortOrder, true);
	}

	// reparent all nodes

	for (auto* pentry : *itsVisibleEntries)
	{
		ProcessEntry* parent;
		if (FindProcessEntry(pentry->GetPPID(), &parent) &&
			parent != pentry)
		{
			parent->InsertSorted(pentry);
		}
		else
		{
			itsRootNode->InsertSorted(pentry);
		}
	}

	Broadcast(ListChanged());
}

/******************************************************************************
 FindProcessEntry

 ******************************************************************************/

bool
ProcessList::FindProcessEntry
	(
	const pid_t		pid,
	ProcessEntry**	entry
	)
	const
{
	const JSize count = itsVisibleEntries->GetItemCount();
	for (JIndex i=1; i<=count; i++)
	{
		ProcessEntry* e = itsVisibleEntries->GetItem(i);
		if (e->GetPID() == pid)
		{
			*entry = e;
			return true;
		}
	}

	*entry = nullptr;
	return false;
}

/******************************************************************************
 ClosestMatch

 ******************************************************************************/

bool
ProcessList::ClosestMatch
	(
	const JString&		prefix,
	ProcessEntry**	entry
	)
	const
{
	ProcessEntry target(itsTree, prefix);
	bool found;
	JIndex i = itsAlphaEntries->SearchSortedOTI(&target, JListT::kFirstMatch, &found);
	if (i > itsAlphaEntries->GetItemCount())		// insert beyond end of list
	{
		i = itsAlphaEntries->GetItemCount();
	}

	if (i > 0)
	{
		*entry = itsAlphaEntries->GetItem(i);
		return true;
	}
	else
	{
		*entry = nullptr;
		return false;
	}
}

/******************************************************************************
 ListColSelected

 ******************************************************************************/

void
ProcessList::ListColSelected
	(
	const JIndex index
	)
{
	bool changed = false;

	if (index == kListPID)
	{
		itsVisibleEntries->SetCompareFunction(ProcessEntry::CompareListPID);
		itsVisibleEntries->SetSortOrder(JListT::kSortAscending);
		changed = true;
	}
	else if (index == kListUser)
	{
		itsVisibleEntries->SetCompareFunction(ProcessEntry::CompareListUser);
		itsVisibleEntries->SetSortOrder(JListT::kSortAscending);
		changed = true;
	}
	else if (index == kListNice)
	{
		itsVisibleEntries->SetCompareFunction(ProcessEntry::CompareListNice);
		itsVisibleEntries->SetSortOrder(JListT::kSortAscending);
		changed = true;
	}
	else if (index == kListSize)
	{
		itsVisibleEntries->SetCompareFunction(ProcessEntry::CompareListSize);
		itsVisibleEntries->SetSortOrder(JListT::kSortDescending);
		changed = true;
	}
	else if (index == kListCPU)
	{
		itsVisibleEntries->SetCompareFunction(ProcessEntry::CompareListPercentCPU);
		itsVisibleEntries->SetSortOrder(JListT::kSortDescending);
		changed = true;
	}
	else if (index == kListMemory)
	{
		itsVisibleEntries->SetCompareFunction(ProcessEntry::CompareListPercentMemory);
		itsVisibleEntries->SetSortOrder(JListT::kSortDescending);
		changed = true;
	}
	else if (index == kListTime)
	{
		itsVisibleEntries->SetCompareFunction(ProcessEntry::CompareListTime);
		itsVisibleEntries->SetSortOrder(JListT::kSortDescending);
		changed = true;
	}
	else if (index == kListCommand)
	{
		itsVisibleEntries->SetCompareFunction(ProcessEntry::CompareListCommand);
		itsVisibleEntries->SetSortOrder(JListT::kSortAscending);
		changed = true;
	}

	if (changed)
	{
		Broadcast(PrepareForUpdate());
		itsVisibleEntries->Sort();
		itsListColType = (ListColType) index;
		Broadcast(ListChanged());
	}
}

/******************************************************************************
 TreeColSelected

 ******************************************************************************/

void
ProcessList::TreeColSelected
	(
	const JIndex index
	)
{
	if (index == kTreePID)
	{
		Broadcast(PrepareForUpdate());
		itsRootNode->SetChildCompareFunction(ProcessEntry::CompareTreePID,
											 JListT::kSortAscending, true);
		itsTreeColType = (TreeColType) index;
		Broadcast(ListChanged());
	}
	else if (index == kTreeUser)
	{
		Broadcast(PrepareForUpdate());
		itsRootNode->SetChildCompareFunction(ProcessEntry::CompareTreeUser,
											 JListT::kSortAscending, true);
		itsTreeColType = (TreeColType) index;
		Broadcast(ListChanged());
	}
	else if (index == kTreeNice)
	{
		Broadcast(PrepareForUpdate());
		itsRootNode->SetChildCompareFunction(ProcessEntry::CompareTreeNice,
											 JListT::kSortAscending, true);
		itsTreeColType = (TreeColType) index;
		Broadcast(ListChanged());
	}
	else if (index == kTreeSize)
	{
		Broadcast(PrepareForUpdate());
		itsRootNode->SetChildCompareFunction(ProcessEntry::CompareTreeSize,
											 JListT::kSortDescending, true);
		itsTreeColType = (TreeColType) index;
		Broadcast(ListChanged());
	}
	else if (index == kTreeCPU)
	{
		Broadcast(PrepareForUpdate());
		itsRootNode->SetChildCompareFunction(ProcessEntry::CompareTreePercentCPU,
											 JListT::kSortDescending, true);
		itsTreeColType = (TreeColType) index;
		Broadcast(ListChanged());
	}
	else if (index == kTreeMemory)
	{
		Broadcast(PrepareForUpdate());
		itsRootNode->SetChildCompareFunction(ProcessEntry::CompareTreePercentMemory,
											 JListT::kSortDescending, true);
		itsTreeColType = (TreeColType) index;
		Broadcast(ListChanged());
	}
	else if (index == kTreeTime)
	{
		Broadcast(PrepareForUpdate());
		itsRootNode->SetChildCompareFunction(ProcessEntry::CompareTreeTime,
											 JListT::kSortDescending, true);
		itsTreeColType = (TreeColType) index;
		Broadcast(ListChanged());
	}
	else if (index == kTreeCommand)
	{
		Broadcast(PrepareForUpdate());
		itsRootNode->SetChildCompareFunction(ProcessEntry::CompareTreeCommand,
											 JListT::kSortAscending, true);
		itsTreeColType = (TreeColType) index;
		Broadcast(ListChanged());
	}
}
