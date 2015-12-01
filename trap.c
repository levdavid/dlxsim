/* 
 * trap.c --
 *
 *	This file contains functions that handle various predefined
 *	trap numbers, which are used to perform system calls.
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

/* one possible problem exists below in the Handle_Printf function.  I
 * did not have access to the output of the DLX compiler, so I assumed
 * that the compiler, when passing a floating point value to a function,
 * places the lower floating point register in the lower address (eg.
 * F0 in 0(r14), and F1 in 4(r14)).  If this is not correct, exchange the
 * assignments to ta[0] and ta[1].
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>
#include <tcl.h>
#include <sys/errno.h>
#include "asm.h"
#include "dlx.h"
#include "sym.h"
#include "traps.h"

extern int errno;

#define RETURN_VALUE  (machPtr->regs[1])
#define ARG(n)  (machPtr->memPtr[ADDR_TO_INDEX(machPtr->regs[14])+(n)].value)
#define SCRATCH_ADDR(n)  ((char *)machPtr->memScratch + (n))

/*
 *----------------------------------------------------------------------
 *
 * Init_Handle_Trap --
 *
 *	Prepare the system for handling traps.  This mainly makes
 *      copies of the stdin, stdout, and stderr for use by the
 *      simluated program.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      The fd_map is set up.
 *
 *----------------------------------------------------------------------
 */

Init_Handle_Trap(machPtr)
DLX *machPtr;
{
  int i;

  for (i = 0; i < FD_SIZE; i++)
    machPtr->fd_map[i] = -1;
  if ((machPtr->fd_map[0] = dup(0)) == -1)
    printf("Could not open simulated standard input\n");
  if ((machPtr->fd_map[1] = dup(1)) == -1)
    printf("Could not open simulated standard output\n");
  if ((machPtr->fd_map[2] = dup(2)) == -1)
    printf("Could not open simulated standard error\n");
}

/*
 *----------------------------------------------------------------------
 *
 * Handle_Trap --
 *
 *	Handle the requested trap number.  Any arguments are expected
 *      to start at the address in r14 (the first argument) with other
 *      arguments in words above that in memory.  The result of the
 *      simulated trap is placed in r1.
 *
 * Results:
 *	The return value is 0 if no error occured, 1 if the trap is
 *      known, but the trap handler had a problem, or 2 if the trap
 *      number is not known
 *
 * Side effects:
 *      The fd_map or the simulated DLX's memory may be changed.
 *
 *----------------------------------------------------------------------
 */

int Handle_Trap(machPtr, trapNum)
DLX *machPtr;
int trapNum;
{
  switch (trapNum) {
  case TRAP_OPEN :
    return Handle_Open(machPtr);
  case TRAP_CLOSE :
    return Handle_Close(machPtr);
  case TRAP_READ :
    return Handle_Read(machPtr);
  case TRAP_WRITE :
    return Handle_Write(machPtr);
  case TRAP_PRINTF :
    return Handle_Printf(machPtr);
  }
  return 2;
}

/*
 *----------------------------------------------------------------------
 *
 * Block_To_Scratch --
 *
 *	Copy a block of memory from the simulated memory to the scratch
 *      pad.
 *
 * Results:
 *	The return value is 0 if no error occured, 1 if a problem
 *      arises.
 *
 * Side effects:
 *      The memScratch is altered.
 *
 *----------------------------------------------------------------------
 */

