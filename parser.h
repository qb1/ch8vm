#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

typedef enum CH8_OPCODE
{
	NONE,
	
	/* Type 1 instr: screen */
	SCDOWN,
	CLS,
	RTS,
	SCRIGHT,
	SCLEFT,
	EXIT,
	LOW,
	HIGH,
	
	/* Type 2 instr: control */
	JMP,
	CALL,
	SKEQ_K,
	SKNE_K,
	SKEQ,
	
	/* Type 3: constant op */
	MOV_K,
	ADD_K,
	
	/* Type 4: register op */
	MOV,
	OR,
	AND,
	XOR,
	ADD,
	SUB,
	SHR,
	RSB,
	SHL,

	SKNE,
	
	/* Type 5 */
	MVI,
	JMI,
	RAND,
	
	/* Type 6: drawing op */
	SPRITE,
	
	/* Type 7: keyboard */
	SKPR,
	SKUP,
	
	/* Type 8: counters */
	GDELAY,
	KEY,
	SDELAY,
	SSOUND,
	ADI,
	FONT,
	XFONT,
	BCD,
	STR,
	LDR,
	STR_RPL,
	LDR_RPL,
}CH8_OPCODE;

#define CH8_INSTR_COUNT 43

typedef struct
{
	CH8_OPCODE code;
	uint16_t param1;
	uint16_t param2;
	uint16_t param3;

	//short address;
} CH8_INSTR;

int ch8p_read_opcode( uint16_t opcode, CH8_INSTR* instruction );
void ch8p_print_instr( CH8_INSTR* instruction );

#endif // PARSER_H