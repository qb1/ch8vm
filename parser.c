#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "parser.h"
#include "ch8vm_llvm.h"

/* 4 bytes representing each insctruction :
		Each hex value <= 0xF must be exactly matched
		Each 0x10 value will be placed into param1 
		Each 0x11 value will be placed into param2
*/

static char instr_parser_repr[CH8_INSTR_COUNT][4] =
{
	{ 0x0, 0x0, 0xC, 0x10 },
	{ 0x0, 0x0, 0xE, 0x0 },
	{ 0x0, 0x0, 0xE, 0xE },
	{ 0x0, 0x0, 0xF, 0xB },
	{ 0x0, 0x0, 0xF, 0xC },
	{ 0x0, 0x0, 0xF, 0xD },
	{ 0x0, 0x0, 0xF, 0xE },
	{ 0x0, 0x0, 0xF, 0xF },
	{ 0x1, 0x10, 0x10, 0x10 },
	{ 0x2, 0x10, 0x10, 0x10 },
	{ 0x3, 0x10, 0x11, 0x11 },
	{ 0x4, 0x10, 0x11, 0x11 },
	{ 0x5, 0x10, 0x11, 0x0 },
	{ 0x6, 0x10, 0x11, 0x11 },
	{ 0x7, 0x10, 0x11, 0x11 },
	{ 0x8, 0x10, 0x11, 0x0 },
	{ 0x8, 0x10, 0x11, 0x1 },
	{ 0x8, 0x10, 0x11, 0x2 },
	{ 0x8, 0x10, 0x11, 0x3 },
	{ 0x8, 0x10, 0x11, 0x4 },
	{ 0x8, 0x10, 0x11, 0x5 },
	{ 0x8, 0x10, 0x11, 0x6 },
	{ 0x8, 0x10, 0x11, 0x7 },
	{ 0x8, 0x10, 0x11, 0xE },
	{ 0x9, 0x10, 0x11, 0x0 },
	{ 0xA, 0x10, 0x10, 0x10 },
	{ 0xB, 0x10, 0x10, 0x10 },
	{ 0xC, 0x10, 0x11, 0x11 },
	{ 0xD, 0x10, 0x11, 0x12 },
	{ 0xE, 0x10, 0x9, 0xE },
	{ 0xE, 0x10, 0xA, 0x1 },
	{ 0xF, 0x10, 0x0, 0x7 },
	{ 0xF, 0x10, 0x0, 0xA },
	{ 0xF, 0x10, 0x1, 0x5 },
	{ 0xF, 0x10, 0x1, 0x8 },
	{ 0xF, 0x10, 0x1, 0xE },
	{ 0xF, 0x10, 0x2, 0x9 },
	{ 0xF, 0x10, 0x3, 0x0 },
	{ 0xF, 0x10, 0x3, 0x3 },
	{ 0xF, 0x10, 0x5, 0x5 },
	{ 0xF, 0x10, 0x6, 0x5 },
	{ 0xF, 0x10, 0x7, 0x5 },
	{ 0xF, 0x10, 0x8, 0x5 }
};

const char* opcodeFuncNames[] = 
{
	"ch8_NONE",
	
	/* Type 1 instr: screen */
	"ch8_SCDOWN",
	"ch8_CLS",
	"ch8_RTS",
	"ch8_SCRIGHT",
	"ch8_SCLEFT",
	"ch8_EXIT",
	"ch8_LOW",
	"ch8_HIGH",
	
	/* Type 2 instr: control */
	"ch8_JMP",
	"ch8_CALL",
	"ch8_SKEQ_K",
	"ch8_SKNE_K",
	"ch8_SKEQ",
	
	/* Type 3: constant op */
	"ch8_MOV_K",
	"ch8_ADD_K",
	
	/* Type 4: register op */
	"ch8_MOV",
	"ch8_OR",
	"ch8_AND",
	"ch8_XOR",
	"ch8_ADD",
	"ch8_SUB",
	"ch8_SHR",
	"ch8_RSB",
	"ch8_SHL",

	"ch8_SKNE",
	
	/* Type 5 */
	"ch8_MVI",
	"ch8_JMI",
	"ch8_RAND",
	
	/* Type 6: drawing op */
	"ch8_SPRITE",
	
	/* Type 7: keyboard */
	"ch8_SKPR",
	"ch8_SKUP",
	
	/* Type 8: counters */
	"ch8_GDELAY",
	"ch8_KEY",
	"ch8_SDELAY",
	"ch8_SSOUND",
	"ch8_ADI",
	"ch8_FONT",
	"ch8_XFONT",
	"ch8_BCD",
	"ch8_STR",
	"ch8_LDR",
	"ch8_STR_RPL",
	"ch8_LDR_RPL",
};

