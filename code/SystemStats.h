/******************************************************************************
 SystemStats.h

	Interface for the SystemStats class

	Copyright (C) 2011 by John Lindal.

 ******************************************************************************/

#ifndef _H_SystemStats
#define _H_SystemStats

#include <jx-af/jx/JXWidget.h>

class ProcessList;

class SystemStats : public JXWidget
{
public:

	SystemStats(ProcessList* processList, JXContainer* enclosure,
				   const HSizingOption hSizing, const VSizingOption vSizing,
				   const JCoordinate x, const JCoordinate y,
				   const JCoordinate w, const JCoordinate h);

	~SystemStats() override;

protected:

	void	Draw(JXWindowPainter& p, const JRect& rect) override;	
	void	DrawBorder(JXWindowPainter& p, const JRect& frame) override;

	void	Receive(JBroadcaster* sender, const Message& message) override;

public:

	struct CPU
	{
		JFloat user;
		JFloat other;
	};

private:

	const JIndex	itsUID;
	ProcessList*	itsProcessList;		// not owned
	JArray<CPU>*	itsCPUHistory;		// newest appended to the end
	JFloat			itsMaxCPU;

private:

	void	ComputeStats(JFloat* userCPUPercentage, JFloat* otherCPUPercentage,
						 JFloat* userMemoryPercentage, JFloat* otherMemoryPercentage) const;
};

#endif
