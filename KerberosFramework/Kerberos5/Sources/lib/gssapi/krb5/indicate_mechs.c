/*
 * Copyright 1993 by OpenVision Technologies, Inc.
 * 
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appears in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of OpenVision not be used
 * in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission. OpenVision makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 * 
 * OPENVISION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL OPENVISION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * $Id: indicate_mechs.c 14592 2002-07-01 20:48:34Z raeburn $
 */

#include "gssapiP_krb5.h"

OM_uint32
krb5_gss_indicate_mechs(minor_status, mech_set)
     OM_uint32 *minor_status;
     gss_OID_set *mech_set;
{
   *minor_status = 0;

   if (! g_copy_OID_set(gss_mech_set_krb5_both, mech_set)) {
         *mech_set     = GSS_C_NO_OID_SET;
         *minor_status = ENOMEM;
         return(GSS_S_FAILURE);
   }

   return(GSS_S_COMPLETE);
}
