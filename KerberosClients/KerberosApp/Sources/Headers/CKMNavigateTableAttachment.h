/* $Copyright:
 *
 * Copyright 1998-2002 by the Massachusetts Institute of Technology.
 * 
 * All rights reserved.
 * 
 * Export of this software from the United States of America may require a
 * specific license from the United States Government.  It is the
 * responsibility of any person or organization contemplating export to
 * obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and distribute
 * this software and its documentation for any purpose and without fee is
 * hereby granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of M.I.T. not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  Furthermore if you
 * modify this software you must label your software as modified software
 * and not distribute it in such a fashion that it might be confused with
 * the original MIT software. M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Individual source code files are copyright MIT, Cygnus Support,
 * OpenVision, Oracle, Sun Soft, FundsXpress, and others.
 * 
 * Project Athena, Athena, Athena MUSE, Discuss, Hesiod, Kerberos, Moira,
 * and Zephyr are trademarks of the Massachusetts Institute of Technology
 * (MIT).  No commercial use of these trademarks may be made without prior
 * written permission of MIT.
 * 
 * "Commercial use" means use of a name in a product or other for-profit
 * manner.  It does NOT prevent a commercial firm from referring to the MIT
 * trademarks in order to convey information (although in doing so,
 * recognition of their trademark status should be given).
 * $
 */

// ===========================================================================
//	CNavigateTableAttachment.h
// ===========================================================================

#pragma once

#include <LAttachment.h>
#include <UTables.h>

//struct PP_PowerPlant::SCommandStatus;

class CKMNavigateTableAttachment : public PP_PowerPlant::LAttachment,
								 public PP_PowerPlant::LBroadcaster {

public:
	enum { class_ID = FOUR_CHAR_CODE('okey') };

							CKMNavigateTableAttachment(
									PP_PowerPlant::LTableView*			inTableView,
									PP_PowerPlant::MessageT			inMessage = PP_PowerPlant::msg_AnyMessage);
							CKMNavigateTableAttachment(
									PP_PowerPlant::LStream*			inStream);
	virtual					~CKMNavigateTableAttachment();

	// event dispatching

protected:
	virtual void			ExecuteSelf(
									PP_PowerPlant::MessageT			inMessage,
									void*				ioParam);

	virtual void			FindCommandStatus(
									PP_PowerPlant::SCommandStatus*		inCommandStatus);
	virtual void			HandleKeyEvent(
									const EventRecord*	inEvent);

	// selection behaviors

	virtual void			SelectAll();

	virtual void			UpArrow(Boolean shiftPressed);
	
	virtual void			DownArrow(Boolean shiftPressed);
			void		 	DoNavigationKey(const EventRecord	&inKeyEvent);
	
			void			GenericDeleteRow();

	PP_PowerPlant::LTableView*				mTableView;
	Boolean					mSupportsSelectAll;
};