int Block_To_Scratch(machPtr, firstAddr, length)
DLX *machPtr;
register int firstAddr, length;
{
  char *first, *last, bytes[4];
  register unsigned int word;
  MemWord *wordPtr;

  if (!length)
    return 0;
  length--;
  if (length < 0) {
    printf("Invalid block movement\n");
    return 1;
  }
  if ((firstAddr < 0) || (firstAddr + length >= machPtr->numChars)) {
    printf("Memory reference out of range\n");
    return 1;
  }

  first = SCRATCH_ADDR(firstAddr);
  last = first + length;
  wordPtr = machPtr->memPtr + ADDR_TO_INDEX(firstAddr);
  word = wordPtr->value;
  bytes[3] = (word & 0xff);
  bytes[2] = ((word >>= 8) & 0xff);
  bytes[1] = ((word >>= 8) & 0xff);
  bytes[0] = ((word >> 8) & 0xff);
  while (first <= last) {
    *first = bytes[firstAddr & 0x3];
    firstAddr++; first++;
    if (!(firstAddr & 0x3)) {
      word = (++wordPtr)->value;
      bytes[3] = (word & 0xff);
      bytes[2] = ((word >>= 8) & 0xff);
      bytes[1] = ((word >>= 8) & 0xff);
      bytes[0] = ((word >> 8) & 0xff);
    }
  }
  return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * Block_From_Scratch --
 *
 *	Copy a block of memory from the scratch pad to the simulated
 *      memory.
 *
 * Results:
 *	The return value is 0 if no error occured, 1 if a problem
 *      arises.
 *
 * Side effects:
 *      The memPtr data is altered.
 *
 *----------------------------------------------------------------------
 */

int Block_From_Scratch(machPtr, firstAddr, length)
DLX *machPtr;
register int firstAddr, length;
{
  char *first, *last;
  register unsigned int word;
  MemWord *wordPtr;

  if (!length)
    return 0;
  length--;
  if (length < 0) {
    printf("Invalid block movement\n");
    return 1;
  }
  if ((firstAddr < 0) || (firstAddr + length >= machPtr->numChars)) {
    printf("Memory reference out of range\n");
    return 1;
  }

  wordPtr = machPtr->memPtr + ADDR_TO_INDEX(firstAddr);
  word = wordPtr->value;
  first = SCRATCH_ADDR(firstAddr);
  last = first + length;
  while (first <= last) {
    switch (firstAddr & 0x3) {
    case 0 :
      word = (word & 0x00ffffff) | (*first << 24);
      break;
    case 1 :
      word = (word & 0xff00ffff) | (*first << 16);
      break;
    case 2 :
      word = (word & 0xffff00ff) | (*first << 8);
      break;
    case 3 :
      word = (word & 0xffffff00) | *first;
      wordPtr->value = word;
      wordPtr->opCode = OP_NOT_COMPILED;
      word = (++wordPtr)->value;
      break;
    }
    firstAddr++; first++;
  }
  if (wordPtr->value != word) {
    wordPtr->value = word;    /* update the last word */
    wordPtr->opCode = OP_NOT_COMPILED;
  }
  return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * String_To_Scratch --
 *
 *	Copy a NULL terminated string from simulator memory to the
 *      scratch pad.
 *
 * Results:
 *	The return value is 0 if no error occured, 1 if a problem
 *      arises.
 *
 * Side effects:
 *      The memScratch data is altered.
 *
 *----------------------------------------------------------------------
 */

int String_To_Scratch(machPtr, firstAddr)
DLX *machPtr;
register int firstAddr;
{
  char *first, *endMem = machPtr->endScratch;
  register unsigned int word;
  register int value;
  unsigned char bytes[4];
  MemWord *wordPtr;

  if ((firstAddr < 0) || (firstAddr >= machPtr->numChars)) {
    printf("Memory reference out of range\n");
    return 1;
  }

  first = SCRATCH_ADDR(firstAddr);
  wordPtr = machPtr->memPtr + ADDR_TO_INDEX(firstAddr);
  word = wordPtr->value;
  bytes[3] = (word & 0xff);
  bytes[2] = ((word >>= 8) & 0xff);
  bytes[1] = ((word >>= 8) & 0xff);
  bytes[0] = ((word >> 8) & 0xff);
  while (first < endMem) {
    if (!(*first = bytes[firstAddr & 0x3]))
      break;
    firstAddr++; first++;
    if (!(firstAddr & 0x3)) {
      word = (++wordPtr)->value;
      bytes[3] = (word & 0xff);
      bytes[2] = ((word >>= 8) & 0xff);
      bytes[1] = ((word >>= 8) & 0xff);
      bytes[0] = ((word >> 8) & 0xff);
    }
  }

  if (first >= endMem) {
    printf("Memory reference out of range\n");
    return 1;
  }
  return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * Handle_Open --
 *
 *	Handle the open system call.
 *
 * Results:
 *	The return value is 0 if no error occured, 1 if a problem
 *      arises.
 *
 * Side effects:
 *      The fd_map may be changed.
 *
 *----------------------------------------------------------------------
 */

int Handle_Open(machPtr)
DLX *machPtr;
{
  int where, path, result;

  for (where = 0; where < FD_SIZE; where++)
    if (machPtr->fd_map[where] == -1)
      break;
  if (where == FD_SIZE) {    /* no free file descriptors */
    Set_Errno(machPtr, EMFILE);
    RETURN_VALUE = -1;
    return 0;
  }
  path = ARG(0);
  if (String_To_Scratch(machPtr, path))
    return 1;
  result = open(SCRATCH_ADDR(path), ARG(1), ARG(2));
  if (result == -1) {
    Set_Errno(machPtr, errno);
    RETURN_VALUE = -1;
  } else {
    machPtr->fd_map[where] = result;
    RETURN_VALUE = where;
  }
  return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * Handle_Close --
 *
 *	Handle the close system call.
 *
 * Results:
 *	The return value is 0 if no error occured, 1 if a problem
 *      arises.
 *
 * Side effects:
 *      The fd_map may be changed.
 *
 *----------------------------------------------------------------------
 */

int Handle_Close(machPtr)
DLX *machPtr;
{
  int result, fd;

  fd = ARG(0);
  if ((fd < 0) || (fd > FD_SIZE) || (machPtr->fd_map[fd] == -1)) {
    /* illegal file descriptor */
    Set_Errno(machPtr, EBADF);
    RETURN_VALUE = -1;
    return 0;
  }
  result = close(machPtr->fd_map[fd]);
  if (result == -1) {
    Set_Errno(machPtr, errno);
    RETURN_VALUE = -1;
  } else {
    RETURN_VALUE = result;
    machPtr->fd_map[fd] = -1;
  }
  return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * Handle_Read --
 *
 *	Handle the read system call.
 *
 * Results:
 *	The return value is 0 if no error occured, 1 if a problem
 *      arises.
 *
 * Side effects:
 *      The simulator memory and scratch pad may be changed.
 *
 *----------------------------------------------------------------------
 */

int Handle_Read(machPtr)
DLX *machPtr;
{
  int result, fd, buf, nbytes;

  fd = ARG(0);
  buf = ARG(1);
  nbytes = ARG(2);
  if ((fd < 0) || (fd > FD_SIZE) || (machPtr->fd_map[fd] == -1)) {
    /* illegal file descriptor */ 
    Set_Errno(machPtr, EBADF);
    RETURN_VALUE = -1;
    return 0;
  }
  if ((buf < 0) || (buf >= machPtr->numChars)) {
    printf("Memory reference out of range\n");
    return 1;
  }
  if ((nbytes < 0) || (buf + nbytes - 1 >= machPtr->numChars)) {
    printf("Memory reference out of range\n");
    return 1;
  }
  result = read(machPtr->fd_map[fd], SCRATCH_ADDR(buf), nbytes);
  if (result == -1) {
    Set_Errno(machPtr, errno);
    RETURN_VALUE = -1;
  } else {
    if (Block_From_Scratch(machPtr, buf, result))
      return 1;
    RETURN_VALUE = result;
  }
  return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * Handle_Write --
 *
 *	Handle the write system call.
 *
 * Results:
 *	The return value is 0 if no error occured, 1 if a problem
 *      arises.
 *
 * Side effects:
 *      The scratch pad may be changed.
 *
 *----------------------------------------------------------------------
 */

int Handle_Write(machPtr)
DLX *machPtr;
{
  int result, fd, buf, nbytes;

  fd = ARG(0);
  buf = ARG(1);
  nbytes = ARG(2);
  if ((fd < 0) || (fd > FD_SIZE) || (machPtr->fd_map[fd] == -1)) {
    /* illegal file descriptor */
    Set_Errno(machPtr, EBADF);
    RETURN_VALUE = -1;
    return 0;
  }
  if (Block_To_Scratch(machPtr, buf, nbytes))
    return 1;
  result = write(machPtr->fd_map[fd], SCRATCH_ADDR(buf), nbytes);
  if (result == -1) {
    Set_Errno(machPtr, errno);
    RETURN_VALUE = -1;
  } else {
    RETURN_VALUE = result;
  }
  return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * Handle_Printf --
 *
 *	Handle the printf system call.  This is done by scanning
 *      through the format string provided, and sending each % command
 *      and following plain text to the printf library function (with
 *      the appropriate argument).
 *
 * Results:
 *	The return value is 0 if no error occured, 1 if a problem
 *      arises.
 *
 * Side effects:
 *      The scratch pad may be changed.
 *
 *----------------------------------------------------------------------
 */

int Handle_Printf(machPtr)
DLX *machPtr;
{
  char *start, *end, *s, temp;
  int fmt = ARG(0), argNumber = 1, ta[2], loc, done, result = 0;
  double *pd = (double *)&(ta[0]);

  /* convert the format string */
  if (String_To_Scratch(machPtr, fmt))
    return 1;

  /* now break the format string into blocks of characters beginning with
   * either the start of the format string, or a % command, and continuing
   * through either the start of the next % command, or the end of the
   * string.  */
  for (start = SCRATCH_ADDR(fmt); *start; start = end) {
    if (*start == '%') {
      if (start[1] == '%')
	end = start + 2;
      else
	end = start + 1;
    } else
      end = start;
    for ( ; *end && (*end != '%'); end++)     /* find the end of this block */
      ;
    temp = *end;
    *end = '\0';
    if (*start != '%')
      result += printf(start);
    else {
      /* find the type of argument this block takes (if any) */
      for (done = 0, s = start + 1; *s && !done; s++)
	switch (*s) {
	case 'c' :
	case 'd' :
	case 'o' :
	case 'u' :
	case 'x' :
	case 'X' :
	  result += printf(start, ARG(argNumber++));
	  done = 1;
	  break;
	case 'f' :
	case 'e' :
	case 'E' :
	case 'g' :
	case 'G' :
	  ta[0] = ARG(argNumber++);
	  ta[1] = ARG(argNumber++);
	  result += printf(start, *pd);
	  done = 1;
	  break;
	case 's' :
	  loc = ARG(argNumber++);
	  if (String_To_Scratch(machPtr, loc))
	    return 1;
	  result += printf(start, SCRATCH_ADDR(loc));
	  done = 1;
	  break;
	case '%' :
	  result += printf(start);
	  done = 1;
	  break;
	}
      if (!done)    /* no % command found */
	result += printf(start);
    }
    *end = temp;
  }
  RETURN_VALUE = result;
  return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * Set_Errno
 *
 *	Set the _error variable (if it exists)
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      See the description above
 *
 *----------------------------------------------------------------------
 */

Set_Errno(machPtr, errorNumber)
DLX *machPtr;
int errorNumber;
{
  char *p, *end;
  int addr, index;

  p = Tcl_GetVar(machPtr->interp, "_errno", 0);
  if (*p != 0) {
    addr = strtoul(p, &end, 0);
    if (*end != 0)
      return;
    index = ADDR_TO_INDEX(addr);
    if (index < machPtr->numWords)
      machPtr->memPtr[index].value = errorNumber;
  }
}
