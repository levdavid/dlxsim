/* 
 * main.c --
 *
 *	Main program for "dlxsim" simulator for DLX architecture.
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

/* 888888888 added by rania */
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define vfork fork
#include <unistd.h>
/* 88888888888888 end of added by rania */

#ifndef lint
static char rcsid[] = "$Header: /user1/ouster/mipsim/RCS/main.c,v 1.7 89/11/05 15:28:36 ouster Exp $ SPRITE (Berkeley)";
#endif /* not lint */

#include <bstring.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tcl.h>
#include "dlx.h"

static Tcl_Interp *interp;
static DLX *machPtr;

/*
 * Forward references to procedures declared later in this file:
 */

static void Interrupt();

/*
 *----------------------------------------------------------------------
 *
 * main --
 *
 *	Top-level procedure for dlxsim simulator.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Tons:  read the user manual for details.
 *
 *----------------------------------------------------------------------
 */

#define Check	if (cmd_arg_flag > 1){ fprintf(stderr, "usage : command line option conflict [%s]", p); exit(0);} 

main(argc, argv)
int argc;
char **argv;
{
    char cmd[1000], *p, *command = *argv;
    int c, result;
    int add_units = NUM_ADD_UNITS;
    int mul_units = NUM_MUL_UNITS;
    int div_units = NUM_DIV_UNITS;
    int add_latency = FP_ADD_LATENCY;
    int mul_latency = FP_MUL_LATENCY;
    int div_latency = FP_DIV_LATENCY;
    int mem_size = MEMSIZE;
    
    /* command line flag for branch prediction strategies */
    int strategy = 0;
    int cmd_arg_flag = 0;
    /* default 0 -> delay slot
      1 -> flushing
      2 -> pred not taken
      3 -> -dyn-branch-pred
      4 -> ideal
    */
    
    interp = Tcl_CreateInterp();

	/* parse the command line */

  while (argv++, --argc) {
    if (*(p = *argv) != '-') {
      usageError:
        fprintf(stderr, "usage : %s [-al#] [-au#] [-dl#] [-du#] [-ml#] [-mu#] [-ms#]\n", command);
        exit(0);
    }
    
    /* todo command line */
    if (!strcmp(p,"-flushing") && !cmd_arg_flag) {
      Check
      cmd_arg_flag++;
      strategy = 1;
      continue;
    }
    else if (!strcmp(p,"-pred-not-taken") && !cmd_arg_flag) {
      cmd_arg_flag++;
      Check
      strategy = 2;
      continue;
    }
    else if (!strcmp(p,"-dyn-branch-pred") && !cmd_arg_flag) {
      cmd_arg_flag++;
      Check
      strategy = 3;
      continue;
    }
    else if (!strcmp(p,"-ideal") && !cmd_arg_flag) {
      cmd_arg_flag++;
      Check
      strategy = 4;
      continue;
    }
    
    switch (*(p+1)) {
      case 'a' :
      switch (*(p+2)) {
        case 'l' :
          add_latency = atoi(p+3);
          break;
        case 'u' :
          add_units = atoi(p+3);
          break;
        default :
          goto usageError;
          break;
      }
      break;
      case 'd' :
      switch (*(p+2)) {
        case 'l' :
        div_latency = atoi(p+3);
        break;
          case 'u' :
        div_units = atoi(p+3);
        break;
          default :
        goto usageError;
        break;
      }
      break;
      case 'm' :
        switch (*(p+2)) {
        case 'l' :
          mul_latency = atoi(p+3);
          break;
        case 's' :
          mem_size = (atoi(p+3) + 3) >> 2;
          break;
        case 'u' :
          mul_units = atoi(p+3);
          break;
        default :
          goto usageError;
        break;
        }
        break;
      default :
        goto usageError;
        break;
    }
  }
    
    if (mem_size < 1) {
	fprintf(stderr, "invalid memory size (>=1)\n");
	exit(0);
    }

    if ((add_units < 1) || (add_units > MAX_FP_UNITS)) {
	fprintf(stderr, "invalid number of add units (1-%d)\n", MAX_FP_UNITS);
	exit(0);
    }

    if ((mul_units < 1) || (mul_units > MAX_FP_UNITS)) {
	fprintf(stderr, "invalid number of multiply units (1-%d)\n", MAX_FP_UNITS);
	exit(0);
    }

    if ((div_units < 1) || (div_units > MAX_FP_UNITS)) {
	fprintf(stderr, "invalid number of divide units (1-%d)\n", MAX_FP_UNITS);
	exit(0);
    }

    if (add_latency < 1) {
	fprintf(stderr, "invalid latency for add unit (>=1)\n");
	exit(0);
    }

    if (mul_latency < 1) {
	fprintf(stderr, "invalid latency for multiply unit (>=1)\n");
	exit(0);
    }

    if (div_latency < 1) {
	fprintf(stderr, "invalid latency for divide unit (>=1)\n");
	exit(0);
    }

    
    /* todo pass strategy to machptr ? */
    machPtr = Sim_Create(strategy, mem_size, interp,
		add_units, add_latency,
		mul_units, mul_latency,
		div_units, div_latency);
    (void) signal(SIGINT, Interrupt);

    /*
     * Read a ".dlxsim" file if one exists.  Check first in the home
     * directory, then in the current directory.
     */

    p = getenv("HOME");
    if (p != NULL) {
	sprintf(cmd, "%.150s/.dlxsim", p);
	if (access(cmd, R_OK) == 0) {
	    sprintf(cmd, "source %.150s/.dlxsim", p);
	    result = Tcl_Eval(interp, cmd, 0, (char **) 0);
	    if (*interp->result != 0) {
		printf("%s\n", interp->result);
	    }
	}
    }
    if (access(".dlxsim", R_OK) == 0) {
	struct stat homeStat, cwdStat;

	/*
	 * Don't process the .dlxsim file in the current directory if
	 * the current directory is the same as the home directory:
	 * it will already have been processed above.
	 */

	(void) stat(p, &homeStat);
	(void) stat(".", &cwdStat);
	if (bcmp((char *) &homeStat, (char *) &cwdStat,
		sizeof(cwdStat))) {
	    result = Tcl_Eval(interp, "source .dlxsim", 0, (char **) 0);
	    if (*interp->result != 0) {
		printf("%s\n", interp->result);
	    }
	}
    }

    while (1) {
	clearerr(stdin);
	p = Tcl_GetVar(interp, "prompt", 1);
	if ((*p == 0) || (Tcl_Eval(interp, p, 0, (char **) 0) != TCL_OK)) {
	    fputs("(dlxsim) ", stdout);
	} else {
	    fputs(interp->result, stdout);
	}
	fflush(stdout);
	p = cmd;
	while (1) {
	    c = getchar();
	    if (c == EOF) {
		if (p == cmd) {
		    exit(0);
		}
		goto gotCommand;
	    }
	    *p = c;
	    p++;
	    if (c == '\n') {
		register char *p2;
		int parens, brackets, numBytes;

		for (p2 = cmd, parens = 0, brackets = 0; p2 != p; p2++) {
		    switch (*p2) {
			case '\\':
			    Tcl_Backslash(p2, &numBytes);
			    p2 += numBytes-1;
			    break;
			case '{':
			    parens++;
			    break;
			case '}':
			    parens--;
			    break;
			case '[':
			    brackets++;
			    break;
			case ']':
			    brackets--;
			    break;
		    }
		}
		if ((parens <= 0) && (brackets <= 0)) {
		    goto gotCommand;
		}
	    }
	}
	gotCommand:
	*p = 0;

	(void) Tcl_Eval(interp, cmd, 0, &p);
	if (*interp->result != 0) {
	    printf("%s\n", interp->result);
	}
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Interrupt --
 *
 *	This procedure is invoked when the interrupt key is typed:
 *	it causes the simulation to stop.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Causes simultor to stop after next instruction.
 *
 *----------------------------------------------------------------------
 */

static void
Interrupt()
{
  fprintf( stderr, "interrupt\n" );
  Sim_Stop(machPtr);
}

/*
 *----------------------------------------------------------------------
 *
 * Main_QuitCmd --
 *
 *	This procedure is invoked to process the "quit" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	None:  this command never returns.
 *
 * Side effects:
 *	The program exits.
 *
 *----------------------------------------------------------------------
 */

/* ARGSUSED */
int
Main_QuitCmd(machPtr, interp, argc, argv)
    DLX *machPtr;			/* Machine description. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    exit(0);
    return TCL_OK;			/* Never gets executed. */
}