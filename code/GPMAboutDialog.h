/******************************************************************************
 GPMAboutDialog.h

	Copyright (C) 2001 by Glenn W. Bach.

 ******************************************************************************/

#ifndef _H_GPMAboutDialog
#define _H_GPMAboutDialog

#include <jx-af/jx/JXDialogDirector.h>

class JXTextButton;

class GPMAboutDialog : public JXDialogDirector
{
public:

	GPMAboutDialog(JXDirector* supervisor, const JString& prevVersStr);

	virtual ~GPMAboutDialog();

protected:

	virtual void	Receive(JBroadcaster* sender, const Message& message) override;

private:

	bool	itsIsUpgradeFlag;

// begin JXLayout

	JXTextButton* itsHelpButton;
	JXTextButton* itsCreditsButton;

// end JXLayout

private:

	void	BuildWindow(const JString& prevVersStr);
};

#endif
