/******************************************************************************
 ProcessEntry.cpp

	BASE CLASS = JNamedTreeNode

	Copyright (C) 2000 by Glenn W. Bach.

 *****************************************************************************/

#include "ProcessEntry.h"
#include "globals.h"
#include <jx-af/jcore/JTree.h>
#include <jx-af/jcore/JDirEntry.h>
#include <jx-af/jcore/JStringIterator.h>
#include <jx-af/jcore/jDirUtil.h>
#include <jx-af/jcore/jStreamUtil.h>
#include <jx-af/jcore/jFStreamUtil.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

#include <jx-af/jcore/jAssert.h>

/******************************************************************************
 Constructor

 *****************************************************************************/

ProcessEntry::ProcessEntry
	(
	JTree*				tree,
	const JDirEntry&	entry
	)
	:
	JNamedTreeNode(tree, JString::empty, false),
	itsLastUTime(0),
	itsLastSTime(0)
{
	itsProcPath = entry.GetFullName();
	itsUID      = entry.GetUserID();
	itsUser     = entry.GetUserName();
	
	JUInt value;
	entry.GetName().ConvertToUInt(&value);
	itsPID = value;
}

// search target

ProcessEntry::ProcessEntry
	(
	JTree*			tree,
	const JString&	prefix
	)
	:
	JNamedTreeNode(tree, prefix, false)
{
	itsCommand = prefix;
}

/******************************************************************************
 Destructor

 *****************************************************************************/

ProcessEntry::~ProcessEntry()
{
}

/******************************************************************************
 Update

 ******************************************************************************/

#if defined KERN_PROCARGS2
#include <mach/mach.h>
#include <mach/task.h>
kern_return_t task_for_pid(task_port_t task, pid_t pid, task_port_t *target);
#endif

void
ProcessEntry::Update
	(
	const JFloat elapsedTime
	)
{
	itsPercentMemory = 0;

	try
	{
		ReadStat();
		ReadStatM();

		JSize mem;
		if (GetSystemMemory(&mem))
		{
			itsPercentMemory = JFloat(itsResident * 100) / mem;
		}

		// shared across #if
		ReadCmdline();	// not in ctor, to make ctor faster
	}
	catch (...)
	{
		itsState = kZombie;
//		std::cerr << "failed to update: " << itsPID << std::endl;
	}

	SetName(itsCommand);
	ShouldBeOpenable(HasChildren());

	itsTime	        = (itsUTime + itsSTime) / sysconf(_SC_CLK_TCK);
	JSize totalTime	= (itsLastUTime == 0 || itsLastSTime == 0) ? 0 : (itsUTime - itsLastUTime) + (itsSTime - itsLastSTime);
	itsLastUTime	= itsUTime;
	itsLastSTime	= itsSTime;
	itsPercentCPU	= elapsedTime == 0 || itsState == kZombie ? 0 : JFloat(totalTime * 1000 / sysconf(_SC_CLK_TCK)) / (10 * elapsedTime);
}

/******************************************************************************
 ReadStat (private)

 ******************************************************************************/

void
ProcessEntry::ReadStat()
{
	const JSize uTime = itsUTime, sTime = itsSTime;

	JString str = JCombinePathAndName(itsProcPath, JString("stat", false));
	std::ifstream is(str.GetBytes());
	if (is.good())
	{
		is >> itsPID;
		is >> std::ws;
		itsCommand = JReadUntilws(is);
		if (itsCommand.GetCharacterCount() > 2)
		{
			JStringIterator iter(&itsCommand);
			iter.RemoveNext();
			iter.MoveTo(JStringIterator::kStartAtEnd, 0);
			iter.RemovePrev();
		}
		JString state = JReadUntilws(is);
		if (state.Contains("S"))
		{
			itsState = kSleep;
		}
		else if (state.Contains("D"))
		{
			itsState = kUnIntSleep;
		}
		else if (state.Contains("Z"))
		{
			itsState = kZombie;
		}
		else if (state.Contains("T"))
		{
			itsState = kStopped;
		}
		else
		{
			itsState = kRun;
		}
		is >> itsPPID;
		is >> std::ws;
		int toss;
		is >> toss;
		is >> toss;
		is >> toss;
		is >> toss;
		is >> toss;
		is >> toss;
		is >> toss;
		is >> toss;
		is >> toss;
		is >> itsUTime;
		is >> itsSTime;
		is >> toss;
		is >> toss;
		is >> itsPriority;
		is >> itsNice;
	}

	if (!is.good())
	{
		itsState = kZombie;
		itsUTime = uTime;
		itsSTime = sTime;
		itsPriority = itsNice = 0;
	}
}

