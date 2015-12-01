/* 
 * tclCmdAH.c --
 *
 *	This file contains the top-level command routines for most of
 *	the Tcl built-in commands whose names begin with the letters
 *	A to H.
 *
 * Copyright 1987 Regents of the University of California
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

#ifndef lint
static char rcsid[] = "$Header: /sprite/src/lib/tcl/RCS/tclCmdAH.c,v 1.38 90/01/15 15:15:48 ouster Exp Locker: ouster $ SPRITE (Berkeley)";
#endif not lint

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include "tclInt.h"
#include "fd.h"

/*
 *----------------------------------------------------------------------
 *
 * Tcl_BreakCmd --
 *
 *	This procedure is invoked to process the "break" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_BreakCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    if (argc != 1) {
	sprintf(interp->result, "too many args: should be \"%.50s\"", argv[0]);
	return TCL_ERROR;
    }
    return TCL_BREAK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_CaseCmd --
 *
 *	This procedure is invoked to process the "case" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_CaseCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int i, result;
    int body;
    char *string;

    if (argc < 4) {
	sprintf(interp->result,
		"%s \"%.50s string [in] patList body ... [default body]\"",
		"not enough args:  should be", argv[0]);
	return TCL_ERROR;
    }
    string = argv[1];
    body = NULL;
    if (strcmp(argv[2], "in") == 0) {
	i = 3;
    } else {
	i = 2;
    }
    for (; i < argc; i += 2) {
	int patArgc, j;
	char **patArgv;
	register char *p;

	if (i == (argc-1)) {
	    sprintf(interp->result, "extra pattern with no body in \"%.50s\"",
		    argv[0]);
	    return TCL_ERROR;
	}

	/*
	 * Check for special case of single pattern (no list) with
	 * no backslash sequences.
	 */

	for (p = argv[i]; *p != 0; p++) {
	    if (isspace(*p) || (*p == '\\')) {
		break;
	    }
	}
	if (*p == 0) {
	    if ((*argv[i] == 'd') && (strcmp(argv[i], "default") == 0)) {
		body = i+1;
	    }
	    if (Tcl_StringMatch(string, argv[i])) {
		body = i+1;
		goto match;
	    }
	    continue;
	}

	/*
	 * Break up pattern lists, then check each of the patterns
	 * in the list.
	 */

	result = Tcl_SplitList(interp, argv[i], &patArgc, &patArgv);
	if (result != TCL_OK) {
	    return result;
	}
	for (j = 0; j < patArgc; j++) {
	    if (Tcl_StringMatch(string, patArgv[j])) {
		body = i+1;
		break;
	    }
	}
	free((char *) patArgv);
	if (j < patArgc) {
	    break;
	}
    }

    match:
    if (body != NULL) {
	result = Tcl_Eval(interp, argv[body], 0, (char **) NULL);
	if (result == TCL_ERROR) {
	    char msg[100];
	    sprintf(msg, " (\"%.50s\" arm line %d)", argv[i],
		    interp->errorLine);
	    Tcl_AddErrorInfo(interp, msg);
	}
	return result;
    }

    /*
     * Nothing matched:  return nothing.
     */
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_CatchCmd --
 *
 *	This procedure is invoked to process the "catch" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_CatchCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int result;

    if ((argc != 2) && (argc != 3)) {
	sprintf(interp->result,
		"wrong # args: should be \"%.50s command [varName]\"",
		argv[0]);
	return TCL_ERROR;
    }
    result = Tcl_Eval(interp, argv[1], 0, (char **) NULL);
    if (argc == 3) {
	Tcl_SetVar(interp, argv[2], interp->result, 0);
    }
    Tcl_Return(interp, (char *) NULL, TCL_STATIC);
    sprintf(interp->result, "%d", result);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_ConcatCmd --
 *
 *	This procedure is invoked to process the "concat" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_ConcatCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    if (argc == 1) {
	return TCL_OK;
    }

    interp->result = Tcl_Concat(argc-1, argv+1);
    interp->dynamic = 1;
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_ContinueCmd --
 *
 *	This procedure is invoked to process the "continue" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_ContinueCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    if (argc != 1) {
	sprintf(interp->result, "too many args: should be \"%.50s\"", argv[0]);
	return TCL_ERROR;
    }
    return TCL_CONTINUE;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_ErrorCmd --
 *
 *	This procedure is invoked to process the "error" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_ErrorCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    if (argc != 2) {
	sprintf(interp->result, "wrong # args: should be \"%.50s message\"",
		argv[0]);
	return TCL_ERROR;
    }
    Tcl_Return(interp, argv[1], TCL_VOLATILE);
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_EvalCmd --
 *
 *	This procedure is invoked to process the "eval" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_EvalCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int result;
    char *cmd;

    if (argc < 2) {
	return TCL_OK;
    }
    if (argc == 2) {
	result = Tcl_Eval(interp, argv[1], 0, (char **) NULL);
    } else {
    
	/*
	 * More than one argument:  concatenate them together with spaces
	 * between, then evaluate the result.
	 */
    
	cmd = Tcl_Concat(argc-1, argv+1);
	result = Tcl_Eval(interp, cmd, 0, (char **) NULL);
	free(cmd);
    }
    if (result == TCL_ERROR) {
	char msg[60];
	sprintf(msg, " (\"eval\" body line %d)", interp->errorLine);
	Tcl_AddErrorInfo(interp, msg);
    }
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_ExecCmd --
 *
 *	This procedure is invoked to process the "exec" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_ExecCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    extern void ExecHandlerProc();	/* Declared below. */
    extern int execSignalled;		/* Ditto. */
    char *input = "";			/* Points to the input remaining to
					 * send to the child process. */
    int inputSize;			/* Number of bytes left to send. */
    char *output = NULL;		/* Output received from child. */
    int outputSize;			/* Number of valid bytes at output. */
    int outputSpace;			/* Total space available at output. */
    int stdIn[2], stdOut[2], count, result, i, deadPid, maxID;
    int pid = 0;			/* 0 means child process doesn't
					 * exist (yet).  Non-zero gives its
					 * id. */
    void (*oldHandler)();
    int handlerSet = 0;
    /*88888888888*/    int status;
    char *cmdName;

    /*
     * Look through the arguments for a standard input specification
     * ("< value" in two arguments).  If found, collapse it out.
     * Shuffle all the arguments back over the "exec" argument, so that
     * there's room for a NULL argument at the end.
     */

    cmdName = argv[0];
    for (i = 1; i < argc; i++) {
	argv[i-1] = argv[i];
	if ((argv[i][0] != '<') || (argv[i][1] != 0)) {
	    continue;
	}
	i++;
	if (i >= argc) {
	    sprintf(interp->result,
		    "specified \"<\" but no input in \"%.50s\" command",
		    cmdName);
	    return TCL_ERROR;
	}
	input = argv[i];
	for (i++; i < argc; i++) {
	    argv[i-3] = argv[i];
	}
	argc -= 2;
    }

    argc -= 1;			/* Drop "exec" argument. */
    argv[argc] = NULL;
    if (argc < 1) {
	sprintf(interp->result, "not enough arguments to \"%.50s\" command",
		cmdName);
	return TCL_ERROR;
    }

    /*
     * Create pipes for standard input and standard output/error, and
     * start up the new process.
     */

    stdIn[0] = stdIn[1] = stdOut[0] = stdOut[1] = -1;
    if ((pipe(stdIn) < 0) || (pipe(stdOut) < 0)) {
	sprintf(interp->result, "couldn't create pipes for \"%.50s\" command",
		cmdName);
	result = TCL_ERROR;
	goto cleanup;
    }
    maxID = stdIn[1];
    if (stdOut[0] > maxID) {
	maxID = stdOut[0];
    }
    pid = fork();
    if (pid == -1) {
	sprintf(interp->result,
		"couldn't fork child for \"%.50s\" command: %.50s",
		cmdName, strerror(errno));
	result = TCL_ERROR;
	goto cleanup;
    }
    if (pid == 0) {
	char errSpace[100];

	if ((dup2(stdIn[0], 0) == -1) || (dup2(stdOut[1], 1) == -1)
		|| (dup2(stdOut[1], 2) == -1)) {
	    char *err;
	    err = "forked process couldn't set up input/output";
	    write(stdOut[1], err, strlen(err));
	    _exit(1);
	}
	close(stdIn[0]);
	close(stdIn[1]);
	close(stdOut[0]);
	close(stdOut[1]);
	execvp(argv[0], argv);
	sprintf(errSpace, "couldn't find a \"%.50s\" to execute", argv[0]);
	write(1, errSpace, strlen(errSpace));
	_exit(1);
    }

    /*
     * In the parent, arrange to be signalled when the child dies, then
     * funnel input and output to/from the process.
     */

    inputSize = strlen(input);
    outputSize = 0;
    outputSpace = 0;
    
    /*8888 added by rania*/ 
    
    #define	F_DUPFD		0	/* Duplicate fildes */
    #define	F_GETFD		1	/* Get fildes flags */
    #define	F_SETFD		2	/* Set fildes flags */
    #define	F_GETFL		3	/* Get file flags */
    #define	F_SETFL		4	/* Set file flags */

    /*8888 added by rania*/ 


    if ((fcntl(stdIn[1], F_SETFL, FNDELAY) == -1)
	    || (fcntl(stdOut[0], F_SETFL, FNDELAY) == -1)) {
	ioSetupError:
	sprintf(interp->result,
		"couldn't set up I/O to/from child in \"%.50s\": %.50s",
		cmdName, strerror(errno));
	result = TCL_ERROR;
	goto cleanup;
    }
    oldHandler = signal(SIGCHLD, ExecHandlerProc);
    if (((int) oldHandler) == -1) {
	goto ioSetupError;
    }
    handlerSet = 1;
    result = -1;
    while (1) {
	static struct timeval timeout = {2, 0};
	fd_set readMask, writeMask;

	/* 
	 * Pass input to the child.
	 */

	if (inputSize > 0) {
	    count = inputSize;
	    if (count > 4096) {
		count = 4096;
	    }
	    count = write(stdIn[1], input, count);
	    if (count < 0) {
		if ((errno != EWOULDBLOCK) && (errno != EAGAIN)) {
		    sprintf(interp->result,
			    "error writing stdin during \"%.50s\": %.50s",
			    cmdName, strerror(errno));
		    result = TCL_ERROR;
		    goto cleanup;
		}
	    } else {
		input += count;
		inputSize -= count;
	    }
	}
	if ((inputSize == 0) && (stdIn[1] >= 0)) {
	    close(stdIn[1]);
	    stdIn[1] = -1;
	}

	/*
	 * See if the child has completed.
	 */

	execSignalled = 0;
	deadPid = wait3(&status, WNOHANG, (struct rusage *) 0);
	if (deadPid == pid) {
	  /*888888888*/ result = status;
	}

	/*
	 * Check for output from the child.  Note that this will always
	 * be done once after the child has died, to collect any remaining
	 * output.  Repeatedly read output until there isn't any more to
	 * read.
	 */

	while (1) {
	    if ((outputSpace - outputSize) < 100) {
		char *newOutput;
    
		if (outputSpace == 0) {
		    outputSpace = 200;
		} else {
		    outputSpace = 2*outputSpace;
		}
		newOutput = (char *) malloc((unsigned) outputSpace);
		if (output != 0) {
		    bcopy(output, newOutput, outputSize);
		    free(output);
		}
		output = newOutput;
	    }
	    count = read(stdOut[0], output+outputSize,
		    outputSpace-outputSize-1);

	    /*
	     * The checks below look funny because the "no data ready"
	     * condition is reported differently by different OS'es:
	     * BSD -	count == -1, errno == EWOULDBLOCK
	     * SYSV -	count == -1, errno == EAGAIN
	     * HPUX -	count == 0, errno == 0
	     */

	    if (count <= 0) {
		if ((errno == EWOULDBLOCK) || (errno == EAGAIN)
			|| (count == 0)) {
		    break;
		}
		sprintf(interp->result,
			"error reading stdout during \"%.50s\": %.50s",
			cmdName, strerror(errno));
		result = TCL_ERROR;
		goto cleanup;
	    } else {
		outputSize += count;
	    }
	}

	/*
	 * Wait for something more to happen.
	 */

	FD_ZERO(&readMask);
	FD_ZERO(&writeMask);
	FD_SET(stdOut[0], &readMask);
	if (stdIn[1] != -1) {
	    FD_SET(stdIn[1], &writeMask);
	}
	if (result != -1) {
	    pid = 0;
	    break;
	}
	if (!execSignalled) {
	    (void) select(maxID+1, (int *) &readMask, (int *) &writeMask,
		    (int *) NULL, &timeout);
	}
    }
    output[outputSize] = 0;
    interp->result = output;
    interp->dynamic = 1;

    cleanup:
    if (pid != 0) {
	kill(pid, SIGQUIT);
    }
    if (stdIn[0] != -1) {
	close(stdIn[0]);
    }
    if (stdIn[1] != -1) {
	close(stdIn[1]);
    }
    if (stdOut[0] != -1) {
	close(stdOut[0]);
    }
    if (stdOut[1] != -1) {
	close(stdOut[1]);
    }
    if (handlerSet) {
	signal(SIGCHLD, oldHandler);
    }
    return result;
}

