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