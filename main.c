#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/BitReader.h>

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

	// Begin exp.

	char *err;

	LLVMModuleRef module;
	// LLVMBuilderRef builder;
	// LLVMPassManagerRef pass_mgr;
	// ExistingModuleProvider mp;
	LLVMExecutionEngineRef exec_engine;
	LLVMMemoryBufferRef buffer;	

	LLVMInitializeNativeTarget();
	LLVMLinkInJIT ();

	if( LLVMCreateMemoryBufferWithContentsOfFile("ch8vmlib.bc", &buffer, &err) ) 
	{
		fprintf (stderr, "error: %s\n", err);
    	LLVMDisposeMessage (err);
    	return 1;
    }

    if( LLVMParseBitcode(buffer, &module, &err) )
    {
    	fprintf (stderr, "error: %s\n", err);
    	LLVMDisposeMessage (err);
    	return 1;
    }

    // VM loaded!
   	// LLVMDumpModule( module );   

   	if(LLVMCreateExecutionEngineForModule(&exec_engine, module, &err) )
   	{
        fprintf (stderr, "error: %s\n", err);
        LLVMDisposeMessage (err);
        return 1;
    }

    // Simple call
    LLVMValueRef test_func = LLVMGetNamedFunction( module, "ch8_test");
    LLVMRunFunction( exec_engine, test_func, 0, NULL );

    // Call with arguments
    LLVMValueRef init_ch8 = LLVMGetNamedFunction( module, "ch8_InitVM");
    LLVMGenericValueRef args[2];
    args[0] = LLVMCreateGenericValueOfPointer( &state );
    args[1] = LLVMCreateGenericValueOfPointer( &instr );
	LLVMRunFunction( exec_engine, init_ch8, 2, args );

	/* Read file contents into buffer */
	fseek(file_program, 0, SEEK_END);
	fileLen=ftell(file_program);
	fseek(file_program, 0, SEEK_SET);
	fread(state.M+0x200, fileLen, 1, file_program);
	fclose(file_program);

	// Simple call
    LLVMValueRef start_func = LLVMGetNamedFunction( module, "ch8_StartVM");
    LLVMRunFunction( exec_engine, start_func, 0, NULL );

   	return 0;

	// module = LLVMModuleCreateWithName ("CHIP-8 JIT");
 //    builder = LLVMCreateBuilder ();

 //    LLVMInitializeNativeTarget ();
 //    LLVMLinkInJIT ();    

 //    if (LLVMCreateExecutionEngineForModule (&exec_engine, module, &err)) {
 //        fprintf (stderr, "error: %s\n", err);
 //        LLVMDisposeMessage (err);
 //        return 1;
 //    }



	// /* Init VM */
	// ch8_InitVM( &state, &instr );

	// /* Read file contents into buffer */
	// fseek(file_program, 0, SEEK_END);
	// fileLen=ftell(file_program);
	// fseek(file_program, 0, SEEK_SET);
	// fread(state.M+0x200, fileLen, 1, file_program);
	// fclose(file_program);

	// ch8_StartVM();

	return 0;
}