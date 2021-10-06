/*
 * Copyright 1998-2003 Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 * require a specific license from the United States Government.
 * It is the responsibility of any person or organization contemplating
 * export to obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 */

/* $Header$ */
 
#ifndef __CREDENTIALSCACHEINTERNAL__
#define __CREDENTIALSCACHEINTERNAL__

#if defined(macintosh) || (defined(__MACH__) && defined(__APPLE__))
#    include <TargetConditionals.h>
#    if TARGET_RT_MAC_CFM
#        error "Use KfM 4.0 SDK headers for CFM compilation."
#    endif
#endif

#include <Kerberos/CredentialsCache.h>
#include <Carbon/Carbon.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if TARGET_OS_MAC
#    pragma options align=mac68k
#endif
    
enum {
	ccClassic_EventClass    = FOUR_CHAR_CODE ('CCae'),	// Our event class
	ccClassic_EventID       = FOUR_CHAR_CODE ('CCae')	// Our only event ID
};

cc_int32 __CredentialsCacheInternalTellCCacheServerToQuit (void);

cc_int32 __CredentialsCacheInternalTellCCacheServerToBecomeUser (uid_t inNewUID);

#if TARGET_OS_MAC
#    pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CREDENTIALSCACHEINTERNAL__ */
