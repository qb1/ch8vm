#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>
#include <stdarg.h>

#include "ch8vm.h"

/* TODO 
	* sound
	* S-CHIP
*/

/* Bunch 'o help macro */
//#define _M (ch8_State->M-0x200)
#define _M ch8_State->M
#define _I ch8_State->I
#define _V ch8_State->V
#define _Scr ch8_State->Screen
#define _PC ch8_State->PC
#define _SP ch8_State->StackPointer
#define _KEY ch8_State->Key

#define _DTIM ch8_State->DelayTimer
#define _STIM ch8_State->SoundTimer

#define _X ch8_Instr->param1
#define _Y ch8_Instr->param2
#define _Z ch8_Instr->param3

unsigned char _ch8Font[] = {
		0xF0,0x90,0x90,0x90,0xF0,
		0x20,0x60,0x20,0x20,0x70,
		0xF0,0x10,0xF0,0x80,0xF0,
		0xF0,0x10,0xF0,0x10,0xF0,
		0x90,0x90,0xF0,0x10,0x10,
		0xF0,0x80,0xF0,0x10,0xF0,
		0xF0,0x80,0xF0,0x90,0xF0,
		0xF0,0x10,0x20,0x40,0x40,
		0xF0,0x90,0xF0,0x90,0xF0,
		0xF0,0x90,0xF0,0x10,0xF0,
		0xF0,0x90,0xF0,0x90,0x90,
		0xE0,0x90,0xE0,0x90,0xE0,
		0xF0,0x80,0x80,0x80,0xF0,
		0xE0,0x90,0x90,0x90,0xE0,
		0xF0,0x80,0xF0,0x80,0xF0,
		0xF0,0x80,0xF0,0x80,0x80,
};

CH8_STATE* ch8_State;
CH8_INSTR* ch8_Instr;

static CH8_STATE global_state = { .I=1 };
static CH8_INSTR global_instr;

int main()
{
	ch8_InitVM();
	ch8_StartVM();

	return 0;
}

void ch8_InitVM()
{
	ch8_State = &global_state;
	ch8_Instr = &global_instr;

	memset( ch8_Instr, 0, sizeof(CH8_INSTR) );
	memcpy( ch8_State->M, _ch8Font, sizeof( _ch8Font ) );

	ch8_State->PC = 0x200;
	ch8_State->StackPointer = ch8_State->CallStack;
	ch8_State->PausedOnKey = -1;

	srand (time(NULL));

	printf( "Called!\n" );

	/* OS specific init */
	ch8_OS_Init();
}

void ch8_StartVM()
{
	int key;
	uint32_t prev_tick_time, tick_time;

	prev_tick_time = tick_time = 0;
	while( 1 )
	{
		if( ch8_OS_tick( &tick_time ) == 1 )
			break;

		key = ch8_OS_ReadKeys();

		if( ch8_State->PausedOnKey != -1 && key != -1)
		{
			_V[ch8_State->PausedOnKey] = key;
			ch8_State->PausedOnKey = -1;			
		}

		if( ch8_State->PausedOnKey == -1 )
		{
			ch8_execInstr();
			//ch8_printState();
		}
				
		// Timers down at 60Hz
		if( tick_time > prev_tick_time + 1000/60 )
		{
			if( _DTIM > 0)
			{
		
				_DTIM--;
			}
			if( _STIM > 0)
			{
				_STIM--;
			}

			prev_tick_time = tick_time;
		}
		
		usleep( 1000000/480 );
	}
}

void ch8_execInstr()
{
 	short opcode = _M[_PC] + ((short)_M[_PC+1] << 8);
	ch8p_read_opcode( opcode, ch8_Instr );
	ch8p_print_instr( ch8_Instr );

	if( ch8_Instr->code != 0 )
	{
		_PC += 2;
		CallTable[ch8_Instr->code]();
	}else{
		printf( "Error : Trying to execute unrecognized opcode %X at %X\n", ch8_Instr->code, _PC );
	}
}

void ch8_printState()
{
	printf( "\n\t- Current state: I=%.4X PC=%.4X", _I, _PC );
	for( int i=0; i <= 0xF; ++i )
	{
		if( i%4 == 0 )
			printf( "\n\t" );
		printf( "V[%X] = %.2X ", i, _V[i] );
	}
	printf( "\n\n" );
}

