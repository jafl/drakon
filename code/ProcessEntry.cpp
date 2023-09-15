/******************************************************************************
 ProcessEntry.cpp

	BASE CLASS = JNamedTreeNode

	Copyright (C) 2000 by Glenn W. Bach.

 *****************************************************************************/

#include "ProcessEntry.h"
#include "globals.h"

#include <jx-af/jcore/JTree.h>

#ifdef _J_HAS_PROC
#include <jx-af/jcore/JDirEntry.h>
#endif

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

#ifdef _J_HAS_PROC

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

#elif defined _J_HAS_SYSCTL

ProcessEntry::ProcessEntry
	(
	JTree*				tree,
	const kinfo_proc&	entry
	)
	:
	JNamedTreeNode(tree, JString::empty, false),
	itsLastUTime(0),
	itsLastSTime(0)
{
	itsUID = entry.kp_eproc.e_pcred.p_ruid;
	itsPID = entry.kp_proc.p_pid;

	passwd* info = getpwuid(itsUID);
	if (info != nullptr)
	{
		itsUser = info->pw_name;
	}
}

#endif

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

#ifdef _J_HAS_PROC
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
#elif defined _J_HAS_SYSCTL
{
	int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_PID, itsPID };

	kinfo_proc entry;
	size_t len = sizeof(entry);
	int result = sysctl(mib, 4, &entry, &len, nullptr, 0);
	if (result != 0)
	{
		itsState = kZombie;
	}
	else
	{
		itsCommand  = entry.kp_proc.p_comm;
		itsPPID     = entry.kp_eproc.e_ppid;
		itsPriority = entry.kp_proc.p_priority;
		itsNice     = entry.kp_proc.p_nice;
		itsSize     = 0;
		itsResident = 0;
		itsShare    = 0;
		itsUTime    = entry.kp_proc.p_uticks;
		itsSTime    = entry.kp_proc.p_sticks;

		if (entry.kp_proc.p_stat == SSLEEP)
		{
			itsState = kSleep;
		}
		else if (entry.kp_proc.p_stat == SSTOP)
		{
			itsState = kStopped;
		}
		else if (entry.kp_proc.p_stat == SZOMB)
		{
			itsState = kZombie;
		}
		else
		{
			itsState = kRun;
		}

		JSize mem;
		if (GetSystemMemory(&mem))
		{
			itsPercentMemory = JFloat(itsResident) / mem;
		}
	}

	// shared across #if
	ReadCmdline();	// not in ctor, to make ctor faster
}
#endif

	SetName(itsCommand);
	ShouldBeOpenable(HasChildren());

	itsTime	        = (itsUTime + itsSTime) / sysconf(_SC_CLK_TCK);
	JSize totalTime	= (itsLastUTime == 0 || itsLastSTime == 0) ? 0 : (itsUTime - itsLastUTime) + (itsSTime - itsLastSTime);
	itsLastUTime	= itsUTime;
	itsLastSTime	= itsSTime;
	itsPercentCPU	= elapsedTime == 0 || itsState == kZombie ? 0 : JFloat(totalTime * 1000 / sysconf(_SC_CLK_TCK)) / (10 * elapsedTime);
}

#ifdef _J_HAS_PROC

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

#elif defined _J_HAS_SYSCTL

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

#ifdef KERN_PROC_ARGS

	int mib[] = { CTL_KERN, KERN_PROC, KERN_PROC_ARGS, itsPID };

	if (sysctl(mib, 4, nullptr, &len, nullptr, 0) == 0)
	{
		void* buf = malloc(len);
		assert( buf != nullptr );

		result = sysctl(mib, 4, buf, &len, nullptr, 0);
		if (result == 0)
		{
			JIndex i = 0;
			while (i < len)
			{
				itsFullCommand += ((char*) buf) + i;
				itsFullCommand.AppendCharacter(' ');

				i += strlen(((char*) buf) + i) + 1;
			}
		}

		free(buf);
	}

#elif defined KERN_PROCARGS2

	int mib[] = { CTL_KERN, KERN_ARGMAX, 0 };

	int argmax;
	size_t len = sizeof(argmax);
	if (sysctl(mib, 2, &argmax, &len, nullptr, 0) == 0)
	{
		void* buf = malloc(argmax);
		assert( buf != nullptr );

		mib[0] = CTL_KERN;
		mib[1] = KERN_PROCARGS2;
		mib[2] = itsPID;

		len = argmax;
		if (sysctl(mib, 3, buf, &len, nullptr, 0) == 0)
		{
			int argc = * (int*) buf;
			buf      = ((char*) buf) + sizeof(argc);

			int offset = 0;
			for (int i=0; i<argc; i++)
			{
				itsFullCommand += ((char*) buf) + offset;
				itsFullCommand.Append(" ");

				offset += strlen(((char*) buf) + offset) + 1;
			}
		}
	}
