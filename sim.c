/* 
 * sim.c --
 *
 *	This file contains a simple Tcl-based simulator for an abridged
 *	version of the MIPS DLX architecture.
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
static char rcsid[] = "$Header: /user1/ouster/mipsim/RCS/sim.c,v 1.18 89/11/20 10:56:51 ouster Exp $ SPRITE (Berkeley)";
#endif /* not lint */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>
#include <tcl.h>
#include "asm.h"
#include "dlx.h"
#include "sym.h"

/*
 * The table below is used to translate bits 31:26 of the instruction
 * into a value suitable for the "opCode" field of a MemWord structure,
 * or into a special value for further decoding.
 */

#define SPECIAL 120
#define FPARITH	121

#define IFMT 1
#define JFMT 2
#define RFMT 3

typedef struct {
    int opCode;		/* Translated op code. */
    int format;		/* Format type (IFMT or JFMT or RFMT) */
} OpInfo;

OpInfo opTable[] = {
    {SPECIAL, RFMT}, {FPARITH, RFMT}, {OP_J, JFMT}, {OP_JAL, JFMT},
    {OP_BEQZ, IFMT}, {OP_BNEZ, IFMT}, {OP_BFPT, IFMT}, {OP_BFPF, IFMT},
    {OP_ADDI, IFMT}, {OP_ADDUI, IFMT}, {OP_SUBI, IFMT}, {OP_SUBUI, IFMT},
    {OP_ANDI, IFMT}, {OP_ORI, IFMT}, {OP_XORI, IFMT}, {OP_LHI, IFMT},
    {OP_RFE, IFMT}, {OP_TRAP, IFMT}, {OP_JR, IFMT}, {OP_JALR, IFMT},
    {OP_SLLI, IFMT}, {OP_RES, IFMT}, {OP_SRLI, IFMT}, {OP_SRAI, IFMT},
    {OP_SEQI, IFMT}, {OP_SNEI, IFMT}, {OP_SLTI, IFMT}, {OP_SGTI, IFMT},
    {OP_SLEI, IFMT}, {OP_SGEI, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT},
    {OP_LB, IFMT}, {OP_LH, IFMT}, {OP_RES, IFMT}, {OP_LW, IFMT},
    {OP_LBU, IFMT}, {OP_LHU, IFMT}, {OP_LF, IFMT}, {OP_LD, IFMT},
    {OP_SB, IFMT}, {OP_SH, IFMT}, {OP_RES, IFMT}, {OP_SW, IFMT},
    {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_SF, IFMT}, {OP_SD, IFMT},
    {OP_SEQUI, IFMT}, {OP_SNEUI, IFMT}, {OP_SLTUI, IFMT}, {OP_SGTUI, IFMT},
    {OP_SLEUI, IFMT}, {OP_SGEUI, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT},
    {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT},
    {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT}, {OP_RES, IFMT}
};

/*
 * the table below is used to convert the "funct" field of SPECIAL
 * instructions into the "opCode" field of a MemWord.
 */

int specialTable[] = {
    OP_NOP, OP_RES, OP_RES, OP_RES, OP_SLL, OP_RES, OP_SRL, OP_SRA,
    OP_RES, OP_RES, OP_RES, OP_RES, OP_TRAP, OP_RES, OP_RES, OP_RES,
    OP_SEQU, OP_SNEU, OP_SLTU, OP_SGTU, OP_SLEU, OP_SGEU, OP_RES, OP_RES,
    OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES,
    OP_ADD, OP_ADDU, OP_SUB, OP_SUBU, OP_AND, OP_OR, OP_XOR, OP_RES,
    OP_SEQ, OP_SNE, OP_SLT, OP_SGT, OP_SLE, OP_SGE, OP_RES, OP_RES,
    OP_MOVI2S, OP_MOVS2I, OP_MOVF, OP_MOVD, OP_MOVFP2I, OP_MOVI2FP,
    OP_RES, OP_RES,
    OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES
};

/*
 * the table below is used to convert the "funct" field of FPARITH
 * instructions into the "opCode" field of a MemWord.
 */
int FParithTable[] = {
  OP_ADDF, OP_SUBF, OP_MULTF, OP_DIVF, OP_ADDD, OP_SUBD, OP_MULTD, OP_DIVD,
  OP_CVTF2D, OP_CVTF2I, OP_CVTD2F, OP_CVTD2I, OP_CVTI2F, OP_CVTI2D, 
  OP_MULT, OP_DIV,
  OP_EQF, OP_NEF, OP_LTF, OP_GTF, OP_LEF, OP_GEF, OP_MULTU, OP_DIVU,
  OP_EQD, OP_NED, OP_LTD, OP_GTD, OP_LED, OP_GED, OP_RES, OP_RES,
  OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES,
  OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES,
  OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES,
  OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES, OP_RES
  };
  
char *operationNames[] = {
    "", "ADD", "ADDI", "ADDU", "ADDUI", "AND", "ANDI", "BEQZ", "BFPF",
    "BFPT", "BNEZ", "J", "JAL", "JALR", "JR", "LB", "LBU", "LD", "LF",
    "LH", "LHI", "LHU", "LW", "MOVD", "MOVF", "MOVFP2I", "MOVI2FP",
    "MOVI2S", "MOVS2I", "OR", "ORI", "RFE", "SB", "SD", "SEQ",  "SEQI",
    "SEQU", "SEQUI",
    "SF", "SGE", "SGEI", "SGEU", "SGEUI", "SGT", "SGTI", "SGTU", "SGTUI",
    "SH", "SLE", "SLEI", "SLEU", "SLEUI", "SLL",
    "SLLI", "SLT", "SLTI", "SLTU", "SLTUI", "SNE", "SNEI", "SNEU", "SNEUI",
    "SRA", "SRAI", "SRL",
    "SRLI", "SUB", "SUBI", "SUBU", "SUBUI", "SW", "TRAP", "XOR", "XORI",
    "NOP", "", "", "","", "",

    /* There must be at least one empty string between the last integer operation
     *	and the first floating point operation.	*/

    "ADDD", "ADDF", "CVTD2F", "CVTD2I", "CVTF2D", "CVTF2I", "CVTI2D",
    "CVTI2F", "DIV", "DIVD", "DIVF", "DIVU", "EQD", "EQF", "GED", "GEF",
    "GTD", "GTF", "LED", "LEF", "LTD", "LTF", "MULT", "MULTD", "MULTF",
    "MULTU", "NED", "NEF", "SUBD", "SUBF", ""
    };

/*
 * The following value is used to handle virtually all special cases
 * while simulating.  The simulator normally executes in a fast path
 * where it ignores all special cases.  However, after executing each
 * instruction it checks the current serial number (total # of instructions
 * executed) agains the value below.  If that serial number has been
 * executed, then the simulator pauses to check for all possible special
 * conditions (stops, callbacks, errors, etc.).  Thus anyone that wants
 * to get a special condition handled must be sure to set checkNum below
 * so that the special-check code will be executed.  This facility means
 * that the simulator only has to check a single condition in its fast
 * path.
 */

static int checkNum;

/*
 * Forward declarations for procedures defined later in this file:
 */

static int	AddressError();
static int	BusError();
static void	Compile();
static int      Mult();
static int	Overflow();
static int	ReadMem();
static int	Simulate();
static int	WriteMem();
static int      FPissue ();
static void     FPwriteBack();
static void     FPwait();
static FPop	*mallocFPop();
static void	freeFPop();

/*
 *----------------------------------------------------------------------
 *
 * Sim_Create --
 *
 *	Create a description of an DLX machine.
 *
 * Results:
 *	The return value is a pointer to the description of the DLX
 *	machine.
 *
 * Side effects:
 *	The DLX structure gets allocated and initialized.  Several
 *	Tcl commands get registered for interp.
 *
 *----------------------------------------------------------------------
 */

DLX *
Sim_Create(memSize, interp, au, al, mu, ml, du, dl)
    int memSize;		/* Number of bytes of memory to be
				 * allocated for the machine. */
    Tcl_Interp *interp;		/* Interpreter to associate with machine. */
    int au, mu, du;		/* latencies for add, multiply, and divide */
    int al, ml, dl;		/* number of add, multiply, and divide units */
{
    register DLX *machPtr;
    register MemWord *wordPtr;
    int i;
    extern int Main_QuitCmd();

    machPtr = (DLX *) calloc(1, sizeof(DLX));
    machPtr->interp = interp;
    machPtr->numWords = (memSize+3) & ~0x3;
    machPtr->numChars = machPtr->numWords * 4;
    machPtr->memPtr = (MemWord *)
	    calloc(1, (unsigned) (sizeof(MemWord) * machPtr->numWords));
    for (i = machPtr->numWords, wordPtr = machPtr->memPtr; i > 0;
	    i--, wordPtr++) {
	wordPtr->value = 0;
	wordPtr->opCode = OP_NOT_COMPILED;
	wordPtr->stopList = NULL;
    }
    machPtr->memScratch = (char *) malloc (machPtr->numChars);
    machPtr->endScratch = machPtr->memScratch + machPtr->numChars;
    for (i = 0; i < NUM_GPRS; i++) {
	machPtr->regs[i] = 0;
    }
    machPtr->regs[PC_REG] = 0;
    machPtr->regs[NEXT_PC_REG] = 1;
    machPtr->badPC = 0;
    machPtr->addrErrNum = 0;
    machPtr->loadReg1 = 0;
    machPtr->loadValue1 = 0;
    machPtr->loadReg2 = 0;
    machPtr->loadValue2 = 0;
    machPtr->insCount = 0;
    machPtr->firstIns = 0;
    machPtr->branchSerial = -1;
    machPtr->branchPC = 0;
    machPtr->flags = 0;
    machPtr->stopNum = 1;
    machPtr->num_add_units = au;
    machPtr->num_mul_units = mu;
    machPtr->num_div_units = du;
    machPtr->fp_add_latency = al;
    machPtr->fp_mul_latency = ml;
    machPtr->fp_div_latency = dl;
    machPtr->stopList = NULL;
    machPtr->callBackList = NULL;
    Hash_InitTable(&machPtr->symbols, 0, HASH_STRING_KEYS);
    Io_Init(machPtr);
    Cop0_Init(machPtr);
    Init_Handle_Trap(machPtr);

    /* set up floating point stuff */
    machPtr->FPstatusReg = 0;
    for (i = 0; i < machPtr->num_add_units; i++) machPtr->fp_add_units[i] = 0;
    for (i = 0; i < machPtr->num_div_units; i++) machPtr->fp_div_units[i] = 0;
    for (i = 0; i < machPtr->num_div_units; i++) machPtr->fp_mul_units[i] = 0;
    machPtr->fp_units[FP_ADD] = machPtr->fp_add_units;
    machPtr->fp_units[FP_DIV] = machPtr->fp_div_units;
    machPtr->fp_units[FP_MUL] = machPtr->fp_mul_units;
    for (i = 0; i < 32; i++) machPtr->waiting_FPRs[i] = 0;
    machPtr->cycleCount = 0;
    machPtr->FPopsList = NULL;
    machPtr->checkFP = 0;
    machPtr->refTraceFile = NULL;

/* initialize counters */
    statsReset(machPtr);

    Tcl_CreateCommand(interp, "asm", Asm_AsmCmd, (ClientData) machPtr,
		      (void (*)()) NULL);
    Tcl_CreateCommand(interp, "fget", Gp_FGetCmd, (ClientData) machPtr,
		      (void (*)()) NULL);
    Tcl_CreateCommand(interp, "fput", Gp_FPutCmd, (ClientData) machPtr,
		      (void (*)()) NULL);
    Tcl_CreateCommand(interp, "get", Gp_GetCmd, (ClientData) machPtr,
		      (void (*)()) NULL);
    Tcl_CreateCommand(interp, "go", Sim_GoCmd, (ClientData) machPtr,
		      (void (*)()) NULL);
    Tcl_CreateCommand(interp, "load", Asm_LoadCmd, (ClientData) machPtr,
		      (void (*)()) NULL);
    Tcl_CreateCommand(interp, "put", Gp_PutCmd, (ClientData) machPtr,
		      (void (*)()) NULL);
    Tcl_CreateCommand(interp, "quit", Main_QuitCmd, (ClientData) machPtr,
		      (void (*)()) NULL);
    Tcl_CreateCommand(interp, "stats", Sim_DumpStats, (ClientData) machPtr,
		      (void (*)()) NULL);
    Tcl_CreateCommand(interp, "step", Sim_StepCmd, (ClientData) machPtr,
		      (void (*)()) NULL);
    Tcl_CreateCommand(interp, "stop", Stop_StopCmd, (ClientData) machPtr,
		      (void (*)()) NULL);
    Tcl_CreateCommand(interp, "trace", Sim_TraceCmd, (ClientData) machPtr,
		      (void (*)()) NULL);
    Sym_AddSymbol(machPtr, "", "memSize", memSize * 4, SYM_GLOBAL);

    return machPtr;
}

