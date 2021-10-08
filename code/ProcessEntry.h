/******************************************************************************
 ProcessEntry.h

	Interface for the ProcessEntry class

	Copyright (C) 2000 by Glenn W. Bach.

 *****************************************************************************/

#ifndef _H_ProcessEntry
#define _H_ProcessEntry

#include <jx-af/jcore/JNamedTreeNode.h>
#include <sys/types.h>

#ifdef _J_HAS_SYSCTL
#include <sys/sysctl.h>
#endif

class JDirEntry;

class ProcessEntry : public JNamedTreeNode
{
public:

	enum State
	{
		kSleep	= 1,
		kUnIntSleep,
		kRun,
		kZombie,
		kStopped
	};

public:

	#ifdef _J_HAS_PROC
	ProcessEntry(JTree* tree, const JDirEntry& entry);
	#elif defined _J_HAS_SYSCTL
	ProcessEntry(JTree* tree, const kinfo_proc& entry);
	#endif

	ProcessEntry(JTree* tree, const JString& prefix);

	~ProcessEntry() override;

	void	Update(const JFloat elapsedTime);

	State			GetState() const;
	uid_t			GetUID() const;
	const JString&	GetUser() const;
	pid_t			GetPID() const;
	const JString&	GetCommand() const;
	const JString&	GetFullCommand() const;
	pid_t			GetPPID() const;
	JInteger		GetPriority() const;
	JInteger		GetNice() const;
	JSize			GetSize() const;
	JSize			GetResident() const;
	JSize			GetShare() const;
	JSize			GetTime() const;
	JFloat			GetPercentMemory() const;
	JFloat			GetPercentCPU() const;

	static JListT::CompareResult
		CompareListPID(ProcessEntry * const & e1, ProcessEntry * const & e2);
	static JListT::CompareResult
		CompareListUser(ProcessEntry * const & e1, ProcessEntry * const & e2);
	static JListT::CompareResult
		CompareListNice(ProcessEntry * const & e1, ProcessEntry * const & e2);
	static JListT::CompareResult
		CompareListSize(ProcessEntry * const & e1, ProcessEntry * const & e2);
	static JListT::CompareResult
		CompareListPercentMemory(ProcessEntry * const & e1, ProcessEntry * const & e2);
	static JListT::CompareResult
		CompareListPercentCPU(ProcessEntry * const & e1, ProcessEntry * const & e2);
	static JListT::CompareResult
		CompareListTime(ProcessEntry * const & e1, ProcessEntry * const & e2);
	static JListT::CompareResult
		CompareListCommand(ProcessEntry * const & e1, ProcessEntry * const & e2);
	static JListT::CompareResult
		CompareListCommandForIncrSearch(ProcessEntry * const & e1, ProcessEntry * const & e2);

	static JListT::CompareResult
		CompareTreePID(JTreeNode * const & e1, JTreeNode * const & e2);
	static JListT::CompareResult
		CompareTreeUser(JTreeNode * const & e1, JTreeNode * const & e2);
	static JListT::CompareResult
		CompareTreeNice(JTreeNode * const & e1, JTreeNode * const & e2);
	static JListT::CompareResult
		CompareTreeSize(JTreeNode * const & e1, JTreeNode * const & e2);
	static JListT::CompareResult
		CompareTreePercentMemory(JTreeNode * const & e1, JTreeNode * const & e2);
	static JListT::CompareResult
		CompareTreePercentCPU(JTreeNode * const & e1, JTreeNode * const & e2);
	static JListT::CompareResult
		CompareTreeTime(JTreeNode * const & e1, JTreeNode * const & e2);
	static JListT::CompareResult
		CompareTreeCommand(JTreeNode * const & e1, JTreeNode * const & e2);

private:

	#ifdef _J_HAS_PROC
	JString			itsProcPath;
	#endif

	uid_t			itsUID;
	JString			itsUser;
	pid_t			itsPID;
	JString			itsCommand;
	JString			itsFullCommand;
	State			itsState;
	pid_t			itsPPID;
	JInteger		itsPriority;
	JInteger		itsNice;
	JSize			itsSize;
	JSize			itsResident;
	JSize			itsShare;
	JSize			itsTime;
	JFloat			itsPercentMemory;
	JFloat			itsPercentCPU;
	JSize			itsUTime;
	JSize			itsSTime;
	JSize			itsLastUTime;
	JSize			itsLastSTime;

private:

	void	ReadCmdline();

	#ifdef _J_HAS_PROC
	void	ReadStat();
	void	ReadStatM();
	#endif
};


/******************************************************************************
 GetState

 ******************************************************************************/

inline ProcessEntry::State
ProcessEntry::GetState()
	const
{
	return itsState;
}

/******************************************************************************
 GetUID

 ******************************************************************************/

inline uid_t
ProcessEntry::GetUID()
	const
{
	return itsUID;
}

/******************************************************************************
 GetUser

 ******************************************************************************/

inline const JString&
ProcessEntry::GetUser()
	const
{
	return itsUser;
}

/******************************************************************************
 GetPID

 ******************************************************************************/

inline pid_t
ProcessEntry::GetPID()
	const
{
	return itsPID;
}

/******************************************************************************
 GetCommand

 ******************************************************************************/

inline const JString&
ProcessEntry::GetCommand()
	const
{
	return itsCommand;
}

/******************************************************************************
 GetFullCommand

 ******************************************************************************/

inline const JString&
ProcessEntry::GetFullCommand()
	const
{
	return itsFullCommand;
}

/******************************************************************************
 GetPPID

 ******************************************************************************/

inline pid_t
ProcessEntry::GetPPID()
	const
{
	return itsPPID;
}

/******************************************************************************
 GetPriority

 ******************************************************************************/

inline JInteger
ProcessEntry::GetPriority()
	const
{
	return itsPriority;
}

/******************************************************************************
 GetNice

 ******************************************************************************/

inline JInteger
ProcessEntry::GetNice()
	const
{
	return itsNice;
}

/******************************************************************************
 GetSize

 ******************************************************************************/

inline JSize
ProcessEntry::GetSize()
	const
{
	return itsSize;
}

/******************************************************************************
 GetResident

 ******************************************************************************/

inline JSize
ProcessEntry::GetResident()
	const
{
	return itsResident;
}

/******************************************************************************
 GetShare

 ******************************************************************************/

inline JSize
ProcessEntry::GetShare()
	const
{
	return itsShare;
}

/******************************************************************************
 GetTime

 ******************************************************************************/

inline JSize
ProcessEntry::GetTime()
	const
{
	return itsTime;
}

/******************************************************************************
 GetPercentMemory

 ******************************************************************************/

inline JFloat
ProcessEntry::GetPercentMemory()
	const
{
	return itsPercentMemory;
}

/******************************************************************************
 GetPercentCPU

 ******************************************************************************/

inline JFloat
ProcessEntry::GetPercentCPU()
	const
{
	return itsPercentCPU;
}

#endif