/******************************************************************************
 ReadStatM (private)

 ******************************************************************************/

void
ProcessEntry::ReadStatM()
{

	JString str = JCombinePathAndName(itsProcPath, JString("statm", false));
	std::ifstream is(str.GetBytes());
	if (is.good())
	{
		is >> itsSize >> itsResident >> itsShare;
		itsSize	    *= 4;
		itsResident *= 4;
		itsShare    *= 4;
	}

	if (!is.good())
	{
		itsState = kZombie;
		itsSize = itsResident = itsShare = 0;
	}
}

/******************************************************************************
 ReadCmdline (private)

 ******************************************************************************/

void
ProcessEntry::ReadCmdline()
{
	if (!itsFullCommand.IsEmpty())
	{
		return;
	}

	JString str = JCombinePathAndName(itsProcPath, JString("cmdline", false));
	std::ifstream is(str.GetBytes());
	if (is.good())
	{
		JString cmdline;
		JReadAll(is, &cmdline);
		cmdline.TrimWhitespace();
		if (cmdline.IsEmpty())
		{
			return;
		}

		JStringIterator iter(&cmdline);
		JUtf8Character c;
		while (iter.Next(&c))
		{
			if (c == '\0')
			{
				iter.SetPrev(JUtf8Character(' '), JStringIterator::kStay);
			}
		}

		itsFullCommand = cmdline;
	}
}

/******************************************************************************
 List comparison (static)

 ******************************************************************************/

std::weak_ordering
ProcessEntry::CompareListPID
	(
	ProcessEntry * const & e1,
	ProcessEntry * const & e2
	)
{
	return e1->itsPID <=> e2->itsPID;
}

std::weak_ordering
ProcessEntry::CompareListUser
	(
	ProcessEntry * const & e1,
	ProcessEntry * const & e2
	)
{
	auto r = JCompareStringsCaseInsensitive(&(e1->itsUser), &(e2->itsUser));
	if (r == std::weak_ordering::equivalent)
	{
		r = CompareListPID(e1, e2);
	}
	return r;
}

std::weak_ordering
ProcessEntry::CompareListNice
	(
	ProcessEntry * const & e1,
	ProcessEntry * const & e2
	)
{
	std::weak_ordering r = e1->itsNice <=> e2->itsNice;
	if (r == std::weak_ordering::equivalent)
	{
		r = CompareListPID(e1, e2);
	}
	return r;
}

std::weak_ordering
ProcessEntry::CompareListSize
	(
	ProcessEntry * const & e1,
	ProcessEntry * const & e2
	)
{
	std::weak_ordering r = e1->itsSize <=> e2->itsSize;
	if (r == std::weak_ordering::equivalent)
	{
		r = CompareListPID(e1, e2);
	}
	return r;
}

std::weak_ordering
ProcessEntry::CompareListPercentMemory
	(
	ProcessEntry * const & e1,
	ProcessEntry * const & e2
	)
{
	if (e1->itsPercentMemory > e2->itsPercentMemory)
	{
		return std::weak_ordering::greater;
	}
	else if (e1->itsPercentMemory < e2->itsPercentMemory)
	{
		return std::weak_ordering::less;
	}
	else
	{
		return CompareListPID(e1, e2);
	}
}

std::weak_ordering
ProcessEntry::CompareListPercentCPU
	(
	ProcessEntry * const & e1,
	ProcessEntry * const & e2
	)
{
	if (e1->itsPercentCPU > e2->itsPercentCPU)
	{
		return std::weak_ordering::greater;
	}
	else if (e1->itsPercentCPU < e2->itsPercentCPU)
	{
		return std::weak_ordering::less;
	}
	else
	{
		return CompareListPID(e1, e2);
	}
}