/*
	task_port_t task;
	task_basic_info task_info;
	unsigned int info_count = TASK_BASIC_INFO_COUNT;
	if (task_for_pid(mach_task_self(), itsPID, &task) == KERN_SUCCESS &&
		task_info(task, TASK_BASIC_INFO, &task_info, &info_count) == KERN_SUCCESS)
	{
		std::cout << task_info.resident_size;
	}
*/
#endif
}

#endif

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
	if (e1->itsPID > e2->itsPID)
	{
		return std::weak_ordering::greater;
	}
	else if (e1->itsPID < e2->itsPID)
	{
		return std::weak_ordering::less;
	}
	else
	{
		return std::weak_ordering::equivalent;
	}
}

std::weak_ordering
ProcessEntry::CompareListUser
	(
	ProcessEntry * const & e1,
	ProcessEntry * const & e2
	)
{
	const std::weak_ordering result =
		JCompareStringsCaseInsensitive(&(e1->itsUser), &(e2->itsUser));

	if (result == std::weak_ordering::equivalent)
	{
		return CompareListPID(e1, e2);
	}
	else
	{
		return result;
	}
}

std::weak_ordering
ProcessEntry::CompareListNice
	(
	ProcessEntry * const & e1,
	ProcessEntry * const & e2
	)
{
	if (e1->itsNice > e2->itsNice)
	{
		return std::weak_ordering::greater;
	}
	else if (e1->itsNice < e2->itsNice)
	{
		return std::weak_ordering::less;
	}
	else
	{
		return CompareListPID(e1, e2);
	}
}

std::weak_ordering
ProcessEntry::CompareListSize
	(
	ProcessEntry * const & e1,
	ProcessEntry * const & e2
	)
{
	if (e1->itsSize > e2->itsSize)
	{
		return std::weak_ordering::greater;
	}
	else if (e1->itsSize < e2->itsSize)
	{
		return std::weak_ordering::less;
	}
	else
	{
		return CompareListPID(e1, e2);
	}
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
	if (e1->itsTime > e2->itsTime)
	{
		return std::weak_ordering::greater;
	}
	else if (e1->itsTime < e2->itsTime)
	{
		return std::weak_ordering::less;
	}
	else
	{
		return CompareListPID(e1, e2);
	}
}

std::weak_ordering
ProcessEntry::CompareListCommand
	(
	ProcessEntry * const & e1,
	ProcessEntry * const & e2
	)
{
	const std::weak_ordering result =
		JCompareStringsCaseInsensitive(&(e1->itsCommand), &(e2->itsCommand));

	if (result == std::weak_ordering::equivalent)
	{
		return CompareListPID(e1, e2);
	}
	else
	{
		return result;
	}
}

std::weak_ordering
ProcessEntry::CompareListCommandForIncrSearch
	(
	ProcessEntry * const & e1,
	ProcessEntry * const & e2
	)
{
	return JCompareStringsCaseInsensitive(&(e1->itsCommand), &(e2->itsCommand));
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

	if (e1->itsPID > e2->itsPID)
	{
		return std::weak_ordering::greater;
	}
	else if (e1->itsPID < e2->itsPID)
	{
		return std::weak_ordering::less;
	}
	else
	{
		return std::weak_ordering::equivalent;
	}
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

	const std::weak_ordering result =
		JCompareStringsCaseInsensitive(&(e1->itsUser), &(e2->itsUser));

	if (result == std::weak_ordering::equivalent)
	{
		return CompareTreePID(n1, n2);
	}
	else
	{
		return result;
	}
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

	if (e1->itsNice > e2->itsNice)
	{
		return std::weak_ordering::greater;
	}
	else if (e1->itsNice < e2->itsNice)
	{
		return std::weak_ordering::less;
	}
	else
	{
		return CompareTreePID(n1, n2);
	}
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

	if (e1->itsSize > e2->itsSize)
	{
		return std::weak_ordering::greater;
	}
	else if (e1->itsSize < e2->itsSize)
	{
		return std::weak_ordering::less;
	}
	else
	{
		return CompareTreePID(n1, n2);
	}
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
		return CompareTreePID(n1, n2);
	}
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
		return CompareTreePID(n1, n2);
	}
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

	if (e1->itsTime > e2->itsTime)
	{
		return std::weak_ordering::greater;
	}
	else if (e1->itsTime < e2->itsTime)
	{
		return std::weak_ordering::less;
	}
	else
	{
		return CompareTreePID(n1, n2);
	}
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

	const std::weak_ordering result =
		JCompareStringsCaseInsensitive(&(e1->itsCommand), &(e2->itsCommand));

	if (result == std::weak_ordering::equivalent)
	{
		return CompareTreePID(n1, n2);
	}
	else
	{
		return result;
	}
}
