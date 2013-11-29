#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ch8vm.h"

int main(int argc, char *argv[])
{
	FILE* file_program=NULL;
	unsigned long fileLen;
	
	CH8_STATE state;
	CH8_INSTR instr;
	
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

	/* Init VM */
	ch8_InitVM( &state, &instr );

	/* Read file contents into buffer */
	fseek(file_program, 0, SEEK_END);
	fileLen=ftell(file_program);
	fseek(file_program, 0, SEEK_SET);
	fread(state.M+0x200, fileLen, 1, file_program);
	fclose(file_program);

	ch8_StartVM();

	return 0;
}