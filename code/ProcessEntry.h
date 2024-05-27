/******************************************************************************
 ProcessEntry.h

	Interface for the ProcessEntry class

	Copyright (C) 2000 by Glenn W. Bach.

 *****************************************************************************/

#ifndef _H_ProcessEntry
#define _H_ProcessEntry

#include <jx-af/jcore/JNamedTreeNode.h>
#include <sys/types.h>

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

	ProcessEntry(JTree* tree, const JDirEntry& entry);
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

	static std::weak_ordering
		CompareListPID(ProcessEntry * const & e1, ProcessEntry * const & e2);
	static std::weak_ordering
		CompareListUser(ProcessEntry * const & e1, ProcessEntry * const & e2);
	static std::weak_ordering
		CompareListNice(ProcessEntry * const & e1, ProcessEntry * const & e2);
	static std::weak_ordering
		CompareListSize(ProcessEntry * const & e1, ProcessEntry * const & e2);
	static std::weak_ordering
		CompareListPercentMemory(ProcessEntry * const & e1, ProcessEntry * const & e2);
	static std::weak_ordering
		CompareListPercentCPU(ProcessEntry * const & e1, ProcessEntry * const & e2);
	static std::weak_ordering
		CompareListTime(ProcessEntry * const & e1, ProcessEntry * const & e2);
	static std::weak_ordering
		CompareListCommand(ProcessEntry * const & e1, ProcessEntry * const & e2);
	static std::weak_ordering
		CompareListCommandForIncrSearch(ProcessEntry * const & e1, ProcessEntry * const & e2);

	static std::weak_ordering
		CompareTreePID(JTreeNode * const & e1, JTreeNode * const & e2);
	static std::weak_ordering
		CompareTreeUser(JTreeNode * const & e1, JTreeNode * const & e2);
	static std::weak_ordering
		CompareTreeNice(JTreeNode * const & e1, JTreeNode * const & e2);
	static std::weak_ordering
		CompareTreeSize(JTreeNode * const & e1, JTreeNode * const & e2);
	static std::weak_ordering
		CompareTreePercentMemory(JTreeNode * const & e1, JTreeNode * const & e2);
	static std::weak_ordering
		CompareTreePercentCPU(JTreeNode * const & e1, JTreeNode * const & e2);
	static std::weak_ordering
		CompareTreeTime(JTreeNode * const & e1, JTreeNode * const & e2);
	static std::weak_ordering
		CompareTreeCommand(JTreeNode * const & e1, JTreeNode * const & e2);

private:

	JString			itsProcPath;
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
	void	ReadStat();
	void	ReadStatM();
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
