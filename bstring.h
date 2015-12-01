/*
 * bstring.h --
 *
 *	Declarations of BSD library procedures for byte manipulation.
 *
 * Copyright 1988 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 * $Header: /sprite/src/lib/include/RCS/bstring.h,v 1.2 89/06/23 11:29:33 rab Exp $ SPRITE (Berkeley)
 */

#ifndef _BSTRING
#define _BSTRING

extern int	bcmp();
extern int	bcopy();
extern int	bzero();
extern int	ffs();

#endif /* _BSTRING */