std::weak_ordering
ProcessEntry::CompareListTime
	(
	ProcessEntry * const & e1,
	ProcessEntry * const & e2
	)
{
	std::weak_ordering r = e1->itsTime <=> e2->itsTime;
	if (r == std::weak_ordering::equivalent)
	{
		r = CompareListPID(e1, e2);
	}
	return r;
}

std::weak_ordering
ProcessEntry::CompareListCommand
	(
	ProcessEntry * const & e1,
	ProcessEntry * const & e2
	)
{
	auto r = JCompareStringsCaseInsensitive(&e1->itsCommand, &e2->itsCommand);
	if (r == std::weak_ordering::equivalent)
	{
		r = CompareListPID(e1, e2);
	}
	return r;
}

std::weak_ordering
ProcessEntry::CompareListCommandForIncrSearch
	(
	ProcessEntry * const & e1,
	ProcessEntry * const & e2
	)
{
	return JCompareStringsCaseInsensitive(&e1->itsCommand, &e2->itsCommand);
}

/******************************************************************************
 Tree comparison (static)

 ******************************************************************************/

std::weak_ordering
ProcessEntry::CompareTreePID
	(
	JTreeNode * const & n1,
	JTreeNode * const & n2
	)
{
	auto * const e1 = dynamic_cast<ProcessEntry*const>(n1);
	auto * const e2 = dynamic_cast<ProcessEntry*const>(n2);
	return CompareListPID(e1, e2);
}

std::weak_ordering
ProcessEntry::CompareTreeUser
	(
	JTreeNode * const & n1,
	JTreeNode * const & n2
	)
{
	auto * const e1 = dynamic_cast<ProcessEntry*const>(n1);
	auto * const e2 = dynamic_cast<ProcessEntry*const>(n2);
	return CompareListUser(e1, e2);
}

std::weak_ordering
ProcessEntry::CompareTreeNice
	(
	JTreeNode * const & n1,
	JTreeNode * const & n2
	)
{
	auto * const e1 = dynamic_cast<ProcessEntry*const>(n1);
	auto * const e2 = dynamic_cast<ProcessEntry*const>(n2);
	return CompareListNice(e1, e2);
}

std::weak_ordering
ProcessEntry::CompareTreeSize
	(
	JTreeNode * const & n1,
	JTreeNode * const & n2
	)
{
	auto * const e1 = dynamic_cast<ProcessEntry*const>(n1);
	auto * const e2 = dynamic_cast<ProcessEntry*const>(n2);
	return CompareListSize(e1, e2);
}

std::weak_ordering
ProcessEntry::CompareTreePercentMemory
	(
	JTreeNode * const & n1,
	JTreeNode * const & n2
	)
{
	auto * const e1 = dynamic_cast<ProcessEntry*const>(n1);
	auto * const e2 = dynamic_cast<ProcessEntry*const>(n2);
	return CompareListPercentMemory(e1, e2);
}

std::weak_ordering
ProcessEntry::CompareTreePercentCPU
	(
	JTreeNode * const & n1,
	JTreeNode * const & n2
	)
{
	auto * const e1 = dynamic_cast<ProcessEntry*const>(n1);
	auto * const e2 = dynamic_cast<ProcessEntry*const>(n2);
	return CompareListPercentCPU(e1, e2);
}

std::weak_ordering
ProcessEntry::CompareTreeTime
	(
	JTreeNode * const & n1,
	JTreeNode * const & n2
	)
{
	auto * const e1 = dynamic_cast<ProcessEntry*const>(n1);
	auto * const e2 = dynamic_cast<ProcessEntry*const>(n2);
	return CompareListTime(e1, e2);
}

std::weak_ordering
ProcessEntry::CompareTreeCommand
	(
	JTreeNode * const & n1,
	JTreeNode * const & n2
	)
{
	auto * const e1 = dynamic_cast<ProcessEntry*const>(n1);
	auto * const e2 = dynamic_cast<ProcessEntry*const>(n2);
	return CompareListCommand(e1, e2);
}
