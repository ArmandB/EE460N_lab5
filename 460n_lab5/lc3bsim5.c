/*
    Name: Armand Behroozi
    UTEID 1: ab56648
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N - Lab 5                                           */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         pagetable    page table in LC-3b machine language   */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {
    IRD,
    COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
/* MODIFY: you have to add all your new control signals */
	LD_PSR,
	LD_SSP,
	LD_USP,
	LD_UNAL,
	LD_PROTPGF,
	LD_EXCV,
	LD_RET,
	LD_PTBR,
	LD_VA,
	LD_MDR2,
	LD_TEMPIEX,
	CLEAR_UNAL,
	CLEAR_PROTPGF,
	GATE_MAR,
	GATE_PSR,
	GATE_SSP,
	GATE_USP,
	GATE_TEMPIEX,
	GATE_PTEADDR,
	GATE_PA,
	LDPSRMUX,
	LDMARMUX,
	RETSELMUX3, RETSELMUX2, RETSELMUX1, RETSELMUX0,
	IEXSELMUX,
	DRR6,
	SRR6,
	COND2,
	LD_CC2,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/


int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0] + (x[COND2] << 2)); } /* TODO: modified*/
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); }
int GetLSHF1(int *x)         { return(x[LSHF1]); }
/* MODIFY: you can add more Get functions for your new control signals */

int GetLD_PSR(int* state)		{ return state[LD_PSR]; }
int GetLD_SSP(int* state)       { return state[LD_SSP]; }
int GetLD_USP(int* state)       { return state[LD_USP]; }
int GetLD_UNAL(int* state)      { return state[LD_UNAL]; }
int GetLD_PROTPGF(int* state)      { return state[LD_PROTPGF]; }
int GetLD_EXCV(int* state)      { return state[LD_EXCV]; }
int GetLD_RET(int* state)		{ return state[LD_RET]; }
int GetLD_PTBR(int* state)		{ return state[LD_PTBR]; }
int GetLD_VA(int* state)		{ return state[LD_VA]; }
int GetLD_MDR2(int* state)		{ return state[LD_MDR2]; }
int GetLD_TEMPIEX(int* state)   { return state[LD_TEMPIEX]; }
int GetCLEAR_UNAL(int* state)	{ return state[CLEAR_UNAL]; }
int GetCLEAR_PROTPGF(int* state){ return state[CLEAR_PROTPGF]; }
int GetGATE_MAR(int* state)     { return state[GATE_MAR]; }
int GetGATE_PSR(int* state)     { return state[GATE_PSR]; }
int GetGATE_SSP(int* state)     { return state[GATE_SSP]; }
int GetGATE_USP(int* state)     { return state[GATE_USP]; }
int GetGATE_TEMPIEX(int* state) { return state[GATE_TEMPIEX]; }
int GetGATE_PTEADDR(int* state) { return state[GATE_PTEADDR]; }
int GetGATE_PA(int* state)		{ return state[GATE_PA]; }
int GetLDPSRMUX(int* state)     { return state[LDPSRMUX]; }
int GetLDMARMUX(int* state)     { return state[LDMARMUX]; }
int GetRETSELMUX(int* state)    { return state[RETSELMUX0] + (state[RETSELMUX1] << 1) + (state[RETSELMUX2] << 2) + (state[RETSELMUX3] << 3); }
int GetIEXSELMUX(int* state)    { return state[IEXSELMUX]; }
int GetDRR6(int* state)         { return state[DRR6]; }
int GetSRR6(int* state)         { return state[SRR6]; }
int GetCOND2(int* state)        { return state[COND2]; }
int GetLD_CC2(int* state)       { return state[LD_CC2]; }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A
   There are two write enable signals, one for each byte. WE0 is used for
   the least significant byte of a word. WE1 is used for the most significant
   byte of a word. */

#define WORDS_IN_MEM    0x2000 /* 32 frames */
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */

/* For lab 4 */
/* MODIFY: you should add here any other registers you need to implement interrupts and exceptions */
int INTV; /* Interrupt vector register */
int EXCV; /* Exception vector register */
int TEMPIEX; /* Temp register to store new pc created from adding LSHF1ed vector and 0x200*/
int SSP; /* Initial value of system stack pointer */
int USP; /* Register to save user stack pointer*/
int PSR; /* Program status register, has mode (user or supervisor), nzp, and priority (which we are not using right now)*/
int PROT; /* Is a 1 if there a protection exception is detected*/
int UNAL; /* Is a 1 if there is a unaligned access exception*/
int INT; /* Set to a 1 whenever there is an interrupt detected*/