/*
 * Variable for communication between Tcl_ShellCmd and ShellHandlerProc:
 * non-zero means a signal has arrived.
 */

int execSignalled;

/*
 * Procedure to receive signals during "exec" command:  just return.
 */

void
ExecHandlerProc()
{
    execSignalled = 1;
    return;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_ExprCmd --
 *
 *	This procedure is invoked to process the "expr" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_ExprCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int result, value;

    if (argc != 2) {
	sprintf(interp->result,
		"wrong # args: should be \"%.50s expression\"", argv[0]);
	return TCL_ERROR;
    }

    result = Tcl_Expr(interp, argv[1], &value);
    if (result != TCL_OK) {
	return result;
    }

    /*
     * Turn the integer result back into a string.
     */

    sprintf(interp->result, "%d", value);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_FileCmd --
 *
 *	This procedure is invoked to process the "file" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_FileCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    char *p;
    int length, mode, statOp;
    struct stat statBuf;

    if (argc != 3) {
	sprintf(interp->result,
		"wrong # args: should be \"%.50s name option\"", argv[0]);
	return TCL_ERROR;
    }
    length = strlen(argv[2]);

    /*
     * First handle operations on the file name.
     */

    if ((argv[2][0] == 'd') && (strncmp(argv[2], "dirname", length) == 0)) {
	p = rindex(argv[1], '/');
	if (p == NULL) {
	    interp->result = ".";
	} else if (p == argv[1]) {
	    interp->result = "/";
	} else {
	    *p = 0;
	    Tcl_Return(interp, argv[1], TCL_VOLATILE);
	    *p = '/';
	}
	return TCL_OK;
    } else if ((argv[2][0] == 'r') && (length >= 2)
	    && (strncmp(argv[2], "rootname", length) == 0)) {
	p = rindex(argv[1], '.');
	if (p == NULL) {
	    Tcl_Return(interp, argv[1], TCL_VOLATILE);
	} else {
	    *p = 0;
	    Tcl_Return(interp, argv[1], TCL_VOLATILE);
	    *p = '.';
	}
	return TCL_OK;
    } else if ((argv[2][0] == 'e') && (length >= 3)
	    && (strncmp(argv[2], "extension", length) == 0)) {
	p = rindex(argv[1], '.');
	if (p != NULL) {
	    Tcl_Return(interp, p, TCL_VOLATILE);
	}
	return TCL_OK;
    } else if ((argv[2][0] == 't') && (strncmp(argv[2], "tail", length) == 0)) {
	p = rindex(argv[1], '/');
	if (p != NULL) {
	    Tcl_Return(interp, p+1, TCL_VOLATILE);
	} else {
	    Tcl_Return(interp, argv[1], TCL_VOLATILE);
	}
	return TCL_OK;
    }

    /*
     * Next, handle operations that can be satisfied with the "access"
     * kernel call.
     */

    /*8888888888 added by me*/
    #define R_OK 4
    #define W_OK 2
    #define X_OK 1
    #define vfork fork
    #define F_OK    0       /* Test for existence of File */    

    /*8888888888*/


    if ((argv[2][0] == 'r') && (length >= 2)
	    && (strncmp(argv[2], "readable", length) == 0)) {
	mode = R_OK;
	checkAccess:
	if (access(argv[1], mode) == -1) {
	    interp->result = "0";
	} else {
	    interp->result = "1";
	}
	return TCL_OK;
    } else if ((argv[2][0] == 'w')
	    && (strncmp(argv[2], "writable", length) == 0)) {
	mode = W_OK;
	goto checkAccess;
    } else if ((argv[2][0] == 'e') && (length >= 3)
	    && (strncmp(argv[2], "executable", length) == 0)) {
	mode = X_OK;
	goto checkAccess;
    } else if ((argv[2][0] == 'e') && (length >= 3)
	    && (strncmp(argv[2], "exists", length) == 0)) {
	mode = F_OK;
	goto checkAccess;
    }

    /*
     * Lastly, check stuff that requires the file to be stat-ed.
     */

    if ((argv[2][0] == 'o') && (strncmp(argv[2], "owned", length) == 0)) {
	statOp = 0;
    } else if ((argv[2][0] == 'i') && (length >= 3)
	    && (strncmp(argv[2], "isfile", length) == 0)) {
	statOp = 1;
    } else if ((argv[2][0] == 'i') && (length >= 3)
	    && (strncmp(argv[2], "isdirectory", length) == 0)) {
	statOp = 2;
    } else {
	sprintf(interp->result, "bad \"%.30s\" option \"%.30s\": must be dirname, executable, exists, extension, isdirectory, isfile, owned, readable, root, tail, or writable",
		argv[0], argv[2]);
	return TCL_ERROR;
    }
    if (stat(argv[1], &statBuf) == -1) {
	interp->result = "0";
	return TCL_OK;
    }
    switch (statOp) {
	case 0:
	    mode = (geteuid() == statBuf.st_uid);
	    break;
	case 1:
	    mode = (statBuf.st_mode & S_IFMT) == S_IFREG;
	    break;
	case 2:
	    mode = (statBuf.st_mode & S_IFMT) == S_IFDIR;
	    break;
    }
    if (mode) {
	interp->result = "1";
    } else {
	interp->result = "0";
    }
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_ForCmd --
 *
 *	This procedure is invoked to process the "for" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_ForCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int result, value;

    if (argc != 5) {
	sprintf(interp->result,
		"wrong # args: should be \"%.50s start test next command\"",
		argv[0]);
	return TCL_ERROR;
    }

    result = Tcl_Eval(interp, argv[1], 0, (char **) NULL);
    if (result != TCL_OK) {
	if (result == TCL_ERROR) {
	    Tcl_AddErrorInfo(interp, " (\"for\" initial command)");
	}
	return result;
    }
    while (1) {
	result = Tcl_Expr(interp, argv[2], &value);
	if (result != TCL_OK) {
	    return result;
	}
	if (!value) {
	    break;
	}
	result = Tcl_Eval(interp, argv[4], 0, (char **) NULL);
	if (result == TCL_CONTINUE) {
	    result = TCL_OK;
	} else if (result != TCL_OK) {
	    if (result == TCL_ERROR) {
		char msg[60];
		sprintf(msg, " (\"for\" body line %d)", interp->errorLine);
		Tcl_AddErrorInfo(interp, msg);
	    }
	    break;
	}
	result = Tcl_Eval(interp, argv[3], 0, (char **) NULL);
	if (result == TCL_BREAK) {
	    break;
	} else if (result != TCL_OK) {
	    if (result == TCL_ERROR) {
		Tcl_AddErrorInfo(interp, " (\"for\" loop-end command)");
	    }
	    return result;
	}
    }
    if (result == TCL_BREAK) {
	result = TCL_OK;
    }
    if (result == TCL_OK) {
	Tcl_Return(interp, (char *) NULL, TCL_STATIC);
    }
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_ForeachCmd --
 *
 *	This procedure is invoked to process the "foreach" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_ForeachCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int listArgc, i, result;
    char **listArgv;

    if (argc != 4) {
	sprintf(interp->result,
		"wrong # args: should be \"%.50s varName list command\"",
		argv[0]);
	return TCL_ERROR;
    }

    /*
     * Break the list up into elements, and execute the command once
     * for each value of the element.
     */

    result = Tcl_SplitList(interp, argv[2], &listArgc, &listArgv);
    if (result != TCL_OK) {
	return result;
    }
    for (i = 0; i < listArgc; i++) {
	Tcl_SetVar(interp, argv[1], listArgv[i], 0);

	result = Tcl_Eval(interp, argv[3], 0, (char **) NULL);
	if (result != TCL_OK) {
	    if (result == TCL_CONTINUE) {
		result = TCL_OK;
	    } else if (result == TCL_BREAK) {
		result = TCL_OK;
		break;
	    } else if (result == TCL_ERROR) {
		char msg[100];
		sprintf(msg, " (\"foreach\" body line %d)", interp->errorLine);
		Tcl_AddErrorInfo(interp, msg);
		break;
	    } else {
		break;
	    }
	}
    }
    free((char *) listArgv);
    if (result == TCL_OK) {
	Tcl_Return(interp, (char *) NULL, TCL_STATIC);
    }
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_FormatCmd --
 *
 *	This procedure is invoked to process the "format" Tcl command.
 *	See the user documentation for details on what it does.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */

	/* ARGSUSED */
int
Tcl_FormatCmd(dummy, interp, argc, argv)
    ClientData dummy;			/* Not used. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    register char *format;	/* Used to read characters from the format
				 * string. */
    char newFormat[40];		/* A new format specifier is generated here. */
    int width;			/* Field width from field specifier, or 0 if
				 * no width given. */
    int precision;		/* Field precision from field specifier, or 0
				 * if no precision given. */
    int size;			/* Number of bytes needed for result of
				 * conversion, based on type of conversion
				 * ("e", "s", etc.) and width from above. */
    char *oneWordValue;		/* Used to hold value to pass to sprintf, if
				 * it's a one-word value. */
    double twoWordValue;	/* Used to hold value to pass to sprintf if
				 * it's a two-word value. */
    int useTwoWords;		/* 0 means use oneWordValue, 1 means use
				 * twoWordValue. */
    char *dst = interp->result;	/* Where result is stored.  Starts off at
				 * interp->resultSpace, but may get dynamically
				 * re-allocated if this isn't enough. */
    int dstSize = 0;		/* Number of non-null characters currently
				 * stored at dst. */
    int dstSpace = TCL_RESULT_SIZE;
				/* Total amount of storage space available
				 * in dst (not including null terminator. */
    int noPercent;		/* Special case for speed:  indicates there's
				 * no field specifier, just a string to copy. */
    char **curArg;		/* Remainder of argv array. */

    /*
     * This procedure is a bit nasty.  The goal is to use sprintf to
     * do most of the dirty work.  There are several problems:
     * 1. this procedure can't trust its arguments.
     * 2. we must be able to provide a large enough result area to hold
     *    whatever's generated.  This is hard to estimate.
     * 2. there's no way to move the arguments from argv to the call
     *    to sprintf in a reasonable way.  This is particularly nasty
     *    because some of the arguments may be two-word values (doubles).
     * So, what happens here is to scan the format string one % group
     * at a time, making many individual calls to sprintf.
     */

    if (argc < 2) {
	sprintf(interp->result,
		"too few args: should be \"%.50s formatString arg arg ...\"",
		argv[0]);
	return TCL_ERROR;
    }
    curArg = argv+2;
    argc -= 2;
    for (format = argv[1]; *format != 0; ) {
	register char *newPtr = newFormat;

	width = precision = useTwoWords = noPercent = 0;

	/*
	 * Get rid of any characters before the next field specifier.
	 * Collapse backslash sequences found along the way.
	 */

	if (*format != '%') {
	    register char *p;
	    int bsSize;

	    oneWordValue = format;
	    for (p = format; (*format != '%') && (*format != 0); p++) {
		if (*format == '\\') {
		    *p = Tcl_Backslash(format, &bsSize);
		    format += bsSize;
		} else {
		    *p = *format;
		    format++;
		}
	    }
	    size = p - oneWordValue;
	    noPercent = 1;
	    goto doField;
	}

	if (format[1] == '%') {
	    oneWordValue = format;
	    size = 1;
	    noPercent = 1;
	    format += 2;
	    goto doField;
	}

	/*
	 * Parse off a field specifier, compute how many characters
	 * will be needed to store the result, and substitute for
	 * "*" size specifiers.
	 */

	*newPtr = '%';
	newPtr++;
	format++;
	if (*format == '-') {
	    *newPtr = '-';
	    newPtr++;
	    format++;
	}
	if (*format == '0') {
	    *newPtr = '0';
	    newPtr++;
	    format++;
	}
	if (isdigit(*format)) {
	    width = atoi(format);
	    do {
		format++;
	    } while (isdigit(*format));
	} else if (*format == '*') {
	    if (argc <= 0) {
		goto notEnoughArgs;
	    }
	    width = atoi(*curArg);
	    argc--;
	    curArg++;
	    format++;
	}
	if (width != 0) {
	    sprintf(newPtr, "%d", width);
	    while (*newPtr != 0) {
		newPtr++;
	    }
	}
	if (*format == '.') {
	    *newPtr = '.';
	    newPtr++;
	    format++;
	}
	if (isdigit(*format)) {
	    precision = atoi(format);
	    do {
		format++;
	    } while (isdigit(*format));
	} else if (*format == '*') {
	    if (argc <= 0) {
		goto notEnoughArgs;
	    }
	    precision = atoi(*curArg);
	    argc--;
	    curArg++;
	    format++;
	}
	if (precision != 0) {
	    sprintf(newPtr, "%d", precision);
	    while (*newPtr != 0) {
		newPtr++;
	    }
	}
	if (*format == '#') {
	    *newPtr = '#';
	    newPtr++;
	    format++;
	}
	if (*format == 'l') {
	    format++;
	}
	*newPtr = *format;
	newPtr++;
	*newPtr = 0;
	if (argc <= 0) {
	    goto notEnoughArgs;
	}
	switch (*format) {
	    case 'D':
	    case 'd':
	    case 'O':
	    case 'o':
	    case 'X':
	    case 'x':
	    case 'U':
	    case 'u': {
		char *end;

		oneWordValue = (char *) strtol(*curArg, &end, 0);
		if ((*curArg == 0) || (*end != 0)) {
		    sprintf(interp->result,
			    "expected integer but got \"%.50s\" instead",
			    *curArg);
		    goto fmtError;
		}
		size = 40;
		break;
	    }
	    case 's':
		oneWordValue = *curArg;
		size = strlen(*curArg);
		break;
	    case 'c': {
		char *end;

		oneWordValue = (char *) strtol(*curArg, &end, 0);
		if ((*curArg == 0) || (*end != 0)) {
		    sprintf(interp->result,
			    "expected integer but got \"%.50s\" instead",
			    *curArg);
		    goto fmtError;
		}
		size = 1;
		break;
	    }
	    case 'F':
	    case 'f':
	    case 'E':
	    case 'e':
	    case 'G':
	    case 'g':
		if (sscanf(*curArg, "%F", &twoWordValue) != 1) {
		    sprintf(interp->result,
			    "expected floating-point number but got \"%.50s\" instead",
			    *curArg);
		    goto fmtError;
		}
		useTwoWords = 1;
		size = 320;
		if (precision > 10) {
		    size += precision;
		}
		break;
	    case 0:
		interp->result = "format string ended in middle of field specifier";
		goto fmtError;
	    default:
		sprintf(interp->result, "bad field specifier \"%c\"", *format);
		goto fmtError;
	}
	argc--;
	curArg++;
	format++;

	/*
	 * Make sure that there's enough space to hold the formatted
	 * result, then format it.
	 */

	doField:
	if (width > size) {
	    size = width;
	}
	if ((dstSize + size) > dstSpace) {
	    char *newDst;
	    int newSpace;

	    newSpace = 2*(dstSize + size);
	    newDst = (char *) malloc((unsigned) newSpace+1);
	    if (dstSize != 0) {
		bcopy(dst, newDst, dstSize);
	    }
	    if (dstSpace != TCL_RESULT_SIZE) {
		free(dst);
	    }
	    dst = newDst;
	    dstSpace = newSpace;
	}
	if (noPercent) {
	    bcopy(oneWordValue, dst+dstSize, size);
	    dstSize += size;
	    dst[dstSize] = 0;
	} else {
	    if (useTwoWords) {
		sprintf(dst+dstSize, newFormat, twoWordValue);
	    } else {
		sprintf(dst+dstSize, newFormat, oneWordValue);
	    }
	    dstSize += strlen(dst+dstSize);
	}
    }

    interp->result = dst;
    interp->dynamic = !(dstSpace == TCL_RESULT_SIZE);
    return TCL_OK;

    notEnoughArgs:
    sprintf(interp->result,
	    "invoked \"%.50s\" without enough arguments", argv[0]);
    fmtError:
    if (dstSpace != TCL_RESULT_SIZE) {
	free(dst);
    }
    return TCL_ERROR;
}

/*
 * ----------------------------------------------------------------------------
 *
 * Tcl_GlobCmd --
 *
 *      Expands a pattern in a directory using csh rules.
 *	
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *      See the user documentation.
 *
 * ----------------------------------------------------------------------------
 */

	 /* ARGSUSED */
int
Tcl_GlobCmd(dummy, interp, argc, argv)
    ClientData dummy;                   /* Not used. */
    Tcl_Interp *interp;                 /* Current interpreter. */
    int argc;                           /* Number of arguments. */
    char **argv;                        /* Argument strings. */
{
    if (argc == 1) {
	return TCL_OK;
    }

    return Tcl_Glob(interp, argc, argv);
}
