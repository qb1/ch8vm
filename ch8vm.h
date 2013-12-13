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
#define CH8_FPS	1000
#define CH8_PAUSED_FPS 2
#define CH8_TIMER_HZ 60
#define CH8_SCREEN_HZ 30

typedef struct
{
	/* Memory */
	uint8_t M[CH8_MEMORY_SIZE];			// Must stay FIRST!
	uint8_t Screen[CH8_SCREEN_SIZE];

	uint16_t I;
	uint8_t  V[0x10];

	uint8_t SoundTimer;
	uint8_t DelayTimer;	

	/* Control Flow */
	uint16_t PC;
	uint16_t CallStack[16];
	uint16_t* StackPointer;
	int8_t	 PausedOnKey;

	int8_t	Paused;

	/* Keys */
	uint8_t Key[0x10];

} CH8_STATE;

/* Pointers to the actual data used in the instruction set functions */
extern CH8_STATE* ch8_State;
extern CH8_INSTR* ch8_Instr;

#define OPCODE_ARGS  uint16_t param1, uint16_t param2, uint16_t param3

typedef void (*CH8_CALL)( OPCODE_ARGS ) ;

void ch8_InitVM( uint8_t *memory, uint16_t mem_size );
void ch8_StartVM();
void ch8_execInstr();
void ch8_VMStep( int key );	// Must be called at CH8_FPS
void ch8_VMTimerUpdate();	// Must be called at CH8_TIMER_HZ
void ch8_printState( int x, int y, int w, int h );

/* OS specific */
void ch8_OS_Init();
void ch8_OS_Start();
void ch8_OS_Pause();
void ch8_OS_Resume();
void ch8_OS_tick();
void ch8_OS_UpdateScreen();

/* CH8 Instruction set */
void ch8_SCDOWN ( OPCODE_ARGS );
void ch8_CLS ( OPCODE_ARGS );
void ch8_RTS ( OPCODE_ARGS );
void ch8_SCRIGHT ( OPCODE_ARGS );
void ch8_SCLEFT ( OPCODE_ARGS );
void ch8_EXIT ( OPCODE_ARGS );
void ch8_LOW ( OPCODE_ARGS );
void ch8_HIGH ( OPCODE_ARGS );

void ch8_JMP ( OPCODE_ARGS );
void ch8_CALL ( OPCODE_ARGS );
void ch8_SKEQ_K ( OPCODE_ARGS );
void ch8_SKNE_K ( OPCODE_ARGS );
void ch8_SKEQ ( OPCODE_ARGS );

void ch8_MOV_K ( OPCODE_ARGS );
void ch8_ADD_K ( OPCODE_ARGS );

void ch8_MOV ( OPCODE_ARGS );
void ch8_OR ( OPCODE_ARGS );
void ch8_AND ( OPCODE_ARGS );
void ch8_XOR ( OPCODE_ARGS );
void ch8_ADD ( OPCODE_ARGS );
void ch8_SUB ( OPCODE_ARGS );
void ch8_SHR ( OPCODE_ARGS );
void ch8_RSB ( OPCODE_ARGS );
void ch8_SHL ( OPCODE_ARGS );

void ch8_SKNE ( OPCODE_ARGS );

void ch8_MVI ( OPCODE_ARGS );
void ch8_JMI ( OPCODE_ARGS );
void ch8_RAND ( OPCODE_ARGS );

void ch8_SPRITE ( OPCODE_ARGS );

void ch8_SKPR ( OPCODE_ARGS );
void ch8_SKUP ( OPCODE_ARGS );

void ch8_GDELAY ( OPCODE_ARGS );
void ch8_KEY ( OPCODE_ARGS );
void ch8_SDELAY ( OPCODE_ARGS );
void ch8_SSOUND ( OPCODE_ARGS );
void ch8_ADI ( OPCODE_ARGS );
void ch8_FONT ( OPCODE_ARGS );
void ch8_XFONT ( OPCODE_ARGS );
void ch8_BCD ( OPCODE_ARGS );
void ch8_STR ( OPCODE_ARGS );
void ch8_LDR ( OPCODE_ARGS );
void ch8_STR_RPL ( OPCODE_ARGS );
void ch8_LDR_RPL ( OPCODE_ARGS );

extern CH8_CALL CallTable[CH8_INSTR_COUNT+1];

#endif // CH8VM_H