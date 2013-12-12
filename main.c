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
	
	ch8_ll_AddOpcodeCall( "ch8_CLS", 0, 0, 0 );
	ch8_ll_AddOpcodeCall( "ch8_MVI", 0x248, 0, 0 );
	ch8_ll_AddOpcodeCall( "ch8_MOV_K", 0, 0, 0 );
	ch8_ll_AddOpcodeCall( "ch8_MOV_K", 1, 0x1E, 0 );
	ch8_ll_AddOpcodeCall( "ch8_MOV_K", 2, 0, 0 );



	ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 0, 2 );
	ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 1, 2 );

	ch8_ll_AddOpcodeCall( "ch8_MOV_K", 2, 0x8, 0 );

	ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 0, 2 );
	ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 1, 2 );

	ch8_ll_AddOpcodeCall( "ch8_MOV_K", 2, 0x8, 0 );

	ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 0, 2 );
	ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 1, 2 );

	ch8_ll_AddOpcodeCall( "ch8_MOV_K", 2, 0x8, 0 );

	ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 0, 2 );
	ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 1, 2 );

	ch8_ll_AddOpcodeCall( "ch8_MOV_K", 2, 0x8, 0 );

	ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 0, 2 );
	ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 1, 2 );

	ch8_ll_AddOpcodeCall( "ch8_MOV_K", 2, 0x8, 0 );

	ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 0, 2 );
	ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 1, 2 );

	ch8_ll_AddOpcodeCall( "ch8_MOV_K", 2, 0x8, 0 );

	ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 0, 2 );
	ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 1, 2 );

	ch8_ll_AddOpcodeCall( "ch8_MOV_K", 2, 0x8, 0 );

	ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 0, 2 );
	ch8_ll_AddOpcodeCall( "ch8_SPRITE", 2, 1, 2 );

	ch8_ll_EndCompilation();
	ch8_ll_RunJIT();

	getchar();

    return 0;
}