/* For lab 5 */
int PTBR; /* This is initialized when we load the page table */
int VA;   /* Temporary VA register */
/* MODIFY: you should add here any other registers you need to implement virtual memory */
int RETURN;
int PGFAULT;


} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/* For lab 5 */
#define PAGE_NUM_BITS 9
#define PTE_PFN_MASK 0x3E00
#define PTE_VALID_MASK 0x0004
#define PAGE_OFFSET_MASK 0x1FF

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void timerInterrupt(void)
{
	if(CYCLE_COUNT == 299) /*cycle_count is number of cycles we have executed, so the 300th cycle would be after 299 cycles have executed*/
	{
		NEXT_LATCHES.INT = 1;
		NEXT_LATCHES.INTV = 2;
		printf("int set\n");
	}
}

void cycle() {

  timerInterrupt(); /*TODO added timer interrupt*/
  eval_micro_sequencer();
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
    int i;

    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
	if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
	}
	cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {
    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
	cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
    int address; /* this is a byte address */

    printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
    int k;

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%.4x\n", BUS);
    printf("MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");
    /*TODO: modified rdump for debugging*/
    printf("INT = %d, INTV = %d, EXCV = %d, TEMPIEX = 0x%.4x\n", CURRENT_LATCHES.INT, CURRENT_LATCHES.INTV, CURRENT_LATCHES.EXCV, CURRENT_LATCHES.TEMPIEX);
    printf("UNAL = %d, PROT = %d, PGF = %d\n", CURRENT_LATCHES.UNAL, CURRENT_LATCHES.PROT, CURRENT_LATCHES.PGFAULT);
    printf("SSP = 0x%.4x, USP = 0x%.4x\n", CURRENT_LATCHES.SSP, CURRENT_LATCHES.USP);
    printf("PSR = 0x%.4x\n", CURRENT_LATCHES.PSR);
    printf("RET = 0x%.4x\n", CURRENT_LATCHES.RETURN);
    printf("VA = 0x%.4x, PTBR = 0x%.4x\n", CURRENT_LATCHES.VA, CURRENT_LATCHES.PTBR);

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
    case 'G':
    case 'g':
	go();
	break;

    case 'M':
    case 'm':
	scanf("%i %i", &start, &stop);
	mdump(dumpsim_file, start, stop);
	break;

    case '?':
	help();
	break;
    case 'Q':
    case 'q':
	printf("Bye.\n");
	exit(0);

    case 'R':
    case 'r':
	if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
	else {
	    scanf("%d", &cycles);
	    run(cycles);
	}
	break;

    default:
	printf("Invalid Command\n");
	break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
	MEMORY[i][0] = 0;
	MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename, int is_virtual_base) {
    FILE * prog;
    int ii, word, program_base, pte, virtual_pc;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
	printf("Error: Can't open program file %s\n", program_filename);
	exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
	program_base = word >> 1;
    else {
	printf("Error: Program file is empty\n");
	exit(-1);
    }

    if (is_virtual_base) {
      if (CURRENT_LATCHES.PTBR == 0) {
	printf("Error: Page table base not loaded %s\n", program_filename);
	exit(-1);
      }

      /* convert virtual_base to physical_base */
      virtual_pc = program_base << 1;
      pte = (MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][1] << 8) |
	     MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][0];

      printf("virtual base of program: %04x\npte: %04x\n", program_base << 1, pte);
		if ((pte & PTE_VALID_MASK) == PTE_VALID_MASK) {
	      program_base = (pte & PTE_PFN_MASK) | ((program_base << 1) & PAGE_OFFSET_MASK);
   	   printf("physical base of program: %x\n\n", program_base);
	      program_base = program_base >> 1;
		} else {
   	   printf("attempting to load a program into an invalid (non-resident) page\n\n");
			exit(-1);
		}
    }
    else {
      /* is page table */
     CURRENT_LATCHES.PTBR = program_base << 1;
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
	/* Make sure it fits. */
	if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
		   program_filename, ii);
	    exit(-1);
	}

	/* Write the word to memory array. */
	MEMORY[program_base + ii][0] = word & 0x00FF;
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;;
	ii++;
    }

    if (CURRENT_LATCHES.PC == 0 && is_virtual_base)
      CURRENT_LATCHES.PC = virtual_pc;

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */
/*             and set up initial state of the machine         */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *pagetable_filename, char *program_filename, int num_prog_files) {
    int i;
    init_control_store(ucode_filename);

    init_memory();
    load_program(pagetable_filename,0);
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(program_filename,1);
	while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */

/* MODIFY: you can add more initialization code HERE */
    CURRENT_LATCHES.PSR = 0x8002; /* Initial value of the process status register*/
    CURRENT_LATCHES.INTV = 0;
    CURRENT_LATCHES.EXCV = 0;
    CURRENT_LATCHES.TEMPIEX = 0;
    CURRENT_LATCHES.USP = 0;
    CURRENT_LATCHES.PROT = 0;
    CURRENT_LATCHES.UNAL = 0;
    CURRENT_LATCHES.INT = 0;

    /*lab 5 modifications*/
    CURRENT_LATCHES.PGFAULT = 0;
    /*PTBR initialized in the shell code*/
    /*VA and RETURN will both be overwritten with useful data later so init value doesn't matter*/


    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 4) {
	printf("Error: usage: %s <micro_code_file> <page table file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argv[2], argv[3], argc - 3);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code, except for the places indicated
   with a "MODIFY:" comment.
   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/
void eval_micro_sequencer()
{
	int nextState;

	int* microI = CURRENT_LATCHES.MICROINSTRUCTION;
	int cond =  GetCOND(microI);
	int j = GetJ(microI);
	int ird = GetIRD(microI);
	int r = CURRENT_LATCHES.READY;
	int ben = CURRENT_LATCHES.BEN;
	int INT = CURRENT_LATCHES.INT;
	int prot = CURRENT_LATCHES.PROT;
	int unal = CURRENT_LATCHES.UNAL;
	int pgf = CURRENT_LATCHES.PGFAULT;
	int protOrUnalOrPgf = prot | unal | pgf;
	int ir11 = (CURRENT_LATCHES.IR >> 11) & 0x01;
	int ir1512 = (CURRENT_LATCHES.IR >> 12) & 0x0f;

	if(CURRENT_LATCHES.STATE_NUMBER == 0x27) /*ack interrupt if we are in state 39 which is the first state in interrupt handling*/
	{
		NEXT_LATCHES.INT = 0;
		printf("int cleared\n");
	}

	if(ird) /*cannot have ird and protOrunalOrpgf high at the same time, never happens*/
	{
		nextState = ir1512;
	}
	else if(protOrUnalOrPgf)
	{
		nextState = 34; /*34 is the common exception state*/
	}
	else
	{
		switch(cond)
		{
		case 0:
			nextState = j;
			break;
		case 1:
			nextState = j + (r << 1);
			break;
		case 2:
			nextState = j + (ben << 2);
			break;
		case 3:
			nextState = j + ir11;
			break;
		case 4:
			nextState = j + INT*6; /*because you already numbered the states and a transition from 33 -> 39 was the best you could do on a short notice*/
			break;
		default:
			break;
		}
	}

	if(cond == 7)/*if all 3 conds set; priority over everything, simulating a mux at the end of the microsequencer*/
	{
		nextState = CURRENT_LATCHES.RETURN & 0x7fff; /*mask the top bit b/c it tells you whether it's a read or a write*/
	}
  /*
   * Evaluate the address of the next state according to the
   * micro sequencer logic. Latch the next microinstruction.
   */
    memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[nextState], sizeof(int)*CONTROL_STORE_BITS);
    NEXT_LATCHES.STATE_NUMBER = nextState;
}


void cycle_memory()
{
	int* microI = CURRENT_LATCHES.MICROINSTRUCTION;
	static int memCount = 1;
	int rw = GetR_W(microI);
	int mioEn = GetMIO_EN(microI);
	int dataSize = GetDATA_SIZE(microI);
	int mar0 = CURRENT_LATCHES.MAR & 0x01;


	if(mioEn)
	{
		if(memCount == 4)
		{
			NEXT_LATCHES.READY = 1;
		}
		if(memCount == 5)
		{
			if(rw)
			{
				if(dataSize == 0)
				{
					if(mar0 == 0)
					{
						MEMORY[CURRENT_LATCHES.MAR >> 1][0] = CURRENT_LATCHES.MDR & 0xff;
					}
					else
					{
						MEMORY[CURRENT_LATCHES.MAR >> 1][1] = (CURRENT_LATCHES.MDR >> 8) & 0xff;
					}
				}
				else
				{
					/*no unaligned accesses*/
					MEMORY[CURRENT_LATCHES.MAR >> 1][0] = CURRENT_LATCHES.MDR & 0xff;
					MEMORY[CURRENT_LATCHES.MAR >> 1][1] = (CURRENT_LATCHES.MDR >> 8) & 0xff;
				}

			}
			NEXT_LATCHES.READY = 0;
			memCount = 0; /*b/c gets incremented as soon as you reset it*/
		}
		memCount += 1;
	}
  /*
   * This function emulates memory and the WE logic.
   * Keep track of which cycle of MEMEN we are dealing with.
   * If fourth, we need to latch Ready bit at the end of
   * cycle to prepare microsequencer for the fifth cycle.
   */

}

int16_t outGateMarmux;
int16_t outGatePc;
int16_t outGateAlu;
int16_t outGateMdr;
int16_t outGateShf;
/*for lab 4*/
int16_t outGateTempiex;
int16_t outGatePsr;
int16_t outGateSsp;
int16_t outGateUsp;
int16_t outGateMar;
/*for lab 5*/
int16_t outGatePa;
int16_t outGatePteAddr;

void eval_bus_drivers()
{
	int* microI = CURRENT_LATCHES.MICROINSTRUCTION;
	int marmux = GetMARMUX(microI);
	int addr2mux = GetADDR2MUX(microI);
	int lshf1 = GetLSHF1(microI);
	int addr1mux = GetADDR1MUX(microI);
	int sr1mux = GetSR1MUX(microI);
	int ir = CURRENT_LATCHES.IR;
	int16_t addend1;
	int16_t addend2;

	/*extra vars needed for calculating gate alu*/
	int aluk = GetALUK(microI);
	int16_t op2;
	int16_t op1;

	/*extra vars for calculating gate shf*/
	int typeShf;
	int amount4;

	/*extra vars for calculating gate mdr*/
	int mar0 = CURRENT_LATCHES.MAR & 0x01;
	int dataSize = GetDATA_SIZE(microI);

	/*lab 4*/
	int srr6 = GetSRR6(microI);

	/*lab 5*/
	int vaOffset = CURRENT_LATCHES.VA & 0x1ff;
	int pfn = (CURRENT_LATCHES.MDR >> 9) & 0x1f;

	int pageNum = (CURRENT_LATCHES.VA >> 9) & 0x7f;



	/*calculate value for GATEmarmux*/

	if(marmux == 0)
	{
		outGateMarmux = (CURRENT_LATCHES.IR & 0xff) << 1; /*zext 7:0 and lshf1*/
	}
	else
	{
		switch(addr2mux)
		{
		case 0:
			addend2 = 0;
			break;
		case 1:
			addend2 = CURRENT_LATCHES.IR & 0x3f;
			if(addend2 & 0x20)
			{
				addend2 |= ~0x3f;
			}
			break;
		case 2:
			addend2 = CURRENT_LATCHES.IR & 0x1ff;
			if(addend2 & 0x100)
			{
				addend2 |= ~0x1ff;
			}
			break;
		case 3:
			addend2 = CURRENT_LATCHES.IR & 0x7ff;
			if(addend2 & 0x400)
			{
				addend2 |= ~0x7ff;
			}
			break;
		}
		if(lshf1 != 0)
		{
			addend2 = addend2 << 1;
		}
		/*now addend2 has been chosen, SEXTed, and lshfed if necessary*/

		if(addr1mux == 0)
		{
			addend1 = NEXT_LATCHES.PC;
		}
		else
		{
			if((sr1mux == 0) && (srr6 == 0))
			{
				addend1 = CURRENT_LATCHES.REGS[(ir >> 9) & 0x07]; /*11:9*/
			}
			else if((sr1mux == 1) && (srr6 == 0))
			{
				addend1 = CURRENT_LATCHES.REGS[(ir >> 6) & 0x07]; /*8:6*/
			}
			else if((sr1mux == 0) && (srr6 == 1))
			{
				addend1 = CURRENT_LATCHES.REGS[6]; /*r6*/
			}
		}
		/*addend1 has been chosen*/

		outGateMarmux = addend1 + addend2;
	}

	/*calculate value for GATEpc*/

	outGatePc = CURRENT_LATCHES.PC;

	/*calculate value for GATEalu*/

	if((sr1mux == 0) && (srr6 == 0))
	{
		op1 = (int16_t) CURRENT_LATCHES.REGS[(ir >> 9) & 0x07]; /*11:9*/
	}
	else if((sr1mux == 1) && (srr6 == 0))
	{
		op1 = (int16_t) CURRENT_LATCHES.REGS[(ir >> 6) & 0x07]; /*8:6*/
	}
	else if((sr1mux == 0) && (srr6 == 1))
	{
		op1 = (int16_t) CURRENT_LATCHES.REGS[6]; /*R6*/
	}

	if(ir & 0x20) /*use bit 5 as steering*/
	{
		/*sexted imm5*/
		op2 = ir & 0x1f;
		if(op2 & 0x10)
		{
			op2 |= ~0x1f;
		}
	}
	else
	{
		op2 = CURRENT_LATCHES.REGS[ir & 0x07];
	}

	switch(aluk)
	{
	case 0: /*add*/
		outGateAlu = op1 + op2;
		break;
	case 1: /*and*/
		outGateAlu = op1 & op2;
		break;
	case 2: /*xor*/
		outGateAlu = op1 ^ op2;
		break;
	case 3: /*passA*/
		outGateAlu = op1;
		break;
	}

	/*calculate value for GATEshf*/

	op1 = CURRENT_LATCHES.REGS[(ir >> 6) & 0x07];
	typeShf = (ir >> 4) & 0x03;
	amount4 = ir & 0x0f;

	switch(typeShf)
	{
	case 0:
		outGateShf = op1 << amount4;
		break;
	case 1:
		outGateShf = op1 >> amount4;
		outGateShf &= (0xffff >> amount4); /*b/c machine does 32 bit arithmetic so when it shifts the leading 1s follow for negative #s*/
		break;
	case 3:
		if(op1 & 0x8000)
		{
			outGateShf = op1 >> amount4;
			outGateShf |= (0xffff << (16 - amount4));
		}
		else
		{
			outGateShf = op1 >> amount4;
			outGateShf &= (0xffff >> amount4); /*shouldn't be necessary b/c not negative but why not*/
		}
		break;
	}

	/*calculate value for GATEmdr*/
	if((mar0 == 0) && dataSize == 0)
	{
		outGateMdr = CURRENT_LATCHES.MDR & 0xff;
		if(outGateMdr & 0x80)
		{
			outGateMdr |= 0xff00;
		}
	}

	if((mar0 == 1) && dataSize == 0)
	{
		outGateMdr = CURRENT_LATCHES.MDR >> 8;
		if(outGateMdr & 0x80)
		{
			outGateMdr |= 0xff00;
		}
	}

	if((mar0 == 0) && dataSize == 1)
	{
		outGateMdr = CURRENT_LATCHES.MDR;
	}
	/*cannot have both be 1s b/c no unaligned accesses*/

	/*calculate value for GATEtempiex*/
	outGateTempiex = CURRENT_LATCHES.TEMPIEX;

	/*calculate value for GATEpsr*/
	outGatePsr = CURRENT_LATCHES.PSR;

	/*calculate value for GATEssp*/
	outGateSsp = CURRENT_LATCHES.SSP - 4;

	/*calculate value for GATEusp*/
	outGateUsp = CURRENT_LATCHES.USP;

	/*calculate vaue for GATEmar*/
	outGateMar = CURRENT_LATCHES.MAR;

	/*calculate value for GatePa*/
	outGatePa = vaOffset + (pfn << 9);

	/*calculate value for GatePteAddr*/
	outGatePteAddr = pageNum*2 + CURRENT_LATCHES.PTBR;


  /*
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers
   *             Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR.
   *
   *		 plus other gates for lab 4, look at about vars and youll know which
   */

}

void drive_bus()
{
	int* microI = CURRENT_LATCHES.MICROINSTRUCTION;
	int gateMarmux = GetGATE_MARMUX(microI);
	int gatePc = GetGATE_PC(microI);
	int gateAlu = GetGATE_ALU(microI);
	int gateMdr = GetGATE_MDR(microI);
	int gateShf = GetGATE_SHF(microI);

	int gateTempiex = GetGATE_TEMPIEX(microI);
	int gatePsr = GetGATE_PSR(microI);
	int gateSsp = GetGATE_SSP(microI);
	int gateUsp = GetGATE_USP(microI);
	int gateMar = GetGATE_MAR(microI);

	/*lab 5*/
	int gatePa = GetGATE_PA(microI);
	int gatePteAddr = GetGATE_PTEADDR(microI);

	int countBusDrivers = 0; /*for debugging*/


	if(gateMarmux)
	{
		BUS = outGateMarmux;
		countBusDrivers += 1;
	}
	if(gatePc)
	{
		BUS = outGatePc;
		countBusDrivers += 1;
	}
	if(gateAlu)
	{
		BUS = outGateAlu;
		countBusDrivers += 1;
	}
	if(gateMdr)
	{
		BUS = outGateMdr;
		countBusDrivers += 1;
	}
	if(gateShf)
	{
		BUS = outGateShf;
		countBusDrivers += 1;
	}
	/*lab4*/
	if(gateTempiex)
	{
		BUS = outGateTempiex;
		countBusDrivers += 1;
	}
	if(gatePsr)
	{
		BUS = outGatePsr;
		countBusDrivers += 1;
	}
	if(gateSsp)
	{
		BUS = outGateSsp;
		countBusDrivers += 1;
	}
	if(gateUsp)
	{
		BUS = outGateUsp;
		countBusDrivers += 1;
	}
	if(gateMar)
	{
		BUS = outGateMar;
		countBusDrivers += 1;
	}
	/*lab 5*/
	if(gatePteAddr)
	{
		BUS = outGatePteAddr;
		countBusDrivers += 1;
	}
	if(gatePa)
	{
		BUS = outGatePa;
		countBusDrivers += 1;
	}

	if(countBusDrivers == 0)
	{
		BUS = 0;
	}
	if(countBusDrivers > 1)
	{
		printf("****MORE THAN TWO GATES DRIVING THE BUS****\n");
	}

	BUS &= 0xffff;
  /*
   * Datapath routine for driving the bus from one of the 5 possible
   * tristate drivers.
   */

}


void latch_datapath_values()
{
	int* microI = CURRENT_LATCHES.MICROINSTRUCTION;
	int ldMar = GetLD_MAR(microI);
	int ldMdr = GetLD_MDR(microI);
	int dataSize = GetDATA_SIZE(microI);
	int mioEn = GetMIO_EN(microI);
	int ldIr = GetLD_IR(microI);;
	int ldBen = GetLD_BEN(microI);
	int n = (CURRENT_LATCHES.IR >> 11) & 0x01;
	int z = (CURRENT_LATCHES.IR >> 10) & 0x01;
	int p = (CURRENT_LATCHES.IR >> 9) & 0x01;
	int ldReg = GetLD_REG(microI);
	int drMux = GetDRMUX(microI);
	int ldCc = GetLD_CC(microI);
	int ldPc = GetLD_PC(microI);
	int pcMux = GetPCMUX(microI);
	int addr1mux = GetADDR1MUX(microI);
	int addr2mux = GetADDR2MUX(microI);
	int sr1mux = GetSR1MUX(microI);
	int lshf1 = GetLSHF1(microI);
	int ir = CURRENT_LATCHES.IR;

	int16_t addend1;
	int16_t addend2;

	/*lab 4*/
	int ldMarMux = GetLDMARMUX(microI);
	int drr6 = GetDRR6(microI);

	int ldPsr = GetLD_PSR(microI);
	int ldSsp = GetLD_SSP(microI);
	int ldUsp = GetLD_USP(microI);
	int ldUnal = GetLD_UNAL(microI);
	int ldExcv = GetLD_EXCV(microI);
	int ldTempiex = GetLD_TEMPIEX(microI);

	int ldPsrMux = GetLDPSRMUX(microI);
	int bus1512 = (BUS >> 12) & 0xf;
	int psr15 = (CURRENT_LATCHES.PSR >> 15) & 0x01;
	int prot = CURRENT_LATCHES.PROT;
	int unal = CURRENT_LATCHES.UNAL;
	int iexSel = GetIEXSELMUX(microI);
	int ldCc2 = GetLD_CC2(microI);

	/*lab 5*/
	int ldMdr2 = GetLD_MDR2(microI);
	int ret15 = (CURRENT_LATCHES.RETURN >> 15) & 0x1;
	int clearUnal = GetCLEAR_UNAL(microI);
	int ldProtPgf = GetLD_PROTPGF(microI);
	int clearProtPgf = GetCLEAR_PROTPGF(microI);
	int pgf = CURRENT_LATCHES.PGFAULT;
	int excvSel = unal*4 + prot*2 + pgf; /*unal'prot'pgf*/
	int ldRet = GetLD_RET(microI);
	int retSel = GetRETSELMUX(microI);
	int ldPtbr = GetLD_PTBR(microI);
	int ldVa = GetLD_VA(microI);


	/*ld mdr- both ld mdr signals cannot be high at once*/
	if(ldMdr)
	{
			if((mioEn != 0) && (CURRENT_LATCHES.READY != 0)) /*try w/o dependence on ready bit. technically would be loading stuff from last access but we can't simulate that*/
			{
				/*loading, regardless of the size you still load both bytes and choose which one you want later*/
				NEXT_LATCHES.MDR = MEMORY[CURRENT_LATCHES.MAR >> 1][1] << 8;
				NEXT_LATCHES.MDR |= MEMORY[CURRENT_LATCHES.MAR >> 1][0] & 0xff;
			}
			if(mioEn == 0) /*Doesn't depend on ready, even on ready being 0*/
			{
				/*storing*/
				if(dataSize == 0)
				{
					NEXT_LATCHES.MDR = (BUS & 0xff) << 8;
					NEXT_LATCHES.MDR |= BUS & 0xff; /*WE signal will decide whether high or low byte gets stored, but 2 copies of data in mdr*/
				}
				else
				{
					NEXT_LATCHES.MDR = BUS;
				}
			}
	}
	else if(ldMdr2)
	{
		NEXT_LATCHES.MDR |= 0x1;
		NEXT_LATCHES.MDR |= (ret15 << 1);
	}

	/*ld ir*/
	if(ldIr)
	{
		NEXT_LATCHES.IR = BUS;
	}

	/*ld ben*/
	if(ldBen)
	{
		NEXT_LATCHES.BEN = (CURRENT_LATCHES.N & n) || (CURRENT_LATCHES.Z & z) || (CURRENT_LATCHES.P & p);
	}

	/*ld cc*/
	if(ldCc)
	{
		NEXT_LATCHES.Z = 0;
		NEXT_LATCHES.N = 0;
		NEXT_LATCHES.P = 0;
		if(BUS == 0)
		{
			NEXT_LATCHES.Z = 1;
		}
		else if(BUS & 0x8000)
		{
			NEXT_LATCHES.N = 1;
		}
		else
		{
			NEXT_LATCHES.P = 1;
		}
	}
	if(ldCc2)
	{
		NEXT_LATCHES.N = (NEXT_LATCHES.PSR >> 2) & 0x1;
		NEXT_LATCHES.Z = (NEXT_LATCHES.PSR >> 1) & 0x1;
		NEXT_LATCHES.P = (NEXT_LATCHES.PSR) & 0x1;
	}


/*modified for lab 4*/

	/*ld pc*/
	if(ldPc)
	{
		switch(pcMux)
		{
		case 0:
			NEXT_LATCHES.PC += 2;
			break;
		case 1:
			NEXT_LATCHES.PC = BUS;
			break;
		case 2:
			switch(addr2mux)
			{
			case 0:
				addend2 = 0;
				break;
			case 1:
				addend2 = CURRENT_LATCHES.IR & 0x3f;
				if(addend2 & 0x20)
				{
					addend2 |= ~0x3f;
				}
				break;
			case 2:
				addend2 = CURRENT_LATCHES.IR & 0x1ff;
				if(addend2 & 0x100)
				{
					addend2 |= ~0x1ff;
				}
				break;
			case 3:
				addend2 = CURRENT_LATCHES.IR & 0x7ff;
				if(addend2 & 0x400)
				{
					addend2 |= ~0x7ff;
				}
				break;
			}
			if(lshf1 != 0)
			{
				addend2 = addend2 << 1;
			}
			/*now addend2 has been chosen, SEXTed, and lshfed if necessary*/

			if(addr1mux == 0)
			{
				addend1 = NEXT_LATCHES.PC;
			}
			else
			{
				if(sr1mux == 0)
				{
					addend1 = CURRENT_LATCHES.REGS[(ir >> 9) & 0x07]; /*11:9*/
				}
				else
				{
					addend1 = CURRENT_LATCHES.REGS[(ir >> 6) & 0x07]; /*8:6*/
				}
			}
			/*addend1 has been chosen*/

			NEXT_LATCHES.PC = addend1 + addend2;
			break;
		case 3:
			NEXT_LATCHES.PC -= 2;
			break;
		}
	}

	/*ld mar*/
	if(ldMar)
	{
		if(ldMarMux == 0)
		{
			NEXT_LATCHES.MAR = BUS;
		}
		else
		{
			NEXT_LATCHES.MAR += 2;
		}

	}

	/*ld reg*/
	if(ldReg)
	{
		if((drMux == 1) && (drr6 == 0))
		{
			NEXT_LATCHES.REGS[7] = BUS; /*drMux = 1 means r7 is dr reg*/
		}
		else if((drMux == 0) && (drr6 == 0))
		{
			NEXT_LATCHES.REGS[(CURRENT_LATCHES.IR >> 9) & 0x07] = BUS;
		}
		else if((drMux == 0) && (drr6 == 1))
		{
			NEXT_LATCHES.REGS[6] = BUS; /*R6 is dr*/
		}
	}

	/*ld psr*/
	if(ldPsr)
	{
		if(ldPsrMux == 0)
		{
			NEXT_LATCHES.PSR = BUS;
		}
		else
		{
			NEXT_LATCHES.PSR &= 0x7fff; /*make top bit a 0 to put into supervisor mode*/
		}
	}
	if(ldCc)
	{
		NEXT_LATCHES.PSR &= 0xfff8;
		NEXT_LATCHES.PSR |= (NEXT_LATCHES.N << 2);
		NEXT_LATCHES.PSR |= (NEXT_LATCHES.Z << 1);
		NEXT_LATCHES.PSR |= NEXT_LATCHES.P;
	}

	/*ld ssp*/
	if(ldSsp)
	{
		NEXT_LATCHES.SSP = BUS;
	}

	/*ld usp*/
	if(ldUsp)
	{
		NEXT_LATCHES.USP = BUS;
	}

	/*ld unal modified for lab 5- Clear and Ld are never 1 at the same time*/
	if(ldUnal)
	{
		if((BUS & 0x01) == 1) /*if BUS[0] is odd then it means we're going to do an unaligned access*/
		{
			NEXT_LATCHES.UNAL = 1;
		}
		else
		{
			NEXT_LATCHES.UNAL = 0;
		}
	}
	else if(clearUnal)
	{
		NEXT_LATCHES.UNAL = 0;
	}

	/*ld prot and pgf modified for lab 5*/
	if(ldProtPgf)
	{
		if((CURRENT_LATCHES.MDR & 0x4) == 0)
		{
			NEXT_LATCHES.PGFAULT = 1;
		}
		else
		{
			NEXT_LATCHES.PGFAULT = 0;
		}

		if( ((CURRENT_LATCHES.MDR & 0x8) == 0) && ((CURRENT_LATCHES.PSR & 0x8000) == 0x8000) ) /*if page protected and in user mode*/
		{
			if((CURRENT_LATCHES.RETURN != 28) && (CURRENT_LATCHES.RETURN != 0x8031)) /*if isn't trap or exception*/
			{
				NEXT_LATCHES.PROT = 1;
			}
			else
			{
				NEXT_LATCHES.PROT = 0;
			}

		}
		else
		{
			NEXT_LATCHES.PROT = 0;
		}
	}
	else if(clearProtPgf)
	{
		NEXT_LATCHES.PROT = 0;
		NEXT_LATCHES.PGFAULT = 0;
	}

	/*ld excv - modified for lab 5*/
	if(ldExcv)
	{
		switch(excvSel)
		{
		case 0:
			NEXT_LATCHES.EXCV = 0xa;
			break;
		case 1:
			NEXT_LATCHES.EXCV = 0x4;
			break;
		case 2:
		case 3:
			NEXT_LATCHES.EXCV = 0x8;
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			NEXT_LATCHES.EXCV = 0x6;
			break;
		}
	}

	/*ld tempiex*/
	if(ldTempiex)
	{
		if(iexSel == 0)
		{
			NEXT_LATCHES.TEMPIEX = 0x200 + CURRENT_LATCHES.INTV;
		}
		else
		{
		NEXT_LATCHES.TEMPIEX = 0x200 + CURRENT_LATCHES.EXCV;
		}
	}

	/*ld ret*/
	if(ldRet)
	{
		switch(retSel)
		{
		case 0:
			NEXT_LATCHES.RETURN = 25;
			break;
		case 1:
			NEXT_LATCHES.RETURN = 28;
			break;
		case 2:
			NEXT_LATCHES.RETURN = 29;
			break;
		case 3:
			NEXT_LATCHES.RETURN = 44;
			break;
		case 4:
			NEXT_LATCHES.RETURN = 57;
			break;
		case 5:
			NEXT_LATCHES.RETURN = 63;
			break;
		case 9:
			NEXT_LATCHES.RETURN = 0x8017;
			break;
		case 10:
			NEXT_LATCHES.RETURN = 0x8018;
			break;
		case 11:
			NEXT_LATCHES.RETURN = 0x8031;
			break;
		default:
			break;
		}
	}

	/*ld ptbr*/
	if(ldPtbr)
	{
		NEXT_LATCHES.PTBR = BUS;
	}

	/*ld va*/
	if(ldVa)
	{
		NEXT_LATCHES.VA = BUS;
	}

  /*
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come
   * after drive_bus.
   */

	NEXT_LATCHES.BEN &= 	0xffff;
	NEXT_LATCHES.IR &= 		0xffff;
	NEXT_LATCHES.MDR &= 	0xffff;
	NEXT_LATCHES.N &= 		0xffff;
	NEXT_LATCHES.Z &= 		0xffff;
	NEXT_LATCHES.P &= 		0xffff;
	NEXT_LATCHES.PC &= 		0xffff;
	NEXT_LATCHES.READY &= 	0xffff;

	NEXT_LATCHES.INTV &= 	0xffff;
	NEXT_LATCHES.EXCV &= 	0xffff;
	NEXT_LATCHES.TEMPIEX &= 0xffff;
	NEXT_LATCHES.SSP &= 	0xffff;
	NEXT_LATCHES.USP &= 	0xffff;
	NEXT_LATCHES.PSR &= 	0xffff;
	NEXT_LATCHES.PROT &= 	0xffff;
	NEXT_LATCHES.UNAL &= 	0xffff;
	NEXT_LATCHES.INT &= 	0xffff;

	NEXT_LATCHES.PGFAULT &= 0xffff;
	NEXT_LATCHES.RETURN &= 	0xffff;
	NEXT_LATCHES.VA &= 		0xffff;
	NEXT_LATCHES.PTBR &= 	0xffff;


}