/**********************
   CHIP8 INSTRUCTIONS 
 **********************/

void ch8_SCDOWN ()
{
	printf( "Error: ch8_SCDOWN not yet implemented\n" );
}

void ch8_CLS ()
{
	memset( _Scr, 0, sizeof(_Scr) );
}

void ch8_RTS ()
{
	_PC = *(--_SP);
}

void ch8_SCRIGHT ()
{
	printf( "Error: ch8_SCRIGHT not yet implemented\n" );
}

void ch8_SCLEFT ()
{
	printf( "Error: ch8_SCLEFT not yet implemented\n" );
}

void ch8_EXIT ()
{
	printf( "CH8 Exits...\n" );
	exit(0);
}

void ch8_LOW ()
{
	printf( "Error: ch8_LOW not yet implemented\n" );
}

void ch8_HIGH ()
{
	printf( "Error: ch8_HIGH not yet implemented\n" );
}


void ch8_JMP ()
{
	_PC = _X;
}

void ch8_CALL ()
{
	*(_SP++) = _PC;
	_PC = _X;
}

void ch8_SKEQ_K ()
{
	if( _V[_X] == _Y )
		_PC += 2;
}

void ch8_SKNE_K ()
{
	if( _V[_X] != _Y )
		_PC += 2;
}

void ch8_SKEQ ()
{
	if( _V[_X] == _V[_Y] )
		_PC += 2;
}


void ch8_MOV_K ()
{
	_V[_X] = (unsigned char)_Y;
}

void ch8_ADD_K ()
{
	_V[_X] += _Y;
}


void ch8_MOV ()
{
	_V[_X] = _V[_Y];
}

void ch8_OR ()
{
	_V[_X] |= _V[_Y];
}

void ch8_AND ()
{
	_V[_X] &= _V[_Y];
}

void ch8_XOR ()
{
	_V[_X] ^= _V[_Y];
}

void ch8_ADD ()
{
	uint16_t res;

	res = (short)_V[_X] + (short)_V[_Y];
	_V[_X] = (uint8_t)(res & 0x00FF);
	if( res > 0xFF )
		_V[0xF] = 1;
	else
		_V[0xF] = 0;
}

void ch8_SUB ()
{	
	if( _V[_X] >= _V[_Y] )
	{		
		_V[_X] -= _V[_Y];
		_V[0xF] = 1;
	}else{
		_V[0xF] = 0;
		_V[_X] = 0xFF - _V[_Y] + _V[_X] + 1;
	}	
}

void ch8_SHR ()
{
	_V[0xF] = _V[_X] & 1;
	_V[_X] >>= 1;
}

void ch8_RSB ()
{	
	if( _V[_Y] >=  _V[_X] )
	{
		_V[_X] = _V[_Y] - _V[_X];
		_V[0xF] = 1;
	}else {
		_V[_X] = 0xFF - _V[_X] + _V[_Y] + 1;
		_V[0xF] = 0;
	}
}

void ch8_SHL ()
{
	_V[0xF] = (_V[_X] & 0x80) >> 7;
	_V[_X] <<= 1;
}


void ch8_SKNE ()
{
	if( _V[_X] != _V[_Y] )
		_PC += 2;
}


void ch8_MVI ()
{
	_I = _X;
}

void ch8_JMI ()
{
	_PC = _V[0] + _X;
}

void ch8_RAND ()
{
	_V[_X] = (uint8_t)(rand() % (_Y+1));
	printf( "rand:%X\n", _V[_X] );
}


