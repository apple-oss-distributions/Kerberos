/*
 * Address.h
 *
 * $Header: /cvs/kfm/KerberosClients/KerberosApp/Sources/Headers/Address.h,v 1.2 2004/09/20 20:32:04 lxs Exp $
 *
 * Copyright 2004 Massachusetts Institute of Technology.
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

@interface Address : NSObject
{
    krb5_address address;
}

- (id) initWithType: (krb5_addrtype) type length: (unsigned int) length contents: (krb5_octet *) contents;
- (void) dealloc;
- (krb5_address *) krb5_address;
- (NSString *) stringValue;

@end