static const char* instr_desc[CH8_INSTR_COUNT] = 
{
	"ch8_SCDOWN : Scroll display %X lines down",
	"ch8_CLS : Clear display ",
	"ch8_RTS : Return from subroutine",
	"ch8_SCRIGHT display 4 pixels right",
	"ch8_SCLEFT : Scroll display 4 pixels left",
	"ch8_EXIT Exit CHIP interpreter",
	"Disable extended screen mode",
	"Enable extended screen mode for full-screen graphics",
	
	
	"ch8_JMP Jump to %X ",
	"ch8_CALL Call subroutine at %X ",
	"Skip next instruction if V%X == %X ",
	"Skip next instruction if V%X <> %X ",
	"Skip next instruction if V%X == V%X ",
	
	
	"ch8_MOV_K V%X := %X ",
	"ch8_ADD_K V%X := + %X ",
	
	
	"ch8_MOV V%X := V%X, VF may change ",
	"ch8_OR V%X := or V%X, VF may change ",
	"ch8_AND V%X := and V%X, VF may change ",
	"ch8_XOR V%X := xor V%X, VF may change",
	"ch8_ADD V%X := + V%X, VF := carry ",
	"ch8_SUB V%X := - V%X, VF := not borrow ",
	"ch8_SHR V%X := shr 1, VF := carry ",
	"ch8_RSB V%X := V%X - VX, VF := not borrow",
	"ch8_SHL V%X := shl 1, VF := carry ",
	
	"Skip next instruction if V%X <> V%X ",
	
	
	"ch8_MVI I := %X ",
	"ch8_JMI Jump to %X+V0 ",
	"V%X := pseudorandom_number and %X ",
	
	
	"ch8_SPRITE Show N-byte sprite from M(I) at coords (V%X,V%X), N=%X, VF := collision.",//\n\tIf N=0 and extended mode, show 16x16 sprite.",
	
	
	"Skip next instruction if key V%X pressed ",
	"Skip next instruction if key V%X not pressed ",
	
	
	"ch8_GDELAY V%X := delay_timer ",
	"ch8_KEY wait for keypress, store hex value of key in V%X ",
	"ch8_SDELAY delay_timer := V%X ",
	"ch8_SSOUND sound_timer := V%X ",
	"ch8_ADI I := I + V%X ",
	"ch8_FONT Point I to 5-byte font sprite for hex character V%X ",
	"ch8_XFONT Point I to 10-byte font sprite for digit V%X (0..9)",
	"ch8_BCD Store BCD representation of V%X in M(I)..M(I+2) ",
	"ch8_STR Store V0..V%X in memory starting at M(I) ",
	"ch8_LDR Read V0..V%X from memory starting at M(I) ",
	"Store V0..V%X in RPL user flags (X <= 7)",
	"Read V0..V%X from RPL user flags (X <= 7)" 
};

void clear_instr( CH8_INSTR* instr )
{
	instr->code = NONE;
	instr->param1 = 0;
	instr->param2 = 0;
	instr->param3 = 0;
}

int ch8p_read_opcode( unsigned short opcode, CH8_INSTR* instr )
{
	clear_instr( instr );
	char expl_instr[4];
	
	CH8_OPCODE code = NONE;
	short param1 = 0;
	short param2 = 0;
	short param3 = 0;
	
	/* explode instr */
	expl_instr[1] = opcode & 0xF;
	expl_instr[0] = (opcode>>4) & 0xF;
	expl_instr[3] = (opcode>>8) & 0xF;
	expl_instr[2] = (opcode>>12) & 0xF;
	
	/* browse through instr set */
	for( int i=0; i<CH8_INSTR_COUNT; ++i )
	{
		param1 = 0;
		param2 = 0;
		param3 = 0;
		code	= (CH8_OPCODE)(i+1);
		
		for( int j=0; j<4; ++j )
		{			
			if( instr_parser_repr[i][j] < 0x10 && instr_parser_repr[i][j] != expl_instr[j] )
			{
				/* Unmatch with instr opcode, break */
				code = NONE;
				break;
			}
			if( instr_parser_repr[i][j] == 0x10 )
			{
				/* Put into param1 */
				param1 = (param1 << 4) + expl_instr[j];
			}
			if( instr_parser_repr[i][j] == 0x11 )
			{
				/* Put into param2 */
				param2 = (param2 << 4) + expl_instr[j];
			}
			if( instr_parser_repr[i][j] == 0x12 )
			{
				/* Put into param2 */
				param3 = (param3 << 4) + expl_instr[j];
			}
		}
		
		if( code != NONE )
		{
			// We found our opcode
			break;
		}
	}
	
	if( code == NONE )
	{		
		instr->param1 = opcode;
		return 0;
	}else{
		instr->code = code;
		instr->param1 = param1;
		instr->param2 = param2;
		instr->param3 = param3;
	}

	return 1;
}

void ch8p_print_instr( CH8_INSTR* instruction )
{
	if( instruction->code == NONE )
	{
		printf( "Data : %.2X%.2X\n", instruction->param1>>8&0xFF, instruction->param1&0xFF );
	}else{
		printf( instr_desc[(int)instruction->code-1], instruction->param1, instruction->param2, instruction->param3 );
		printf( "\n" );
	}
}

void ch8p_decode_instr_llvm( CH8_INSTR* instr, uint16_t address )
{
	switch( instr->code )
	{
		case 0:
			// Not an instruction
			break;

		case SKEQ_K:
			ch8_ll_AddCondEqV( instr->param1, instr->param2, address );
			break;
		case SKNE_K:
			ch8_ll_AddCondNEqV( instr->param1, instr->param2, address );
			break;
		case SKEQ:
			ch8_ll_AddCondEq( instr->param1, instr->param2, address );
			break;
		case SKNE:
			ch8_ll_AddCondNEq( instr->param1, instr->param2, address );
			break;

		case JMP:
			ch8_ll_AddJump( instr->param1, address );
			break;

		case CALL:
		case JMI:
			printf( "Unsupported instruction\n" );
			ch8_ll_AddOpcodeCall( "ch8_UNSUPP", 
				instr->param1, instr->param2, instr->param3, address );
			break;

		default:
			ch8_ll_AddOpcodeCall( opcodeFuncNames[instr->code], 
				instr->param1, instr->param2, instr->param3, address );
	}
}