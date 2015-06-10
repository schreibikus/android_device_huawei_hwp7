/*-
 * Copyright (c) 1999 Brian Somers <brian@Awfulhak.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/usr.sbin/ppp/layer.h,v 1.3 1999/08/28 01:18:31 peter Exp $
 */
#ifndef __PPP_LAYER_H__
#define __PPP_LAYER_H__

#include "vos.h"
#include "ppp_public.h"

#pragma pack(4)
#define    LAYER_ASYNC    2
#define    LAYER_SYNC    3
#define    LAYER_HDLC    4
#define    LAYER_ACF    5
#define    LAYER_PROTO    6
#define    LAYER_LQR    7
#define    LAYER_CCP    8
#define    LAYER_VJ    9
#define    LAYER_NAT    10

#define    LAYER_MAX    10    /* How many layers we can handle on a link */

struct ppp_mbuf;
struct link;


struct layer {
  VOS_INT32 type;
  const VOS_CHAR *name;
  struct ppp_mbuf *(*push)(/*struct bundle *, */struct link *, struct ppp_mbuf *,
                       VOS_INT32 pri, VOS_UINT16 *proto);
  PPP_ZC_STRU *(*pull)(/*struct bundle *,*/struct link *, PPP_ZC_STRU *,
                       VOS_UINT16 *);
};
#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif

#endif /* end of layer.h */
