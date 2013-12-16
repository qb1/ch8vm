#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ch8vm.h"
#include "ch8vm_llvm.h"

int main(int argc, char *argv[])
{
	FILE* file_program=NULL;
	unsigned long fileLen;	
	
	uint8_t *rom_memory;

	if( argc != 2 )
	{
		printf( "Error : no input file specified\n" );
		return 1;
	}
	
	file_program = fopen( argv[1], "rb" );
	
	if( !file_program )
	{
		printf( "Error : unable to open specified input file\n" );
		return 1;
	}

	/* Read file contents into buffer */
	fseek(file_program, 0, SEEK_END);
	fileLen=ftell(file_program);
	fseek(file_program, 0, SEEK_SET);

	rom_memory = malloc(fileLen);
	fread(rom_memory, fileLen, 1, file_program);
	fclose(file_program);

	// Begin exp.
	ch8_ll_Init( "ch8vmlib.bc", rom_memory, fileLen );


	
	// ch8_ll_AddOpcodeCall( "ch8_CLS", 0, 0, 0, 0x200 );
	// ch8_ll_AddOpcodeCall( "ch8_MOV_K", 3, 0, 0, 0x08 );
	// ch8_ll_AddOpcodeCall( "ch8_MOV_K", 4, 200, 0, 0x09 );
	// ch8_ll_AddOpcodeCall( "ch8_MVI", 0x248, 0, 0, 0x202 );
	// ch8_ll_AddOpcodeCall( "ch8_MOV_K", 0, 0, 0, 0x204 );
	// ch8_ll_AddOpcodeCall( "ch8_MOV_K", 1, 0x1E, 0, 0x206 );
	// ch8_ll_AddOpcodeCall( "ch8_MOV_K", 2, 0, 0, 0x208 );
	// ch8_ll_AddOpcodeCall( "ch8_ADD_K", 3, 1, 0, 0x209 );


	// ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 0, 2, 0x210 );
	// ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 1, 2, 0x212 );

	// ch8_ll_AddCondEq( 3, 4, 0x214 );	
	// ch8_ll_AddJump( 0x208, 0x216 );	
	// ch8_ll_AddOpcodeCall( "ch8_EXIT", 0, 0, 0, 0x218 );

	CH8_INSTR instr;
	int offset;

	for( offset=0; offset+1 < fileLen; offset += 2 )
	{
		short opcode = rom_memory[offset] + ((short)rom_memory[offset+1] << 8);
		ch8p_read_opcode( opcode, &instr );
		printf( "0x%.4X - ", 0x200+offset );
		ch8p_print_instr( &instr );

		ch8p_decode_instr_llvm( &instr, 0x200+offset );		
	}

	for( offset=1; offset+1 < fileLen; offset += 2 )
	{
		short opcode = rom_memory[offset] + ((short)rom_memory[offset+1] << 8);
		ch8p_read_opcode( opcode, &instr );
		printf( "0x%.4X - ", 0x200+offset );
		ch8p_print_instr( &instr );

		ch8p_decode_instr_llvm( &instr, 0x200+offset );
	}

	ch8_ll_EndCompilation();

	ch8_ll_DumpOnStdout( );
	ch8_ll_RunJIT();

    return 0;
}