void ch8_SPRITE ()
{
	uint8_t line;
	uint8_t* sprite = _M+_I;
	uint8_t* scr;
	uint8_t  oldscr;
	uint8_t x, y;

	_V[0xF] = 0;

	x = (_V[_X]) % CH8_SCREEN_WIDTH;
	y = (_V[_Y]) % CH8_SCREEN_HEIGHT;

	for( int i=y; i < y+_Z; ++i )
	{
		/* Sprite line to draw */
		line = *(sprite++);
		
		/* locate first byte */
		scr = _Scr + (i%CH8_SCREEN_HEIGHT) * CH8_SCREEN_WIDTH/8;		
		scr += x/8;

		oldscr = *scr;
		for( int j=x%8; j<8; ++j )
		{
			*scr ^= (line&0x80) >> (7-j);
			line <<= 1;
		}
		if( ((oldscr^(*scr)) & oldscr) != 0 )	/* has a bit gone from 1 to 0? */
			_V[0xF]=1;

		/* locate second byte */
		scr = _Scr + (i%CH8_SCREEN_HEIGHT) * CH8_SCREEN_WIDTH/8;		
		scr += ((x+8-(x%8))%CH8_SCREEN_WIDTH)/8;

		oldscr = *scr;
		for( int j=0; j<x%8; ++j )
		{
			*scr ^= (line&0x80) >> (7-j);
			line <<= 1;
		}
		if( ((oldscr^*scr) & oldscr) != 0 )
			_V[0xF]=1;
	}

	ch8_OS_PrintScreen();
}


void ch8_SKPR ()
{
	if( _KEY[_V[_X]] != 0 )
		_PC += 2;
}

void ch8_SKUP ()
{
	if( _KEY[_V[_X]] == 0 )
		_PC += 2;
}


void ch8_GDELAY ()
{
	_V[_X] = _DTIM;
}

void ch8_KEY ()
{
	ch8_State->PausedOnKey = _X;
}

void ch8_SDELAY ()
{
	_DTIM = _V[_X];
}

void ch8_SSOUND ()
{
	_STIM = _V[_X];
}

void ch8_ADI ()
{
	_I += _V[_X];
}

void ch8_FONT ()
{
	_I = _V[_X]*5;
}

void ch8_XFONT ()
{
	printf( "Error: ch8_XFONT not yet implemented\n" );
}

void ch8_BCD ()
{
	_M[_I] = (_V[_X]/100);
	_M[_I+1] = (_V[_X]%100)/10;
	_M[_I+2] = (_V[_X]%100)%10;
}

void ch8_STR ()
{
	for( int i=0; i<=_X; ++i )
	{
		_M[_I + i] = _V[i];
	}
}

void ch8_LDR ()
{
	for( int i=0; i<=_X; ++i )
	{
		_V[i] = _M[_I + i];
	}
}

void ch8_STR_RPL ()
{
	printf( "Error: ch8_STR_RPL not yet implemented\n" );
}

void ch8_LDR_RPL ()
{
	printf( "Error: ch8_LDR_RPL not yet implemented\n" );
}


/* Call Table */
CH8_CALL CallTable[CH8_INSTR_COUNT+1] = 
{
	NULL,
	
	/* Type 1 instr: screen */
	ch8_SCDOWN,
	ch8_CLS,
	ch8_RTS,
	ch8_SCRIGHT,
	ch8_SCLEFT,
	ch8_EXIT,
	ch8_LOW,
	ch8_HIGH,
	
	/* Type 2 instr: control */
	ch8_JMP,
	ch8_CALL,
	ch8_SKEQ_K,
	ch8_SKNE_K,
	ch8_SKEQ,
	
	/* Type 3: constant op */
	ch8_MOV_K,
	ch8_ADD_K,
	
	/* Type 4: register op */
	ch8_MOV,
	ch8_OR,
	ch8_AND,
	ch8_XOR,
	ch8_ADD,
	ch8_SUB,
	ch8_SHR,
	ch8_RSB,
	ch8_SHL,

	ch8_SKNE,
	
	/* Type 5 */
	ch8_MVI,
	ch8_JMI,
	ch8_RAND,
	
	/* Type 6: drawing op */
	ch8_SPRITE,
	
	/* Type 7: keyboard */
	ch8_SKPR,
	ch8_SKUP,
	
	/* Type 8: counters */
	ch8_GDELAY,
	ch8_KEY,
	ch8_SDELAY,
	ch8_SSOUND,
	ch8_ADI,
	ch8_FONT,
	ch8_XFONT,
	ch8_BCD,
	ch8_STR,
	ch8_LDR,
	ch8_STR_RPL,
	ch8_LDR_RPL
};
