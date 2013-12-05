#ifndef CH8VM_H
#define CH8VM_H

#include <stdio.h>
#include <stdint.h>

#include "parser.h"


/* VM Current state */ 
#define CH8_MEMORY_SIZE 0x10000
//#define CH8_MEMORY_START 0x200

#define CH8_SCREEN_WIDTH 64
#define CH8_SCREEN_HEIGHT 32
#define CH8_SCREEN_SIZE CH8_SCREEN_WIDTH*CH8_SCREEN_HEIGHT/8


typedef struct
{
	/* Memory */
	uint16_t I;
	uint8_t  V[0x10];

	uint8_t SoundTimer;
	uint8_t DelayTimer;

	uint8_t M[CH8_MEMORY_SIZE];
	uint8_t Screen[CH8_SCREEN_SIZE];

	/* Control Flow */
	uint16_t PC;
	uint16_t CallStack[16];
	uint16_t* StackPointer;
	int8_t	 PausedOnKey;

	/* Keys */
	uint8_t Key[0x10];

} CH8_STATE;

/* Pointers to the actual data used in the instruction set functions */
extern CH8_STATE* ch8_State;
extern CH8_INSTR* ch8_Instr;

typedef void (*CH8_CALL)() ;

void ch8_InitVM( CH8_STATE* state, CH8_INSTR* instr );
void ch8_execInstr();
void ch8_StartVM();
void ch8_printState( int x, int y, int w, int h );

/* OS specific */
void ch8_OS_Init();
int ch8_OS_ReadKeys();					// Returns a pressed key, -1 otherwise
void ch8_OS_PrintScreen();
int ch8_OS_tick( uint32_t *tick );		// Returns 1 if must quit

/* CH8 Instruction set */
void ch8_SCDOWN ();
void ch8_CLS ();
void ch8_RTS ();
void ch8_SCRIGHT ();
void ch8_SCLEFT ();
void ch8_EXIT ();
void ch8_LOW ();
void ch8_HIGH ();

void ch8_JMP ();
void ch8_CALL ();
void ch8_SKEQ_K ();
void ch8_SKNE_K ();
void ch8_SKEQ ();

void ch8_MOV_K ();
void ch8_ADD_K ();

void ch8_MOV ();
void ch8_OR ();
void ch8_AND ();
void ch8_XOR ();
void ch8_ADD ();
void ch8_SUB ();
void ch8_SHR ();
void ch8_RSB ();
void ch8_SHL ();

void ch8_SKNE ();

void ch8_MVI ();
void ch8_JMI ();
void ch8_RAND ();

void ch8_SPRITE ();

void ch8_SKPR ();
void ch8_SKUP ();

void ch8_GDELAY ();
void ch8_KEY ();
void ch8_SDELAY ();
void ch8_SSOUND ();
void ch8_ADI ();
void ch8_FONT ();
void ch8_XFONT ();
void ch8_BCD ();
void ch8_STR ();
void ch8_LDR ();
void ch8_STR_RPL ();
void ch8_LDR_RPL ();

extern CH8_CALL CallTable[CH8_INSTR_COUNT+1];

#endif // CH8VM_H