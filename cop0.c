/* 
 * cop0.c --
 *
 *	This file implements a subset of the DLX's coprocessor 0
 *	facilities for DlXsim.  Right now only enough is implemented
 *	to handle simple I/O interrupts.
 *
 * Copyright 1989 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#ifndef lint
static char rcsid[] = "$Header: /user1/ouster/mipsim/RCS/cop0.c,v 1.1 89/11/20 10:57:47 ouster Exp $ SPRITE (Berkeley)";
#endif /* not lint */

#include <stdio.h>
#include "dlx.h"

#define VECTOR_ADDRESS		0x80

/*
 * Forward declarations for procedures defined later in this file:
 */

static void	Interrupt();

/*
 *----------------------------------------------------------------------
 *
 * Cop0_Init --
 *
 *	This procedure is called when a machine is created to
 *	initialize its coprocessor 0 state.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Whatever it takes to initialize the state.
 *
 *----------------------------------------------------------------------
 */

void
Cop0_Init(machPtr)
    register DLX *machPtr;		/* Machine being created. */
{
    int i;

    machPtr->cop0.cause = 0;
    machPtr->cop0.status = 0;
    machPtr->cop0.epc = 0;
    for (i = 0; i < COP0_NUM_LEVELS; i++) {
	machPtr->cop0.pending[i] = 0;
    }
    machPtr->cop0.pendingMask = 0;
}

/*
 *----------------------------------------------------------------------
 *
 * Cop0_IntPending --
 *
 *	This procedure is called to register a change in the pending
 *	status of interrupts.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The number of pending interrupts at level "level" is incremented
 *	by "change".  If the new number is greater than zero and the
 *	corresponding interrupt level is enabled, then an I/O interrupt
 *	will occur.
 *
 *----------------------------------------------------------------------
 */

void
Cop0_IntPending(machPtr, level, change)
    register DLX *machPtr;		/* Machine to manipulate. */
    int level;				/* Interrupt level (0-5). */
    int change;				/* 1 means a new interrupt source
					 * has just become ready;  -1 means
					 * an interrupt source has become
					 * no longer ready. */
{
    int old, new;
    old = machPtr->cop0.pending[level];
    new = old + change;
    machPtr->cop0.pending[level] = new;

    if (new < 0) {
	printf("Hey, level %d just went negative!\n", level);
    }
    if ((old <= 0) && (new > 0)) {
	machPtr->cop0.pendingMask |= COP0_STATUS_IMASK0 << level;
	if (machPtr->cop0.status & machPtr->cop0.pendingMask) {
	    Sim_CallBack(machPtr, 0, Interrupt, (ClientData) machPtr);
	}
    } else if ((old > 0) && (new <= 0)) {
	machPtr->cop0.pendingMask &= ~(COP0_STATUS_IMASK0 << level);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Interrupt --
 *
 *	This is a Sim_CallBack procedure that is invoked at times
 *	when there MAY be an interrupt pending.  This procedure
 *	checks to see if an interrupt really should be delivered;
 *	if so, it delivers it.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The state of the machine may be updated to reflect the
 *	delivery of an interrupt.
 *
 *----------------------------------------------------------------------
 */

static void
Interrupt(machPtr)
    register DLX *machPtr;	/* Machine to check for interrupts. */
{
    if (!(machPtr->cop0.status & COP0_STATUS_IEC)) {
	return;
    }
    if ((machPtr->cop0.status & machPtr->cop0.pendingMask) == 0) {
	return;
    }
    machPtr->cop0.cause = (machPtr->cop0.status & machPtr->cop0.pendingMask)
	    | COP0_CAUSE_CODE_INT;
    if (machPtr->branchSerial == (machPtr->insCount-1)) {
	machPtr->cop0.epc = machPtr->branchPC;
	machPtr->cop0.cause |= COP0_CAUSE_BD;
    } else {
	machPtr->cop0.epc = Sim_GetPC(machPtr);
    }
    if (machPtr->cop0.status & COP0_STATUS_IEP) {
	machPtr->cop0.status |= COP0_STATUS_IE0;
    } else {
	machPtr->cop0.status &= ~COP0_STATUS_IE0;
    }
    if (machPtr->cop0.status & COP0_STATUS_IEC) {
	machPtr->cop0.status |= COP0_STATUS_IEP;
    } else {
	machPtr->cop0.status &= ~COP0_STATUS_IEP;
    }
    machPtr->cop0.status &= ~COP0_STATUS_IEC;
    machPtr->regs[PC_REG] = ADDR_TO_INDEX(VECTOR_ADDRESS);
    machPtr->regs[NEXT_PC_REG] = machPtr->regs[PC_REG] + 1;
}