/*
 *----------------------------------------------------------------------
 * statsReset --
 *
 *	This procedure clears the various statistics which are kept on
 *	dynamic execution of the code.
 *
 * Results:
 *	None
 *
 * Side effects:
 *	Changes the related global variables.
 *
 *----------------------------------------------------------------------
 */

statsReset(machPtr)
     DLX *machPtr;                   /* machine description */
{
    int i;

    machPtr->loadStalls = 0;
    machPtr->FPstalls = 0;
    machPtr->branchYes = 0;
    machPtr->branchNo = 0;
    for (i = 0; i <= OP_LAST; i++)
	machPtr->operationCount[i] = 0;
}

/*
 *----------------------------------------------------------------------
 *
 * Sim_DumpStats --
 *
 *	This procedure is invoked to process the "stats" Tcl command.
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

int
Sim_DumpStats(machPtr, interp, argc, argv)
    DLX *machPtr;			/* Machine description. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    int i, j, intCount, floatCount;
    int doStalls = 0, doOpCount = 0, doFPpending = 0, 
        doBranch = 0, doHW = 0, doReset = 0;
    char *opName = *argv;

	/* default is to perform the ALL operation */
    if (argc == 1) {
	doStalls = doOpCount = doFPpending = doBranch = doHW = 1;
    } else while (++argv, --argc) {
	if (!strcmp(*argv, "reset") || !strcmp(*argv, "RESET")) {
	    doReset = 1;
	} else if (!strcmp(*argv, "stalls") || !strcmp(*argv, "STALLS")) {
	    doStalls = 1;
	} else if (!strcmp(*argv, "opcount") || !strcmp(*argv, "OPCOUNT")) {
	    doOpCount = 1;
	} else if (!strcmp(*argv, "pending") || !strcmp(*argv, "PENDING")) {
	    doFPpending = 1;
	} else if (!strcmp(*argv, "branch") || !strcmp(*argv, "BRANCH")) {
	    doBranch = 1;
	} else if (!strcmp(*argv, "hw") || !strcmp(*argv, "HW")) {
	    doHW = 1;
	} else if (!strcmp(*argv, "all") || !strcmp(*argv, "ALL")) {
		/* all prints everything, but does not reset */
	    doStalls = doOpCount = doFPpending = doBranch = doHW = 1;
	} else {
	    sprintf(interp->result,
		"illegal option:  should be \"%.50s\" [reset] [stalls] [opcount] [pending] [all]", opName);
	    return TCL_ERROR;
	}
    }

    if (doHW) {
      printf("\nMemory size: %d bytes.\n", machPtr->numWords * 4);
      printf("\nFloating Point Hardware Configuration\n");
      printf("%2d add/subtract units, latency = %2d cycles\n",
	     machPtr->num_add_units, machPtr->fp_add_latency);
      printf("%2d divide units,       latency = %2d cycles\n",
	     machPtr->num_div_units, machPtr->fp_div_latency);
      printf("%2d multiply units,     latency = %2d cycles\n",
	     machPtr->num_mul_units, machPtr->fp_mul_latency);
    }

    if (doStalls) {
	printf("Load Stalls = %d\n", machPtr->loadStalls);
	printf("Floating Point Stalls = %d\n", machPtr->FPstalls);
    }

    if (doBranch) {
      int total = machPtr->branchYes + machPtr->branchNo;
      if (!total) printf ("\nNo branch instructions executed.\n");
      else 
	printf ("\nBranches:  total %d, taken %d (%.2lf%%), untaken %d (%.2lf%%)\n", 
		total, machPtr->branchYes, 100.0 * machPtr->branchYes / total, 
		machPtr->branchNo, 100.0 * machPtr->branchNo / total);
    }

    if (doFPpending) {
	/* print fp unit status */
	printf ("\nPending Floating Point Operations:\n");
	if (machPtr->FPopsList == NULL) printf ("none.\n");
	else {
	  FPop *opPtr;
	  for (opPtr = machPtr->FPopsList; opPtr != NULL; opPtr = opPtr->nextPtr) {
	    switch (opPtr->type) {
	    case FP_ADD: printf ("adder            "); break;
	    case FP_SUB: printf ("adder (sub)      "); break;
	    case INT_UDIV:
	    case INT_DIV: printf("divider (int)    "); break;
	    case FP_DIV: printf ("divider          "); break;
	    case INT_MUL:
	    case INT_UMUL:printf("multiplier (int) "); break;
	    case FP_MUL: printf ("multiplier       "); break;
	    }
	    printf (" #%1d :  will complete in %2d more cycle(s)", opPtr->unit,
		    opPtr->ready - machPtr->cycleCount);
	    switch (opPtr->resultType) {
	    case FP_INT_OP:
	      printf("  0x%08lx ==> F%1d\n", *((int *)&(opPtr->result[0])), opPtr->dest);
	      break;
	    case FP_SINGLE_FP_OP:
	      printf("  %f ==> F%1d\n", *((float *)&(opPtr->result[0])), opPtr->dest);
	      break;
	    case FP_DOUBLE_FP_OP:
	      printf ("  %lf ==> F%1d:F%1d\n",
		      *((double *)&(opPtr->result[0])), opPtr->dest, opPtr->dest + 1);
	      break;
	    }
	  }
	}
    }

    if (doOpCount) {
	printf("\t\t\t\tINTEGER OPERATIONS\n");
	printf("\t\t\t\t==================\n");
	for (intCount = 0, i = 1; *operationNames[i]; i++) {
	    if (i % 4 == 1)
	        putchar('\n');
	    printf("%8s %8d  ", operationNames[i], machPtr->operationCount[i]);
	    intCount += machPtr->operationCount[i];
	}
	printf("\nTotal integer operations = %d\n\n", intCount);
	
	/* find the first floating pointer operation string */
	while (!*operationNames[i])
	    i++;

	printf("\t\t\tFLOATING POINT OPERATIONS\n");
	printf("\t\t\t=========================\n");
	for (floatCount = j = 0; *operationNames[i]; i++, j++) {
	    if (j % 4 == 0)
		putchar('\n');
	    printf("%8s %8d  ", operationNames[i], machPtr->operationCount[i]);
	    floatCount += machPtr->operationCount[i];
	}
	printf("\nTotal floating point operations = %d\n", floatCount);
	printf("Total operations = %d\n", intCount + floatCount);
	printf("Total cycles = %d\n", intCount + floatCount + 
	       machPtr->loadStalls + machPtr->FPstalls);
	interp->result[0] = '\0';
    }

    if (doReset) {
	statsReset(machPtr);
    }

    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * Sim_GoCmd --
 *
 *	This procedure is invoked to process the "go" Tcl command.
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

int
Sim_GoCmd(machPtr, interp, argc, argv)
    DLX *machPtr;			/* Machine description. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    if (argc > 2) {
	sprintf(interp->result,
		"too many args:  should be \"%.50s\" [address]", argv[0]);
	return TCL_ERROR;
    }

    if (argc == 2) {
	char *end;
	int newPc;

	if (Sym_EvalExpr(machPtr, (char *) NULL, argv[1], 0, &newPc, &end)
		!= TCL_OK) {
	    return TCL_ERROR;

	  }
	if ((*end != 0) || (newPc & 0x3)) {
	    sprintf(interp->result,
		    "\"%.50s\" isn't a valid starting address", argv[1]);
	    return TCL_ERROR;
	}
	machPtr->regs[PC_REG] = ADDR_TO_INDEX(newPc);
	machPtr->regs[NEXT_PC_REG] = machPtr->regs[PC_REG] + 1;
	machPtr->flags = 0;
	machPtr->badPC = 0;
    }

    return Simulate(machPtr, interp, 0);
}

/*
 *----------------------------------------------------------------------
 *
 * Sim_StepCmd --
 *
 *	This procedure is invoked to process the "step" Tcl command.
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

int
Sim_StepCmd(machPtr, interp, argc, argv)
    DLX *machPtr;			/* Machine description. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
    if (argc > 2) {
	sprintf(interp->result,
		"too many args:  should be \"%.50s\" [address]", argv[0]);
	return TCL_ERROR;
    }

    if (argc == 2) {
	char *end;
	int newPc;

	if (Sym_EvalExpr(machPtr, (char *) NULL, argv[1], 0, &newPc, &end)
		!= TCL_OK) {
	    return TCL_ERROR;
	}
	if ((*end != 0) || (newPc & 0x3)) {
	    sprintf(interp->result,
		    "\"%.50s\" isn't a valid address", argv[1]);
	    return TCL_ERROR;
	}
	machPtr->regs[PC_REG] = ADDR_TO_INDEX(newPc);
	machPtr->regs[NEXT_PC_REG] = machPtr->regs[PC_REG] + 1;
	machPtr->flags = 0;
	machPtr->badPC = 0;
    }

    return Simulate(machPtr, interp, 1);
}

/*
 *----------------------------------------------------------------------
 *
 * Sim_CallBack --
 *
 *	Arrange for a particular procedure to be invoked after a given
 *	number of instructions have been simulated.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	After numIns instructions have been executed, proc will be
 *	invoked in the following way:
 *
 *	void
 *	proc(clientData, machPtr)
 *	    ClientData clientData;
 *	    DLX *machPtr;
 *	{
 *	}
 *
 *	The clientData and machPtr arguments will be the same as those
 *	passed to this procedure.
 *----------------------------------------------------------------------
 */

void
Sim_CallBack(machPtr, numIns, proc, clientData)
    DLX *machPtr;		/* Machine of interest. */
    int numIns;			/* Call proc after this many instructions
				 * have been executed in machPtr. */
    void (*proc)();		/* Procedure to call. */
    ClientData clientData;	/* Arbitrary one-word value to pass to proc. */
{
    register CallBack *cbPtr;

    cbPtr = (CallBack *) calloc(1, sizeof(CallBack));
    cbPtr->serialNum = machPtr->insCount + numIns;
    cbPtr->proc = proc;
    cbPtr->clientData = clientData;
    if ((machPtr->callBackList == NULL) ||
	    (cbPtr->serialNum < machPtr->callBackList->serialNum)) {
	cbPtr->nextPtr = machPtr->callBackList;
	machPtr->callBackList = cbPtr;
    } else {
	register CallBack *cbPtr2;

	for (cbPtr2 = machPtr->callBackList; cbPtr2->nextPtr != NULL;
		cbPtr2 = cbPtr2->nextPtr) {
	    if (cbPtr->serialNum < cbPtr2->nextPtr->serialNum) {
		break;
	    }
	}
	cbPtr->nextPtr = cbPtr2->nextPtr;
	cbPtr2->nextPtr = cbPtr;
    }
    if (cbPtr->serialNum < checkNum) {
	checkNum = cbPtr->serialNum;
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Sim_Stop --
 *
 *	Arrange for the execution of the machine to stop after the
 *	current instruction.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The machine will stop executing (if it was executing in the
 *	first place).
 *
 *----------------------------------------------------------------------
 */

void
Sim_Stop(machPtr)
    DLX *machPtr;			/* Machine to stop. */
{
    machPtr->flags |= STOP_REQUESTED;
    checkNum = machPtr->insCount + 1;
}

/*
 *----------------------------------------------------------------------
 *
 * Sim_GetPC --
 *
 *	This procedure computes the current program counter for
 *	machPtr.
 *
 * Results:
 *	The return value is the current program counter for the
 *	machine.  This is a bit tricky to compute because the PC
 *	is stored as an index, and there may have been an unaligned
 *	value put in the PC.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

unsigned int
Sim_GetPC(machPtr)
    register DLX *machPtr;		/* Machine whose PC is wanted. */
{
    if ((machPtr->badPC != 0) && (machPtr->insCount >= machPtr->addrErrNum)) {
	return machPtr->badPC;
    }
    return INDEX_TO_ADDR(machPtr->regs[PC_REG]);
}

/*
 *----------------------------------------------------------------------
 *
 * ReadMem --
 *
 *	Read a word from DLX memory.
 *
 * Results:
 *	Under normal circumstances, the result is 1 and the word at
 *	*valuePtr is modified to contain the DLX word at the given
 *	address.  If no such memory address exists, or if a stop is
 *	set on the memory location, then 0 is returned to signify that
 *	simulation should stop.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
ReadMem(machPtr, address, valuePtr)
    register DLX *machPtr;	/* Machine whose memory is being read. */
    unsigned int address;	/* Desired word address. */
    int *valuePtr;		/* Store contents of given word here. */
{
    unsigned int index;
    int result;
    register MemWord *wordPtr;

    index = ADDR_TO_INDEX(address);
    if (index < machPtr->numWords) {
	wordPtr = &machPtr->memPtr[index];
	if ((wordPtr->stopList != NULL)
		&& (machPtr->insCount != machPtr->firstIns)) {
	    result = Stop_Execute(machPtr, wordPtr->stopList);
	    if ((result != TCL_OK) || (machPtr->flags & STOP_REQUESTED)) {
		return 0;
	    }
	}
	if (machPtr->refTraceFile)
	  fprintf( machPtr->refTraceFile, "0 %x\n", address );
	*valuePtr = wordPtr->value;
	return 1;
    }

    /*
     * The word isn't in the main memory.  See if it is an I/O
     * register.
     */

    if (Io_Read(machPtr, (address & ~0x3), valuePtr) == 1) {
	return 1;
    }

    /*
     * The word doesn't exist.  Register a bus error.  If interrupts
     * ever get implemented for bus errors, this code will have to
     * change a bit.
     */

    (void) BusError(machPtr, address, 0);
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * WriteMem --
 *
 *	Write a value into the DLX's memory.
 *
 * Results:
 *	If the write completed successfully then 1 is returned.  If
 *	any sort of problem occurred (such as an addressing error or
 *	a stop) then 0 is returned;  the caller should stop simulating.
 *
 * Side effects:
 *	The DLX memory gets updated with a new byte, halfword, or word
 *	value.
 *
 *----------------------------------------------------------------------
 */

static int
WriteMem(machPtr, address, size, value)
    register DLX *machPtr;	/* Machine whose memory is being read. */
    unsigned int address;	/* Desired word address. */
    int size;			/* Size to be written (1, 2, or 4 bytes). */
    int value;			/* New value to write into memory. */
{
    unsigned int index;
    int result;
    register MemWord *wordPtr;

    if (((size == 4) && (address & 0x3)) || ((size == 2) && (address & 0x1))) {
	(void) AddressError(machPtr, address, 0);
	return 0;
    }
    index = ADDR_TO_INDEX(address);
    if (index < machPtr->numWords) {
	wordPtr = &machPtr->memPtr[index];
	if ((wordPtr->stopList != NULL)
		&& (machPtr->insCount != machPtr->firstIns)) {
	    result = Stop_Execute(machPtr, wordPtr->stopList);
	    if ((result != TCL_OK) || (machPtr->flags & STOP_REQUESTED)) {
		return 0;
	    }
	}
	if (machPtr->refTraceFile)
	  fprintf(machPtr->refTraceFile, "1 %x\n", address);
	if (size == 4) {
	    wordPtr->value = value;
	} else if (size == 2) {
	    if (address & 0x2) {
		wordPtr->value = (wordPtr->value & 0xffff0000)
			| (value & 0xffff);
	    } else {
		wordPtr->value = (wordPtr->value & 0xffff)
			| (value << 16);
	    }
	} else {
	    switch (address & 0x3) {
		case 0:
		    wordPtr->value = (wordPtr->value & 0x00ffffff)
			    | (value << 24);
		    break;
		case 1:
		    wordPtr->value = (wordPtr->value & 0xff00ffff)
			    | ((value & 0xff) << 16);
			    break;
		case 2:
		    wordPtr->value = (wordPtr->value & 0xffff00ff)
			    | ((value & 0xff) << 8);
		    break;
		case 3:
		    wordPtr->value = (wordPtr->value & 0xffffff00)
			    | (value & 0xff);
		    break;
	    }
	}
	wordPtr->opCode = OP_NOT_COMPILED;
	return 1;
    }

    /*
     * Not in main memory.  See if it's an I/O device register.
     */

    if (Io_Write(machPtr, address, value, size) == 1) {
	return 1;
    }

    (void) BusError(machPtr, address, 0);
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * Simulate --
 *
 *	This procedure forms the core of the simulator.  It executes
 *	instructions until either a break occurs or an error occurs
 *	(or until a single instruction has been executed, if single-
 *	stepping is requested).
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	The state of *machPtr changes in response to the simulation.
 *	Return information may be left in *interp.
 *
 *----------------------------------------------------------------------
 */

#define Checker(x) ((lastLoad1 && (lastLoad1 == (x))) || (lastLoad2 && (lastLoad2 == (x))))
#define CheckerF(x) Checker((x) + 32)
#define CheckerD(x) (Checker((x) + 32) || Checker((x) + 33))
#define Check	if (stalled) {machPtr->loadStalls++; machPtr->cycleCount++;}
#define CheckS1	if (!stalled && Checker(wordPtr->rs1)) stalled = 1;
#define CheckS2	if (!stalled && Checker(wordPtr->rs2)) stalled = 1;
#define CheckD	if (!stalled && Checker(wordPtr->rd)) stalled = 1;
#define CheckS1F	if (!stalled && CheckerF(wordPtr->rs1)) stalled = 1;
#define CheckS2F	if (!stalled && CheckerF(wordPtr->rs2)) stalled = 1;
#define CheckDF	if (!stalled && CheckerF(wordPtr->rd)) stalled = 1;
#define CheckS1D	if (!stalled && CheckerD(wordPtr->rs1)) stalled = 1;
#define CheckS2D	if (!stalled && CheckerD(wordPtr->rs2)) stalled = 1;
#define CheckDD	if (!stalled && CheckerD(wordPtr->rd)) stalled = 1;


static int
Simulate(machPtr, interp, singleStep)
    register DLX *machPtr;		/* Machine description. */
    Tcl_Interp *interp;			/* Tcl interpreter, for results and
					 * break commands. */
    int singleStep;			/* Non-zero means execute exactly
					 * one instruction, regardless of
					 * breaks found. */
{
    register MemWord *wordPtr;		/* Memory word for instruction. */
    register unsigned int pc;		/* Current ins. address, then new
					 * nextPc value. */
    unsigned int tmp;
    int i, result;
    char *errMsg, msg[20];
    int errorValue;

    /*
     * Can't continue from an addressing error on the program counter.
     */

    if ((machPtr->badPC != 0) && (machPtr->addrErrNum == machPtr->insCount)) {
	sprintf(interp->result,
		"address error on instruction fetch, pc = 0x%x",
		machPtr->badPC);
	return TCL_ERROR;
    }

    machPtr->flags &= ~STOP_REQUESTED;
    machPtr->firstIns = machPtr->insCount;
    Io_BeginSim(machPtr);

    setCheckNum:
    if (machPtr->callBackList != NULL) {
	checkNum = machPtr->callBackList->serialNum;
    } else {
	checkNum = machPtr->insCount+100000;
    }
    if ((machPtr->badPC != 0) && (machPtr->addrErrNum > machPtr->insCount)) {
	if (checkNum > machPtr->addrErrNum) {
	    checkNum = machPtr->addrErrNum;
	}
    } else {
	machPtr->badPC = 0;
    }
    if (singleStep) {
	checkNum = machPtr->insCount+1;
    }
    while (1) {
	int last_pc, lastLoad1, lastLoad2, stalled, trapCaught;

	stalled = 0;
	trapCaught = 0;

	/* First handle a delayed laod */
	if (lastLoad1 = machPtr->loadReg1) {
	    machPtr->regs[lastLoad1] = machPtr->loadValue1;
	    machPtr->loadReg1 = 0;
	    if (lastLoad2 = machPtr->loadReg2) {
	        machPtr->regs[lastLoad2] = machPtr->loadValue2;
	        machPtr->loadReg2 = 0;
	    }
	} else lastLoad2 = 0;

	/*
	 * Fetch an instruction, and compute the new next pc (but don't
	 * store it yet, in case the instruction doesn't complete).
	 */
	pc = machPtr->regs[PC_REG];
	if (pc >= machPtr->numWords) {
	    result = BusError(machPtr, INDEX_TO_ADDR(pc), 1);
	    if (result != TCL_OK) {
		goto stopSimulation;
	    } else {
		goto endOfIns;
	    }
	}
	if (machPtr->refTraceFile)
	  fprintf(machPtr->refTraceFile, "2 %x\n", INDEX_TO_ADDR(pc));
	wordPtr = &machPtr->memPtr[pc];
	last_pc = pc;
	pc = machPtr->regs[NEXT_PC_REG]+1;

	/*
	 * Handle breaks on the instruction, if this isn't the first
	 * instruction executed.
	 */

	if ((wordPtr->stopList != NULL)
		&& (machPtr->insCount != machPtr->firstIns)) {
	    result = Stop_Execute(machPtr, wordPtr->stopList);
	    if ((result != TCL_OK) || (machPtr->flags & STOP_REQUESTED)) {
		goto stopSimulation;
	    }
	}

	/*
	 * Execute the instruction.
	 */

	execute:
	machPtr->operationCount[wordPtr->opCode]++;
	switch (wordPtr->opCode) {

	    case OP_ADD: {
		int sum;
		CheckS1 CheckS2 Check
		sum = machPtr->regs[wordPtr->rs1] + machPtr->regs[wordPtr->rs2];
		if (!((machPtr->regs[wordPtr->rs1] ^ machPtr->regs[wordPtr->rs2])
			& SIGN_BIT) && ((machPtr->regs[wordPtr->rs1]
			^ sum) & SIGN_BIT)) {
		    result = Overflow(machPtr);
		    if (result != TCL_OK) {
			goto stopSimulation;
		    } else {
			goto endOfIns;
		    }
		}
		machPtr->regs[wordPtr->rd] = sum;
		break;
	    }

	    case OP_ADDI: {
		int sum;
		CheckS1 Check
		sum = machPtr->regs[wordPtr->rs1] + wordPtr->extra;
		if (!((machPtr->regs[wordPtr->rs1] ^ wordPtr->extra)
			& SIGN_BIT) && ((wordPtr->extra ^ sum) & SIGN_BIT)) {
		    result = Overflow(machPtr);
		    if (result != TCL_OK) {
			goto stopSimulation;
		    } else {
			goto endOfIns;
		    }
		}
		machPtr->regs[wordPtr->rd] = sum;
		break;
	    }

	    case OP_ADDU:
		CheckS1 CheckS2 Check
		machPtr->regs[wordPtr->rd] = machPtr->regs[wordPtr->rs1]
			+ machPtr->regs[wordPtr->rs2];
		break;

	    case OP_ADDUI:
		CheckS1 Check
		machPtr->regs[wordPtr->rd] = machPtr->regs[wordPtr->rs1]
			+ (wordPtr->extra & 0xffff);
		break;

	    case OP_AND:
		CheckS1 CheckS2 Check
		machPtr->regs[wordPtr->rd] = machPtr->regs[wordPtr->rs1]
			& machPtr->regs[wordPtr->rs2];
		break;

	    case OP_ANDI:
		CheckS1 Check
		machPtr->regs[wordPtr->rd] = machPtr->regs[wordPtr->rs1]
			& (wordPtr->extra & 0xffff);
		break;

	    case OP_BEQZ:
	      CheckS1 Check
	      if (machPtr->regs[wordPtr->rs1] == 0) {
		pc = machPtr->regs[NEXT_PC_REG] + ADDR_TO_INDEX(wordPtr->extra);
		machPtr->branchYes++;
	      }
	      else machPtr->branchNo++;
	      machPtr->branchSerial = machPtr->insCount;
	      machPtr->branchPC = INDEX_TO_ADDR(machPtr->regs[PC_REG]);
	      break;

	    case OP_BFPF:
	      if (!(machPtr->FPstatusReg)) {
		pc = machPtr->regs[NEXT_PC_REG] + ADDR_TO_INDEX(wordPtr->extra);
		machPtr->branchYes++;
	      }
	      else machPtr->branchNo++;
	      machPtr->branchSerial = machPtr->insCount;
	      machPtr->branchPC = INDEX_TO_ADDR(machPtr->regs[PC_REG]);
	      break;

	    case OP_BFPT:
	      if (machPtr->FPstatusReg) {
		pc = machPtr->regs[NEXT_PC_REG] + ADDR_TO_INDEX(wordPtr->extra);
		machPtr->branchYes++;
	      }
	      else machPtr->branchNo++;
	      machPtr->branchSerial = machPtr->insCount;
	      machPtr->branchPC = INDEX_TO_ADDR(machPtr->regs[PC_REG]);
	      break;

	    case OP_BNEZ:
	      CheckS1 Check
	      if (machPtr->regs[wordPtr->rs1] != 0) {
		pc = machPtr->regs[NEXT_PC_REG] + ADDR_TO_INDEX(wordPtr->extra);
		machPtr->branchYes++;
	      }
	      else machPtr->branchNo++;
	      machPtr->branchSerial = machPtr->insCount;
	      machPtr->branchPC = INDEX_TO_ADDR(machPtr->regs[PC_REG]);
	      break;

	    case OP_JAL:
		machPtr->regs[R31] =
			INDEX_TO_ADDR(machPtr->regs[NEXT_PC_REG] + 1);
	    case OP_J:
		pc = last_pc + 1 + ADDR_TO_INDEX(wordPtr->extra);
		machPtr->branchSerial = machPtr->insCount;
		machPtr->branchPC = INDEX_TO_ADDR(machPtr->regs[PC_REG]);
		break;

	    case OP_JALR:
		machPtr->regs[R31] =
			INDEX_TO_ADDR(machPtr->regs[NEXT_PC_REG] + 1);
	    case OP_JR:
		CheckS1 Check
		tmp = machPtr->regs[wordPtr->rs1];
		pc = ADDR_TO_INDEX(tmp);
		if ((tmp & 0x3) && (machPtr->badPC == 0)) {
		    machPtr->badPC = tmp;
		    machPtr->addrErrNum = machPtr->insCount + 2;
		    if (checkNum > machPtr->addrErrNum) {
			checkNum = machPtr->addrErrNum;
		    }
		}
		machPtr->branchSerial = machPtr->insCount;
		machPtr->branchPC = INDEX_TO_ADDR(machPtr->regs[PC_REG]);
		break;

	    case OP_LB:
	    case OP_LBU: {
		int value;
		CheckS1 Check
		tmp = machPtr->regs[wordPtr->rs1] + wordPtr->extra;
		if (ReadMem(machPtr, tmp, &value) == 0) {
		    goto stopSimulation;
		}

		switch (tmp & 0x3) {
		    case 0:
			value >>= 24;
			break;
		    case 1:
			value >>= 16;
			break;
		    case 2:
			value >>= 8;
		}
		if ((value & 0x80) && (wordPtr->opCode == OP_LB)) {
		    value |= 0xffffff00;
		} else {
		    value &= 0xff;
		}
		machPtr->loadValue1 = value;
		machPtr->loadReg1 = wordPtr->rd;
		break;
	    }

	    case OP_LD: {
		int valueHi, valueLo;
		CheckS1 Check
		FPwait (machPtr, 2, wordPtr->rd, wordPtr->rd + 1, 0, 0);
		tmp = machPtr->regs[wordPtr->rs1] + wordPtr->extra;
		if (tmp & 0x3) {
		    result = AddressError(machPtr, tmp, 1);
		    if (result != TCL_OK) {
			goto stopSimulation;
		    } else {
			goto endOfIns;
		    }
		}
		if (ReadMem(machPtr, tmp, &valueHi) == 0) {
		  goto stopSimulation;
		}
		if (ReadMem(machPtr, tmp+4, &valueLo) == 0) {
		  goto stopSimulation;
		}
		machPtr->loadValue1 = valueHi;
		machPtr->loadValue2 = valueLo;
		machPtr->loadReg1 = wordPtr->rd + 32;
		machPtr->loadReg2 = wordPtr->rd + 32 + 1;
		break;
	      }

	    case OP_LF: {
		int value;
		CheckS1 Check
		FPwait (machPtr, 1, wordPtr->rd, 0, 0, 0);
		tmp = machPtr->regs[wordPtr->rs1] + wordPtr->extra;
		if (tmp & 0x3) {
		    result = AddressError(machPtr, tmp, 1);
		    if (result != TCL_OK) {
			goto stopSimulation;
		    } else {
			goto endOfIns;
		    }
		}
		if (ReadMem(machPtr, tmp, &value) == 0) {
		    goto stopSimulation;
		}
		machPtr->loadValue1 = value;
		machPtr->loadReg1 = wordPtr->rd + 32;
		break;
	      }

	    case OP_LH:
	    case OP_LHU: {
		int value;
		CheckS1 Check
		tmp = machPtr->regs[wordPtr->rs1] + wordPtr->extra;
		if (tmp & 0x1) {
		    result = AddressError(machPtr, tmp, 1);
		    if (result != TCL_OK) {
			goto stopSimulation;
		    } else {
			goto endOfIns;
		    }
		}
		if (ReadMem(machPtr, tmp, &value) == 0) {
		    goto stopSimulation;
		}
		if (!(tmp & 0x2)) {
		    value >>= 16;
		}
		if ((value & 0x8000) && (wordPtr->opCode == OP_LH)) {
		    value |= 0xffff0000;
		} else {
		    value &= 0xffff;
		}
		machPtr->loadValue1 = value;
		machPtr->loadReg1 = wordPtr->rd;
		break;
	    }

	    case OP_LHI:
		machPtr->regs[wordPtr->rd] = wordPtr->extra << 16;
		break;

	    case OP_LW: {
		int value;
		CheckS1 Check
		tmp = machPtr->regs[wordPtr->rs1] + wordPtr->extra;
		if (tmp & 0x3) {
		    result = AddressError(machPtr, tmp, 1);
		    if (result != TCL_OK) {
			goto stopSimulation;
		    } else {
			goto endOfIns;
		    }
		}
		if (ReadMem(machPtr, tmp, &value) == 0) {
		    goto stopSimulation;
		}
		machPtr->loadValue1 = value;
		machPtr->loadReg1 = wordPtr->rd;
		break;
	      }

	    case OP_MOVD: {
	      double *source, *dest;
	      CheckS1D Check
	      FPwait (machPtr, 4, wordPtr->rs1, wordPtr->rs1 + 1,
		      wordPtr->rd, wordPtr->rd + 1);
	      source = (double *) & (machPtr->regs[32 + wordPtr->rs1]);
	      dest = (double *) & (machPtr->regs[32 + wordPtr->rd]);
	      *dest = *source;
	      break;
	    }

	    case OP_MOVF: {
	      float *source, *dest;
	      CheckS1F Check
	      FPwait (machPtr, 2, wordPtr->rs1, wordPtr->rd, 0, 0);
	      source = (float *) & (machPtr->regs[32 + wordPtr->rs1]);
	      dest = (float *) & (machPtr->regs[32 + wordPtr->rd]);
	      *dest = *source;
	      break;
	    }

	    case OP_MOVFP2I:
	      CheckS1F Check
	      FPwait (machPtr, 1, wordPtr->rs1, 0, 0, 0);
	      machPtr->regs[wordPtr->rd] = machPtr->regs[32 + wordPtr->rs1];
	      break;

	    case OP_MOVI2FP:
	      CheckS1 Check
	      FPwait (machPtr, 1, wordPtr->rd, 0, 0, 0);
	      machPtr->regs[32 + wordPtr->rd] = machPtr->regs[wordPtr->rs1];
	      break;

	    case OP_MOVI2S:
		/* CheckS1 Check */
	        errMsg = "MOVI2S instruction is unimplemented";
	        goto error;

	    case OP_MOVS2I:
	        errMsg = "MOVS2I instruction is unimplemented";
	        goto error;

	    case OP_OR:
	      CheckS1 CheckS2 Check
		machPtr->regs[wordPtr->rd] = machPtr->regs[wordPtr->rs1]
			| machPtr->regs[wordPtr->rs2];
		break;

	    case OP_ORI:
	        CheckS1 Check
		machPtr->regs[wordPtr->rd] = machPtr->regs[wordPtr->rs1]
			| (wordPtr->extra & 0xffff);
		break;

	    case OP_RFE:
	        errMsg = "RFE instruction is unimplemented";
	        goto error;

	    case OP_SB:
		CheckS1 CheckD Check
		if (WriteMem(machPtr, (unsigned) (machPtr->regs[wordPtr->rs1]
				+ wordPtr->extra),
			1, machPtr->regs[wordPtr->rd]) == 0) {
		    goto stopSimulation;
		}
		break;

	    case OP_SD:
		CheckS1 CheckDD Check
	      FPwait (machPtr, 2, wordPtr->rd, wordPtr->rd + 1, 0, 0);
	      if (WriteMem(machPtr, (unsigned) (machPtr->regs[wordPtr->rs1]
						+ wordPtr->extra),
			   4, machPtr->regs[wordPtr->rd + 32]) == 0)
		goto stopSimulation;
	      if (WriteMem(machPtr, (unsigned) (machPtr->regs[wordPtr->rs1]
						+ wordPtr->extra + 4),
			   4, machPtr->regs[wordPtr->rd + 32 + 1]) == 0)
		goto stopSimulation;
	      break;


	    case OP_SEQ:
		CheckS1 CheckS2 Check
		if (machPtr->regs[wordPtr->rs1] == machPtr->regs[wordPtr->rs2]) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SEQI:
		CheckS1 Check
		if (machPtr->regs[wordPtr->rs1] == wordPtr->extra) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SEQU:
		CheckS1 CheckS2 Check
		if (((unsigned int) machPtr->regs[wordPtr->rs1]) ==
                    ((unsigned int) machPtr->regs[wordPtr->rs2])) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SEQUI:
		CheckS1 Check
		if (((unsigned int) machPtr->regs[wordPtr->rs1]) ==
                    ((unsigned int) wordPtr->extra)) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SF:
	      CheckS1 CheckDF Check
	      FPwait (machPtr, 1, wordPtr->rd, 0, 0, 0);
	      if (WriteMem(machPtr, (unsigned) (machPtr->regs[wordPtr->rs1]
						+ wordPtr->extra),
			   4, machPtr->regs[wordPtr->rd + 32]) == 0)
		goto stopSimulation;
	      break;

	    case OP_SGE:
		CheckS1 CheckS2 Check
		if (machPtr->regs[wordPtr->rs1] >= machPtr->regs[wordPtr->rs2]) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SGEI:
		CheckS1 Check
		if (machPtr->regs[wordPtr->rs1] >= wordPtr->extra) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SGEU:
		CheckS1 CheckS2 Check
		if (((unsigned int) machPtr->regs[wordPtr->rs1]) >=
                    ((unsigned int) machPtr->regs[wordPtr->rs2])) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SGEUI:
		CheckS1 Check
		if (((unsigned int) machPtr->regs[wordPtr->rs1]) >=
                    ((unsigned int) wordPtr->extra)) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

       	    case OP_SGT:
		CheckS1 CheckS2 Check
		if (machPtr->regs[wordPtr->rs1] > machPtr->regs[wordPtr->rs2]) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SGTI:
		CheckS1 Check
		if (machPtr->regs[wordPtr->rs1] > wordPtr->extra) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SGTU:
		CheckS1 CheckS2 Check
		if (((unsigned int) machPtr->regs[wordPtr->rs1]) >
                    ((unsigned int) machPtr->regs[wordPtr->rs2])) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SGTUI:
		CheckS1 Check
		if (((unsigned int) machPtr->regs[wordPtr->rs1]) >
                    ((unsigned int) wordPtr->extra)) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SH:
		CheckS1 CheckD Check
		if (WriteMem(machPtr, (unsigned) (machPtr->regs[wordPtr->rs1]
				+ wordPtr->extra),
			2, machPtr->regs[wordPtr->rd]) == 0) {
		    goto stopSimulation;
		}
		break;

	    case OP_SLE:
		CheckS1 CheckS2 Check
		if (machPtr->regs[wordPtr->rs1] <= machPtr->regs[wordPtr->rs2]) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SLEI:
		CheckS1 Check
		if (machPtr->regs[wordPtr->rs1] <= wordPtr->extra) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SLEU:
		CheckS1 CheckS2 Check
		if (((unsigned int) machPtr->regs[wordPtr->rs1]) <=
                    ((unsigned int) machPtr->regs[wordPtr->rs2])) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SLEUI:
		CheckS1 Check
		if (((unsigned int) machPtr->regs[wordPtr->rs1]) <=
                    ((unsigned int) wordPtr->extra)) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SLL:
		CheckS1 CheckS2 Check
		machPtr->regs[wordPtr->rd] = machPtr->regs[wordPtr->rs1]
			<< (machPtr->regs[wordPtr->rs2] & 0x1f);
		break;

	    case OP_SLLI:
		CheckS2 Check
		machPtr->regs[wordPtr->rd] = machPtr->regs[wordPtr->rs1]
			<< (wordPtr->extra & 0x1f);
		break;

	    case OP_SLT:
		CheckS1 CheckS2 Check
		if (machPtr->regs[wordPtr->rs1] < machPtr->regs[wordPtr->rs2]) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SLTI:
		CheckS1 Check
		if (machPtr->regs[wordPtr->rs1] < wordPtr->extra) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SLTU:
		CheckS1 CheckS2 Check
		if (((unsigned int) machPtr->regs[wordPtr->rs1]) <
                    ((unsigned int) machPtr->regs[wordPtr->rs2])) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SLTUI:
		CheckS1 Check
		if (((unsigned int) machPtr->regs[wordPtr->rs1]) <
                    ((unsigned int) wordPtr->extra)) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SNE:
		CheckS1 CheckS2 Check
		if (machPtr->regs[wordPtr->rs1] != machPtr->regs[wordPtr->rs2]) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SNEI:
		CheckS1 Check
		if (machPtr->regs[wordPtr->rs1] != wordPtr->extra) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SNEU:
		CheckS1 CheckS2 Check
		if (((unsigned int) machPtr->regs[wordPtr->rs1]) !=
                    ((unsigned int) machPtr->regs[wordPtr->rs2])) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SNEUI:
		CheckS1 Check
		if (((unsigned int) machPtr->regs[wordPtr->rs1]) !=
                    ((unsigned int) wordPtr->extra)) {
		    machPtr->regs[wordPtr->rd] = 1;
		} else {
		    machPtr->regs[wordPtr->rd] = 0;
		}
		break;

	    case OP_SRA:
		CheckS1 CheckS2 Check
		machPtr->regs[wordPtr->rd] = machPtr->regs[wordPtr->rs1]
			>> (machPtr->regs[wordPtr->rs2] & 0x1f);
		break;

	    case OP_SRAI:
	      CheckS2 Check
	      machPtr->regs[wordPtr->rd] = machPtr->regs[wordPtr->rs1]
		>> (wordPtr->extra & 0x1f);
	      break;

	    case OP_SRL:
	      CheckS1 CheckS2 Check
	      tmp = machPtr->regs[wordPtr->rs1];
	      tmp >>= (machPtr->regs[wordPtr->rs2] & 0x1f);
	      machPtr->regs[wordPtr->rd] = tmp;
	      break;

	    case OP_SRLI:
		CheckS2 Check
		tmp = machPtr->regs[wordPtr->rs1];
		tmp >>= (wordPtr->extra & 0x1f);
		machPtr->regs[wordPtr->rd] = tmp;
		break;

	    case OP_SUB: {
		int diff;

		CheckS1 CheckS2 Check
		diff = machPtr->regs[wordPtr->rs1] - machPtr->regs[wordPtr->rs2];
		if (((machPtr->regs[wordPtr->rs1] ^ machPtr->regs[wordPtr->rs2])
			& SIGN_BIT) && ((machPtr->regs[wordPtr->rs1]
			^ diff) & SIGN_BIT)) {
		    result = Overflow(machPtr);
		    if (result != TCL_OK) {
			goto stopSimulation;
		    } else {
			goto endOfIns;
		    }
		}
		machPtr->regs[wordPtr->rd] = diff;
		break;
	    }

	    case OP_SUBI: {
		int diff;

		CheckS1 Check
		diff = machPtr->regs[wordPtr->rs1] - wordPtr->extra;
		if (((machPtr->regs[wordPtr->rs1] ^ wordPtr->extra)
			& SIGN_BIT) && ((machPtr->regs[wordPtr->rs1]
			^ diff) & SIGN_BIT)) {
		    result = Overflow(machPtr);
		    if (result != TCL_OK) {
			goto stopSimulation;
		    } else {
			goto endOfIns;
		    }
		}
		machPtr->regs[wordPtr->rd] = diff;
		break;
	    }

	    case OP_SUBU:
		CheckS1 CheckS2 Check
		machPtr->regs[wordPtr->rd] = machPtr->regs[wordPtr->rs1]
			- machPtr->regs[wordPtr->rs2];
		break;

	    case OP_SUBUI:
		CheckS1 Check
		machPtr->regs[wordPtr->rd] = machPtr->regs[wordPtr->rs1]
			- (wordPtr->extra & 0xffff);
		break;

	    case OP_SW:
		CheckS1 CheckD Check
		if (WriteMem(machPtr, (unsigned) (machPtr->regs[wordPtr->rs1]
				+ wordPtr->extra),
			4, machPtr->regs[wordPtr->rd]) == 0) {
		    goto stopSimulation;
		}
		break;

	    case OP_TRAP:
	        switch (Handle_Trap(machPtr, (wordPtr->extra & 0x3ffffff))) {
		case 1 :
		  printf("TRAP #%d failed\n", wordPtr->extra & 0x3ffffff);
		  trapCaught = 1;
		  break;
		case 2 :
		  printf("TRAP #%d received\n", wordPtr->extra & 0x3ffffff);
		  trapCaught = 1;
		  break;
		}
		break;

	    case OP_XOR:
		CheckS1 CheckS2 Check
		machPtr->regs[wordPtr->rd] = machPtr->regs[wordPtr->rs1]
			^ machPtr->regs[wordPtr->rs2];
		break;

	    case OP_XORI:
		CheckS1 Check
		machPtr->regs[wordPtr->rd] = machPtr->regs[wordPtr->rs1]
			^ (wordPtr->extra & 0xffff);
		break;

        case OP_NOP:
          break; 

/* floating point operations */

	    case OP_DIV: {
	      int stalls;
	      CheckS1F CheckS2F Check
	      while (stalls = FPissue (machPtr, INT_DIV, FP_INT_OP,
				       wordPtr->rs1, wordPtr->rs2,
				       wordPtr->rd, &errorValue)) {
		machPtr->FPstalls += stalls;
		machPtr->cycleCount += stalls;
		FPwriteBack (machPtr);
	      }
	      if (errorValue) {
		errMsg = "divide by zero";
		goto error;
	      }
	      break;
	    }

	    case OP_DIVU: {
	      int stalls;
	      CheckS1F CheckS2F Check
	      while (stalls = FPissue (machPtr, INT_UDIV, FP_INT_OP,
				       wordPtr->rs1, wordPtr->rs2,
				       wordPtr->rd, &errorValue)) {
		machPtr->FPstalls += stalls;
		machPtr->cycleCount += stalls;
		FPwriteBack (machPtr);
	      }
	      if (errorValue) {
		errMsg = "divide by zero";
		goto error;
	      }
	      break;
	    }

	    case OP_MULT: {
	      int stalls;
	      CheckS1F CheckS2F Check
	      while (stalls = FPissue (machPtr, INT_MUL, FP_INT_OP,
				       wordPtr->rs1, wordPtr->rs2,
				       wordPtr->rd, &errorValue)) {
		machPtr->FPstalls += stalls;
		machPtr->cycleCount += stalls;
		FPwriteBack (machPtr);
	      }
	      if (errorValue) {
		printf("mult error ==> %d", errorValue);
		errMsg = "signed multiply overflow";
		goto error;
	      }
	      break;
	    }

	    case OP_MULTU: {
	      int stalls;
	      CheckS1F CheckS2F Check
	      while (stalls = FPissue (machPtr, INT_UMUL, FP_INT_OP,
				       wordPtr->rs1, wordPtr->rs2,
				       wordPtr->rd, &errorValue)) {
		machPtr->FPstalls += stalls;
		machPtr->cycleCount += stalls;
		FPwriteBack (machPtr);
	      }
	      if (errorValue) {
	        errMsg = "unsigned multiply overflow";
	        goto error;
	      }
	      break;
	    }

	    case OP_ADDD: {
	      int stalls;
	      CheckS1D CheckS2D Check
	      while (stalls = FPissue (machPtr, FP_ADD, FP_DOUBLE_FP_OP,
				       wordPtr->rs1, wordPtr->rs2,
				       wordPtr->rd, &errorValue)) {
		machPtr->FPstalls += stalls;
		machPtr->cycleCount += stalls;
		FPwriteBack (machPtr);
	      }
	      break;
	    }

	    case OP_ADDF: {
	      int stalls;
	      CheckS1F CheckS2F Check
	      while (stalls = FPissue (machPtr, FP_ADD, FP_SINGLE_FP_OP,
				       wordPtr->rs1, wordPtr->rs2,
				       wordPtr->rd, &errorValue)) {
		machPtr->FPstalls += stalls;
		machPtr->cycleCount += stalls;
		FPwriteBack (machPtr);
	      }
	      break;
	    }

	    case OP_CVTD2F: {
	      double *source;
	      float *dest;
	      CheckS1D Check
	      FPwait(machPtr, 3, wordPtr->rs1, wordPtr->rs1 + 1,
		     wordPtr->rd, 0);
	      source = (double *) & (machPtr->regs[32 + wordPtr->rs1]);
	      dest = (float *) & (machPtr->regs[32 + wordPtr->rd]);
	      *dest = (float) *source;
	      break;
	    }

	    case OP_CVTD2I: {
	      double *source;
	      int *dest;
	      CheckS1D Check
	      FPwait(machPtr, 3, wordPtr->rs1, wordPtr->rs1 + 1, 
		     wordPtr->rd, 0);
	      source = (double *) & (machPtr->regs[32 + wordPtr->rs1]);
	      dest = (int *) & (machPtr->regs[32 + wordPtr->rd]);
	      *dest = (int) *source;
	      break;
	    }

	    case OP_CVTF2D: {
	      float *source;
	      double *dest;
	      CheckS1F Check
	      FPwait(machPtr, 3, wordPtr->rs1,
		     wordPtr->rd, wordPtr->rd + 1, 0);
	      source = (float *) & (machPtr->regs[32 + wordPtr->rs1]);
	      dest = (double *) & (machPtr->regs[32 + wordPtr->rd]);
	      *dest = (double) *source;
	      break;
	    }
	      
	    case OP_CVTF2I: {
	      float *source;
	      int *dest;
	      CheckS1F Check
	      FPwait(machPtr, 2, wordPtr->rs1, wordPtr->rd, 0, 0);
	      source = (float *) & (machPtr->regs[32 + wordPtr->rs1]);
	      dest = (int *) & (machPtr->regs[32 + wordPtr->rd]);
	      *dest = (int) *source;
	      break;
	    }

	    case OP_CVTI2D: {
	      int *source;
	      double *dest;
	      CheckS1F Check
	      FPwait(machPtr, 3, wordPtr->rs1, wordPtr->rd, wordPtr->rd + 1, 0);
	      source = (int *) & (machPtr->regs[32 + wordPtr->rs1]);
	      dest = (double *) & (machPtr->regs[32 + wordPtr->rd]);
	      *dest = (double) *source;
	      break;
	    }
	    case OP_CVTI2F: {
	      int *source;
	      float *dest;
	      CheckS1F Check
	      FPwait(machPtr, 2, wordPtr->rs1, wordPtr->rd, 0, 0);
	      source = (int *) & (machPtr->regs[32 + wordPtr->rs1]);
	      dest = (float *) & (machPtr->regs[32 + wordPtr->rd]);
	      *dest = (float) *source;
	      break;
	    }

	    case OP_DIVD: {
	      int stalls;
	      CheckS1D CheckS2D Check
	      while (stalls = FPissue (machPtr, FP_DIV, FP_DOUBLE_FP_OP,
				       wordPtr->rs1, wordPtr->rs2,
				       wordPtr->rd, &errorValue)) {
		machPtr->FPstalls += stalls;
		machPtr->cycleCount += stalls;
		FPwriteBack (machPtr);
	      }
	      break;
	    }

	    case OP_DIVF: {
	      int stalls;
	      CheckS1F CheckS2F Check
	      while (stalls = FPissue (machPtr, FP_DIV, FP_SINGLE_FP_OP,
				       wordPtr->rs1, wordPtr->rs2,
				       wordPtr->rd, &errorValue)) {
		machPtr->FPstalls += stalls;
		machPtr->cycleCount += stalls;
		FPwriteBack (machPtr);
	      }
	      break;
	    }

	    case OP_EQD: {
	      double *source1, *source2;
	      CheckS1D CheckS2D Check
	      FPwait(machPtr, 4, wordPtr->rs1, wordPtr->rs1 + 1,
		     wordPtr->rs2, wordPtr->rs2 + 1);
	      source1 = (double *) & (machPtr->regs[32 + wordPtr->rs1]);
	      source2 = (double *) & (machPtr->regs[32 + wordPtr->rs2]);
	      machPtr->FPstatusReg = (*source1 == *source2);
	      break;
	    }

	    case OP_EQF: {
	      float *source1, *source2;
	      CheckS1F CheckS2F Check
	      FPwait(machPtr, 2, wordPtr->rs1, wordPtr->rs2, 0, 0);
	      source1 = (float *) & (machPtr->regs[32 + wordPtr->rs1]);
	      source2 = (float *) & (machPtr->regs[32 + wordPtr->rs2]);
	      machPtr->FPstatusReg = (*source1 == *source2);
	      break;
	    }

	    case OP_GED: {
	      double *source1, *source2;
	      CheckS1D CheckS2D Check
	      FPwait(machPtr, 4, wordPtr->rs1, wordPtr->rs1 + 1,
		     wordPtr->rs2, wordPtr->rs2 + 1);
	      source1 = (double *) & (machPtr->regs[32 + wordPtr->rs1]);
	      source2 = (double *) & (machPtr->regs[32 + wordPtr->rs2]);
	      machPtr->FPstatusReg = (*source1 >= *source2);
	      break;
	    }

	    case OP_GEF: {
	      float *source1, *source2;
	      CheckS1F CheckS2F Check
	      FPwait(machPtr, 2, wordPtr->rs1, wordPtr->rs2, 0, 0);
	      source1 = (float *) & (machPtr->regs[32 + wordPtr->rs1]);
	      source2 = (float *) & (machPtr->regs[32 + wordPtr->rs2]);
	      machPtr->FPstatusReg = (*source1 >= *source2);
	      break;
	    }

	    case OP_GTD: {
	      double *source1, *source2;
	      CheckS1D CheckS2D Check
	      FPwait(machPtr, 4, wordPtr->rs1, wordPtr->rs1 + 1,
		     wordPtr->rs2, wordPtr->rs2 + 1);
	      source1 = (double *) & (machPtr->regs[32 + wordPtr->rs1]);
	      source2 = (double *) & (machPtr->regs[32 + wordPtr->rs2]);
	      machPtr->FPstatusReg = (*source1 > *source2);
	      break;
	    }

	    case OP_GTF: {
	      float *source1, *source2;
	      CheckS1F CheckS2F Check
	      FPwait(machPtr, 2, wordPtr->rs1, wordPtr->rs2, 0, 0);
	      source1 = (float *) & (machPtr->regs[32 + wordPtr->rs1]);
	      source2 = (float *) & (machPtr->regs[32 + wordPtr->rs2]);
	      machPtr->FPstatusReg = (*source1 > *source2);
	      break;
	    }

	    case OP_LED: {
	      double *source1, *source2;
	      CheckS1D CheckS2D Check
	      FPwait(machPtr, 4, wordPtr->rs1, wordPtr->rs1 + 1,
		     wordPtr->rs2, wordPtr->rs2 + 1);
	      source1 = (double *) & (machPtr->regs[32 + wordPtr->rs1]);
	      source2 = (double *) & (machPtr->regs[32 + wordPtr->rs2]);
	      machPtr->FPstatusReg = (*source1 <= *source2);
	      break;
	    }

	    case OP_LEF: {
	      float *source1, *source2;
	      CheckS1F CheckS2F Check
	      FPwait(machPtr, 2, wordPtr->rs1, wordPtr->rs2, 0, 0);
	      source1 = (float *) & (machPtr->regs[32 + wordPtr->rs1]);
	      source2 = (float *) & (machPtr->regs[32 + wordPtr->rs2]);
	      machPtr->FPstatusReg = (*source1 <= *source2);
	      break;
	    }

	    case OP_LTD: {
	      double *source1, *source2;
	      CheckS1D CheckS2D Check
	      FPwait(machPtr, 4, wordPtr->rs1, wordPtr->rs1 + 1,
		     wordPtr->rs2, wordPtr->rs2 + 1);
	      source1 = (double *) & (machPtr->regs[32 + wordPtr->rs1]);
	      source2 = (double *) & (machPtr->regs[32 + wordPtr->rs2]);
	      machPtr->FPstatusReg = (*source1 < *source2);
	      break;
	    }

	    case OP_LTF: {
	      float *source1, *source2;
	      CheckS1F CheckS2F Check
	      FPwait(machPtr, 2, wordPtr->rs1, wordPtr->rs2, 0, 0);
	      source1 = (float *) & (machPtr->regs[32 + wordPtr->rs1]);
	      source2 = (float *) & (machPtr->regs[32 + wordPtr->rs2]);
	      machPtr->FPstatusReg = (*source1 < *source2);
	      break;
	    }

	    case OP_MULTD: {
	      int stalls;
	      CheckS1D CheckS2D Check
	      while (stalls = FPissue (machPtr, FP_MUL, FP_DOUBLE_FP_OP,
				       wordPtr->rs1, wordPtr->rs2,
				       wordPtr->rd, &errorValue)) {
		machPtr->FPstalls += stalls;
		machPtr->cycleCount += stalls;
		FPwriteBack (machPtr);
	      }
	      break;
	    }

	    case OP_MULTF: {
	      int stalls;
	      CheckS1F CheckS2F Check
	      while (stalls = FPissue (machPtr, FP_MUL, FP_SINGLE_FP_OP,
				       wordPtr->rs1, wordPtr->rs2,
				       wordPtr->rd, &errorValue)) {
		machPtr->FPstalls += stalls;
		machPtr->cycleCount += stalls;
		FPwriteBack (machPtr);
	      }
	      break;
	    }

	    case OP_NED: {
	      double *source1, *source2;
	      CheckS1D CheckS2D Check
	      FPwait(machPtr, 4, wordPtr->rs1, wordPtr->rs1 + 1,
		     wordPtr->rs2, wordPtr->rs2 + 1);
	      source1 = (double *) & (machPtr->regs[32 + wordPtr->rs1]);
	      source2 = (double *) & (machPtr->regs[32 + wordPtr->rs2]);
	      machPtr->FPstatusReg = (*source1 != *source2);
	      break;
	    }

	    case OP_NEF: {
	      float *source1, *source2;
	      CheckS1F CheckS2F Check
	      FPwait(machPtr, 2, wordPtr->rs1, wordPtr->rs2, 0, 0);
	      source1 = (float *) & (machPtr->regs[32 + wordPtr->rs1]);
	      source2 = (float *) & (machPtr->regs[32 + wordPtr->rs2]);
	      machPtr->FPstatusReg = (*source1 != *source2);
	      break;
	    }

	    case OP_SUBD: {
	      int stalls;
	      CheckS1D CheckS2D Check
	      while (stalls = FPissue (machPtr, FP_SUB, FP_DOUBLE_FP_OP,
				       wordPtr->rs1, wordPtr->rs2,
				       wordPtr->rd, &errorValue)) {
		machPtr->FPstalls += stalls;
		machPtr->cycleCount += stalls;
		FPwriteBack (machPtr);
	      }
	      break;
	    }

	    case OP_SUBF: {
	      int stalls;
	      CheckS1F CheckS2F Check
	      while (stalls = FPissue (machPtr, FP_SUB, FP_SINGLE_FP_OP,
				       wordPtr->rs1, wordPtr->rs2,
				       wordPtr->rd, &errorValue)) {
		machPtr->FPstalls += stalls;
		machPtr->cycleCount += stalls;
		FPwriteBack (machPtr);
	      }
	      break;
	    }

	    case OP_NOT_COMPILED:
		Compile(wordPtr);
		goto execute;
		break;

	    case OP_RES:
		errMsg = "reserved operation";
		goto error;

	    case OP_UNIMP:
		errMsg = "instruction not implemented in simulator";
		goto error;

	    default:
		i = wordPtr->opCode;
		sprintf(interp->result,
			"internal error in Simulate():  bad opCode %d, pc = %.100s",
			i, Sym_GetString(machPtr, Sim_GetPC(machPtr)));
		result = TCL_ERROR;
		goto stopSimulation;
	    }

	/*
	 * Monitor cycleCount and FP units
	 */
	
	machPtr->cycleCount++;

    /*
     * if we flush we do dis.
     */
    machPtr->cycleCount+=branchNo + branchYes;

    if (machPtr->checkFP && machPtr->cycleCount >= machPtr->checkFP)
	  FPwriteBack (machPtr);
	/* re-zero the cycle count periodically to avoid wrap around probs */
	if (machPtr->cycleCount >= CYC_CNT_RESET) {
	  int i;
	  FPop *opPtr = machPtr->FPopsList;
	  for (i = 0; i < 32; i++)
	    if (machPtr->waiting_FPRs[i])
	      machPtr->waiting_FPRs[i] -= CYC_CNT_RESET - 5;
	  machPtr->cycleCount -= CYC_CNT_RESET - 5;
	  if (machPtr->checkFP)
	    machPtr->checkFP -= CYC_CNT_RESET - 5;
	  while (opPtr != NULL) {
	    opPtr->ready -= CYC_CNT_RESET - 5;
	    opPtr = opPtr->nextPtr;
	  }
	}
	  
	/*
	 * Make sure R0 stays zero.
	 */
    
	machPtr->regs[0] = 0;
    
	/*
	 * Advance program counters.
	 */
    
	machPtr->regs[PC_REG] = machPtr->regs[NEXT_PC_REG];
	machPtr->regs[NEXT_PC_REG] = pc;

	/*
	 * Check flags for special actions to perform after the instruction.
	 */

	if (trapCaught) {
	    result = TCL_OK;
	    goto stopSimulation;
	}
	if ((machPtr->insCount += 1) >= checkNum) {
	    while (machPtr->callBackList != NULL) {
		register CallBack *cbPtr;

		cbPtr = machPtr->callBackList;
		if (machPtr->insCount < cbPtr->serialNum) {
		    break;
		}
		machPtr->callBackList = cbPtr->nextPtr;
		(*cbPtr->proc)(cbPtr->clientData, machPtr);
		free((char *) cbPtr);
	    }
	    if ((machPtr->badPC != 0)
		    && (machPtr->insCount == machPtr->addrErrNum)) {
		result = AddressError(machPtr, machPtr->badPC, 1);
		if (result != TCL_OK) {
		    goto stopSimulation;
		}
	    }
	    if (singleStep) {
		tmp = Sim_GetPC(machPtr);
		sprintf(interp->result,
			"stopped after single step, pc = %.100s: %.50s",
			Sym_GetString(machPtr, tmp),
			Asm_Disassemble(machPtr,
				machPtr->memPtr[machPtr->regs[PC_REG]].value,
				tmp & ~0x3));
		result = TCL_OK;
		goto stopSimulation;
	    }
	    if (machPtr->flags & STOP_REQUESTED) {
		errMsg = "execution stopped";
		goto error;
	    }
	    goto setCheckNum;
	}

	endOfIns:
	continue;
    }

    error:
    tmp = Sim_GetPC(machPtr);
    sprintf(interp->result, "%s, pc = %.50s: %.60s", errMsg,
	    Sym_GetString(machPtr, tmp),
	    Asm_Disassemble(machPtr,
		    machPtr->memPtr[machPtr->regs[PC_REG]].value, tmp & ~0x3));
    result = TCL_ERROR;

    /*
     * Before returning, store the current instruction serial number
     * in a Tcl variable.
     */

    stopSimulation:
    Io_EndSim(machPtr);
    sprintf(msg, "%d", machPtr->insCount);
    Tcl_SetVar(machPtr->interp, "insCount", msg, 1);
    if (machPtr->refTraceFile)
      fflush(machPtr->refTraceFile);
    return result;
}

/*
 *----------------------------------------------------------------------
 *
 * Compile --
 *
 *	Given a memory word, decode it into a set of fields that
 *	permit faster interpretation.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The contents of *wordPtr are modified.
 *
 *----------------------------------------------------------------------
 */

static void
Compile(wordPtr)
    register MemWord *wordPtr;		/* Memory location to be compiled. */
{
    register OpInfo *opPtr;

    wordPtr->rs1 = (wordPtr->value >> 21) & 0x1f;
    wordPtr->rs2 = (wordPtr->value >> 16) & 0x1f;
    wordPtr->rd = (wordPtr->value >> 11) & 0x1f;
    opPtr = &opTable[(wordPtr->value >> 26) & 0x3f];
    wordPtr->opCode = opPtr->opCode;
    if (opPtr->format == IFMT) {
        wordPtr->rd = wordPtr->rs2;
	wordPtr->extra = wordPtr->value & 0xffff;
	if (wordPtr->extra & 0x8000) {
	    wordPtr->extra |= 0xffff0000;
	}
    } else if (opPtr->format == RFMT) {
	wordPtr->extra = (wordPtr->value >> 6) & 0x1f;
    } else {
	wordPtr->extra = wordPtr->value & 0x3ffffff;
	if (wordPtr->extra & 0x2000000) {
	    wordPtr->extra |= 0xfc000000;
	}
    }
    if (wordPtr->opCode == SPECIAL) {
      wordPtr->opCode = specialTable[wordPtr->value & 0x3f];
    }
    else if (wordPtr->opCode == FPARITH) {
      wordPtr->opCode = FParithTable[wordPtr->value & 0x3f];
    }
  }

/*
 *----------------------------------------------------------------------
 *
 * BusError --
 *
 *	Handle bus error execptions.
 *
 * Results:
 *	A standard Tcl return value.  If TCL_OK, then there is no return
 *	string and it's safe to keep on simulating.
 *
 * Side effects:
 *	May simulate a trap for the machine.
 *
 *----------------------------------------------------------------------
 */

/* ARGSUSED */
static int
BusError(machPtr, address, iFetch)
    DLX *machPtr;		/* Machine description. */
    unsigned int address;	/* Location that was referenced but doesn't
				 * exist. */
    int iFetch;			/* 1 means error occurred during instruction
				 * fetch.  0 means during a load or store. */
{
    unsigned int pcAddr;

    pcAddr = Sim_GetPC(machPtr);
    if (iFetch) {
	sprintf(machPtr->interp->result,
		"bus error: tried to fetch instruction at 0x%x",
		address);
    } else {
	sprintf(machPtr->interp->result,
		"bus error: referenced 0x%x during load or store, pc = %.100s: %s",
		address, Sym_GetString(machPtr, pcAddr),
		Asm_Disassemble(machPtr,
			machPtr->memPtr[machPtr->regs[PC_REG]].value,
			pcAddr & ~0x3));
    }
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * AddressError --
 *
 *	Handle address error execptions.
 *
 * Results:
 *	A standard Tcl return value.  If TCL_OK, then there is no return
 *	string and it's safe to keep on simulating.
 *
 * Side effects:
 *	May simulate a trap for the machine.
 *
 *----------------------------------------------------------------------
 */

/* ARGSUSED */
static int
AddressError(machPtr, address, load)
    DLX *machPtr;		/* Machine description. */
    unsigned int address;	/* Location that was referenced but doesn't
				 * exist. */
    int load;			/* 1 means error occurred during instruction
				 * fetch or load, 0 means during a store. */
{
    sprintf(machPtr->interp->result,
	    "address error:  referenced 0x%x, pc = %.100s", address,
	    Sym_GetString(machPtr, Sim_GetPC(machPtr)));
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * Overflow --
 *
 *	Handle arithmetic overflow execptions.
 *
 * Results:
 *	A standard Tcl return value.  If TCL_OK, then there is no return
 *	string and it's safe to keep on simulating.
 *
 * Side effects:
 *	May simulate a trap for the machine.
 *
 *----------------------------------------------------------------------
 */

/* ARGSUSED */
static int
Overflow(machPtr)
    DLX *machPtr;		/* Machine description. */
{
    unsigned int pcAddr;

    pcAddr = Sim_GetPC(machPtr);
    sprintf(machPtr->interp->result, "arithmetic overflow, pc = %.100s: %s",
	    Sym_GetString(machPtr, pcAddr),  Asm_Disassemble(machPtr,
		    machPtr->memPtr[machPtr->regs[PC_REG]].value,
		    pcAddr & ~0x3));
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * Mult --
 *
 *	Simulate DLX multiplication.
 *
 * Results:
 *      The word at *resultPtr is overwritten with the result
 *      of the multiplication and 0 is returned if there was no
 *      overflow.  On overflow, a negative integer is returned
 *      and *resultPtr is not changed.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
Mult(a, b, signedArith, resultPtr)
    int a, b;			/* Two operands to multiply. */
    int signedArith;		/* Zero means perform unsigned arithmetic,
				 * one means perform signed arithmetic. */
     int *resultPtr;            /* Put result here. */
{
    unsigned int aHi, aLo, bHi, bLo, res1, res2, res3, res4, res5;
    unsigned int *resPtr;
    int negative;

    if ((a == 0) || (b == 0)) {
	*resultPtr = 0;
	return 0;
    }

    /*
     * Compute the sign of the result, then make everything positive
     * so unsigned computation can be done in the main computation.
     */

    negative = 0;
    if (signedArith) {
	if (a < 0) {
	    negative = !negative;
	    a = -a;
	}
	if (b < 0) {
	    negative = !negative;
	    b = -b;
	}
    }

    /*
     * Compute the result in unsigned arithmetic (check a's bits one at
     * a time, and add in a shifted value of b).
     */

    aLo = (unsigned int) (a & 0xffff);
    aHi = ((unsigned int) a) >> 16;
    bLo = (unsigned int) (b & 0xffff);
    bHi = ((unsigned int) b) >> 16;

    /* printf ("\naHi %d aLo %d bHi %d bLo %d\n", aHi, aLo, bHi, bLo); */

    if (aHi && bHi) return -1;

    res1 = aLo * bLo;
    res2 = aHi * bLo;
    res3 = aLo * bHi;
    if ((res2 | res3) & 0xffff0000) return -2;
    
    res2 <<= 16;
    res3 <<= 16;
    if ((res2 & res3) & 0x80000000) return -3;
    
    res4 = res2 + res3;
    if (((res2 | res3) & 0x80000000) && (!(res4 & 0x80000000))) return -4;

    if ((res1 & res4) & 0x80000000) return -5;
    res5 = res1 + res4;
    if (((res1 | res4) & 0x80000000) && (!(res5 & 0x80000000))) return -6;

    /*
     * If the result is supposed to be negative, compute the two's
     * complement of the double-word result.
     */

    if (negative) {
        if ((res5 & 0x80000000) && (res5 & 0x7fffffff)) return -7;
	else res5 = (~res5) + 1;
      }
    
    resPtr = (unsigned int *) resultPtr;
    *resPtr = res5;
    return 0;
}

/*
 *----------------------------------------------------------------------
 *
 * FPissue --
 *
 *      Initiate a pending floating point operation.
 *
 * Results:
 *	Returns 0 if the instruction was successfully issued.
 *      The following hazards prevent issuing:
 *           RAW    (source operand is destination of a pending FP op)
 *           WAW    (destination operand is destination of a pending FP op)
 *           structural (a proper type of FP unit is not available).
 *      In any of these cases, an integer is returned which indicates how
 *      many stall cycles occur before the hazard condition has ended.
 *      If multiple hazards apply, the integer returned indicates when the
 *      first hazard condition has ended; others may still be present.
 *
 * Side effects:
 *      In the case of successful issue, the list of pending floating 
 *      point operations is updated.  After the appropriate latency,
 *      FPwriteBack will be invoked.
 *
 *----------------------------------------------------------------------
 */

static int
FPissue (machPtr, type, resultType, source1, source2, dest, error)
     DLX *machPtr;              /* Machine of interest. */
     int type;                  /* Type of FP operation. */
     int resultType;            /* Type of result (see dlx.h) */
     int source1, source2;      /* FP source operands register numbers. */
     int dest;                  /* FP destination register number. */
     int *error;                /* set to non-zero if an error occurs,
				 * 0 otherwise */
{
  register FPop *opPtr;
  int i,j;
  int soonest;
  int wait;
  int num_units;
  int latency;
  int unitType;
  double *ds1, *ds2, *dd;
  float *fs1, *fs2, *fd;

  *error = 0;

  opPtr = mallocFPop();

  wait = 0;
  /* check WAW and RAW hazards */
  if (machPtr->waiting_FPRs[dest]) 
    wait = machPtr->waiting_FPRs[dest];
  if (machPtr->waiting_FPRs[source1] && 
      (!wait || machPtr->waiting_FPRs[source1] < wait))
    wait = machPtr->waiting_FPRs[source1];
  if (machPtr->waiting_FPRs[source2] && 
      (!wait || machPtr->waiting_FPRs[source2] < wait))
    wait = machPtr->waiting_FPRs[source2];

  unitType = type;
  switch (type) {
  case FP_SUB:
    unitType = FP_ADD;
  case FP_ADD:
    num_units = machPtr->num_add_units;
    latency = machPtr->fp_add_latency;
    break;
  case INT_DIV:
  case INT_UDIV:
    unitType = FP_DIV;
  case FP_DIV:
    num_units = machPtr->num_div_units;
    latency = machPtr->fp_div_latency;
    break;
  case INT_MUL:
  case INT_UMUL:
    unitType = FP_MUL;
  case FP_MUL:
    num_units = machPtr->num_mul_units;
    latency = machPtr->fp_mul_latency;
    break;
  }
    
  for (i = 0, soonest = MAXINT; i < num_units; i++) {
    if (!(j = machPtr->fp_units[unitType][i])) break;
    else if (j < soonest) soonest = j;
  }
  if (i == num_units && (soonest < wait || !wait)) wait = soonest;
  if (wait) return (wait - machPtr->cycleCount);

  opPtr->unit = i;
  opPtr->dest = dest;
  opPtr->ready = machPtr->cycleCount + latency;
  machPtr->fp_units[unitType][i] = opPtr->ready;
  machPtr->waiting_FPRs[dest] = opPtr->ready;

  switch (resultType) {
  case FP_SINGLE_FP_OP:
    fs1 = (float *)&(machPtr->regs[source1 + 32]);
    fs2 = (float *)&(machPtr->regs[source2 + 32]);
    fd = (float *)&(opPtr->result[0]);
    switch (type) {
    case FP_ADD: *fd = *fs1 + *fs2; break;
    case FP_DIV: *fd = *fs1 / *fs2; break;
    case FP_MUL: *fd = *fs1 * *fs2; break;
    case FP_SUB: *fd = *fs1 - *fs2; break;
    }
    break;
  case FP_DOUBLE_FP_OP:
    ds1 = (double *)&(machPtr->regs[source1 + 32]);
    ds2 = (double *)&(machPtr->regs[source2 + 32]);
    dd = (double *)&(opPtr->result[0]);
    switch (type) {
    case FP_ADD: *dd = *ds1 + *ds2; break;
    case FP_DIV: *dd = *ds1 / *ds2; break;
    case FP_MUL: *dd = *ds1 * *ds2; break;
    case FP_SUB: *dd = *ds1 - *ds2; break;
    }
    machPtr->waiting_FPRs[dest+1] = opPtr->ready;
    break;
  case FP_INT_OP:
    switch (type) {
    case INT_MUL:
      *error = Mult (machPtr->regs[source1 + 32],
		     machPtr->regs[source2 + 32], 1,
		     (int *)&(opPtr->result[0]));
      break;
    case INT_UMUL:
      *error = Mult (machPtr->regs[source1 + 32],
		     machPtr->regs[source2 + 32], 0,
		     (int *)&(opPtr->result[0]));
      break;
    case INT_DIV:
      if (machPtr->regs[source2 + 32] == 0) {
	/* divide by 0 */
	*((int *)&(opPtr->result[0])) = 0;
	*error = 1;
      } else
	*((int *)&(opPtr->result[0])) = machPtr->regs[source1 + 32] /
	  machPtr->regs[source2 + 32];
      break;
    case INT_UDIV:
      {
	unsigned int rs1, rs2, tmp;
	
	rs1 = machPtr->regs[source1 + 32];
	rs2 = machPtr->regs[source2 + 32];
	if (rs2 == 0) {
	  /* divide by 0 */
	  *((unsigned int *)&(opPtr->result[0])) = 0;
	  *error = 1;
	} else {
	  tmp = rs1 / rs2;
	  *((unsigned int *)&(opPtr->result[0])) = tmp;
	}
	break;
      }
    }
    break;
  }

  opPtr->type = type;
  opPtr->resultType = resultType;

  /* printf ("FP issue:\n");
  printf ("type %d unit %d resultType %d ready %d dest %d\n",
	  opPtr->type, opPtr->unit, opPtr->resultType, opPtr->ready,
	  opPtr->dest); */

  if (machPtr->FPopsList == NULL || 
      opPtr->ready < machPtr->FPopsList->ready) {
    opPtr->nextPtr = machPtr->FPopsList;
    machPtr->FPopsList = opPtr;
  } else {
    register FPop *opPtr2;
    for (opPtr2 = machPtr->FPopsList; opPtr2->nextPtr != NULL; 
	 opPtr2 = opPtr2->nextPtr)
      if (opPtr->ready < opPtr2->nextPtr->ready) break;
    opPtr->nextPtr = opPtr2->nextPtr;
    opPtr2->nextPtr = opPtr;
  }

  if (!machPtr->checkFP || (opPtr->ready < machPtr->checkFP))
    machPtr->checkFP = opPtr->ready;
  return 0;
}



/*
 *----------------------------------------------------------------------
 *
 * FPwriteBack --
 *
 *      Complete a pending floating point operation, and write result
 *      to destination register.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      Any FP operations which have completed at the current cycle count
 *      have their results written back to the appropriate registers.
 *      All the required housekeeping of the FP unit and pending operation
 *      data structures is performed.
 *
 *----------------------------------------------------------------------
 */

static void
FPwriteBack (machPtr)
     DLX *machPtr;              /* Machine of interest. */
{
  register FPop *opPtr;
  int i,j;

  while (machPtr->FPopsList != NULL && 
	 machPtr->FPopsList->ready <= machPtr->cycleCount) {
    opPtr = machPtr->FPopsList;
    machPtr->FPopsList = opPtr->nextPtr;
    if (machPtr->FPopsList != NULL) 
      machPtr->checkFP = machPtr->FPopsList->ready;
    else machPtr->checkFP = 0;

    switch (opPtr->type) {
    case FP_ADD:
    case FP_SUB: machPtr->fp_add_units[opPtr->unit] = 0; break;
    case INT_DIV:
    case INT_UDIV:
    case FP_DIV: machPtr->fp_div_units[opPtr->unit] = 0; break;
    case INT_MUL:
    case INT_UMUL:
    case FP_MUL: machPtr->fp_mul_units[opPtr->unit] = 0;
    }

    switch (opPtr->resultType) {
    case FP_INT_OP: {
      int *source, *dest;
      source = (int *)&(opPtr->result[0]);
      dest = (int *)&(machPtr->regs[32 + opPtr->dest]);
      *dest = *source;
      machPtr->waiting_FPRs[opPtr->dest] = 0;
      break;
    }
    case FP_SINGLE_FP_OP: {
      float *source, *dest;
      source = (float *)&(opPtr->result[0]);
      dest = (float *)&(machPtr->regs[32 + opPtr->dest]);
      *dest = *source;
      machPtr->waiting_FPRs[opPtr->dest] = 0;
      break;
    }
    case FP_DOUBLE_FP_OP: {
      double *source, *dest;
      source = (double *)&(opPtr->result[0]);
      dest = (double *)&(machPtr->regs[32 + opPtr->dest]);
      *dest = *source;
      machPtr->waiting_FPRs[opPtr->dest] = 0;
      machPtr->waiting_FPRs[opPtr->dest + 1] = 0;
      break;
    }
    } /* switch */
    freeFPop (opPtr);
  }
}

/*
 *----------------------------------------------------------------------
 *
 * FPwait --
 *
 *      Checks up to four floating point registers to see if they are
 *      destinations for pending operations.  This is used to check
 *      for both RAW and WAW hazards (e.g. in a MOVD instruction).
 *
 * Results:
 *	None.
 *
 * Side effects:
 *      The count of floating point stalls and clock cycles will be
 *      incremented an appropriate amount so that all of the specified
 *      registers receive their proper results from pending floating
 *      operations.
 *
 *----------------------------------------------------------------------
 */

static void
FPwait (machPtr, numChks, fpr1, fpr2, fpr3, fpr4)
     DLX *machPtr;                /* Machine of interest. */
     int numChks;                 /* Number of FP regs to check for hazards. */
     int fpr1, fpr2, fpr3, fpr4;  /* FP registers to wait on.  Only the first
				     n values are examined, where n is the
				     number passed in as numChks. */
{
  register FPop *opPtr;
  int latest, dest;

  opPtr = machPtr->FPopsList;
  latest = 0;
  while (opPtr != NULL) {
    dest = opPtr->dest;

    switch (opPtr->resultType) {
    case FP_DOUBLE_FP_OP:
      switch (numChks) {
      case 4: if ((dest == fpr4) || ((dest+1) == fpr4)) {
	latest = opPtr->ready; break;}
      case 3: if ((dest == fpr3) || ((dest+1) == fpr3)) {
	latest = opPtr->ready; break;}
      case 2: if ((dest == fpr2) || ((dest+1) == fpr2)) {
	latest = opPtr->ready; break;}
      case 1: if ((dest == fpr1) || ((dest+1) == fpr1))
	latest = opPtr->ready;
      }
      break;
    case FP_SINGLE_FP_OP:
    case FP_INT_OP:
      switch (numChks) {
      case 4: if (dest == fpr4) {latest = opPtr->ready; break;}
      case 3: if (dest == fpr3) {latest = opPtr->ready; break;}
      case 2: if (dest == fpr2) {latest = opPtr->ready; break;}
      case 1: if (dest == fpr1)	latest = opPtr->ready;
      }
      break;
    }
      
    opPtr = opPtr->nextPtr;
  }
  
  if (latest) {
    while (machPtr->cycleCount < latest) {
      machPtr->FPstalls += (machPtr->checkFP - machPtr->cycleCount);
      machPtr->cycleCount = machPtr->checkFP;
      FPwriteBack(machPtr);
    }
  }
}

static FPop *FPfree = NULL;

/*
 *----------------------------------------------------------------------
 *
 * mallocFPop --
 *
 *	Return a pointer to an unused FPop structure.
 *
 * Results:
 *	See above
 *
 * Side effects:
 *
 *----------------------------------------------------------------------
 */

static FPop *
mallocFPop()
{
    FPop *p;

    if (!FPfree)
	return (FPop *)malloc(sizeof(FPop));
    p = FPfree;
    FPfree = FPfree->nextPtr;
    return p;
}

/*
 *----------------------------------------------------------------------
 *
 * freeFPop --
 *
 *	Free what is pointed to
 *
 * Results:
 *	See above
 *
 * Side effects:
 *
 *----------------------------------------------------------------------
 */

static void
freeFPop(p)
FPop *p;
{
    p->nextPtr = FPfree;
    FPfree = p;
}

static char *errstring()
{
	extern int errno, sys_nerr;
	extern char *sys_errlist[];
	static char msgbuf[64];

	if( !errno )
		return "no error";
	if( errno >= sys_nerr ) {
		sprintf( msgbuf, "unknown error %d", errno );
		return msgbuf;
	}
	return sys_errlist[ errno ];
}

/*
 *----------------------------------------------------------------------
 *
 * Sim_TraceCmd --
 *
 *	This command turns on or off tracing in the simulator.
 *
 * Results:
 *	A standard Tcl result.
 *
 * Side effects:
 *	See the user documentation.
 *
 *----------------------------------------------------------------------
 */
int
Sim_TraceCmd(machPtr, interp, argc, argv)
    DLX *machPtr;			/* Machine description. */
    Tcl_Interp *interp;			/* Current interpreter. */
    int argc;				/* Number of arguments. */
    char **argv;			/* Argument strings. */
{
	if( argc == 2 && !strcmp( argv[1], "off" ) ) {
		if( !machPtr->refTraceFile ) {
			sprintf( interp->result, "tracing wasn't on" );
			return TCL_ERROR;
		}
		fclose( machPtr->refTraceFile );
		machPtr->refTraceFile = NULL;
		return TCL_OK;
	}
	if( argc == 3 && !strcmp( argv[1], "on" ) ) {
		if( machPtr->refTraceFile ) {
			sprintf( interp->result,
 			  "tracing already on -- turn it off first" );
			return TCL_ERROR;
		}
		if( !(machPtr->refTraceFile = fopen( argv[2], "a" )) ) {
			sprintf( interp->result, "couldn't open \"%s\": %s",
			  argv[2], errstring() );
			return TCL_ERROR;
		}
		return TCL_OK;
	}
	sprintf( interp->result, "bad option to trace: should be 'on' <filename> or 'off'." );
	return TCL_ERROR;
}
