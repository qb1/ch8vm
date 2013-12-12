#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/BitReader.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Analysis.h>

#include "ch8vm.h"

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
	char *err;

	LLVMModuleRef module;
	// LLVMBuilderRef builder;
	// LLVMPassManagerRef pass_mgr;
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

   	// Add the global state's initialiser
   	LLVMContextRef context = LLVMGetModuleContext(module);
   	LLVMTypeRef rommem_type = LLVMArrayType( LLVMInt8TypeInContext(context), fileLen);
   	
   	LLVMValueRef rommem_var = LLVMGetNamedGlobal( module, "rom_memory" );
   	LLVMDeleteGlobal( rommem_var );
   	rommem_var = LLVMAddGlobal( module, rommem_type, "rom_memory");
   	LLVMSetInitializer( rommem_var, LLVMConstStringInContext( context, (char*)rom_memory, fileLen, 1 ) );

   	LLVMValueRef romsize_var = LLVMGetNamedGlobal( module, "rom_memory_size" );
   	LLVMSetInitializer( romsize_var, LLVMConstInt( LLVMInt16TypeInContext(context), fileLen, 0 ) );   	
   	
	//LLVMDumpModule( module ); 

	LLVMPassManagerRef passmgr = LLVMCreatePassManager( );
	LLVMRunPassManager( passmgr, module );
	LLVMDisposePassManager( passmgr );

	// LLVMWriteBitcodeToFile( module, "output.bc" );

	// Call the VM
   	if(LLVMCreateExecutionEngineForModule(&exec_engine, module, &err) )
   	{
        fprintf (stderr, "error: %s\n", err);
        LLVMDisposeMessage (err);
        return 1;
    }
    
    const char * const main_func_args [] = {"ch8vm"};
    LLVMValueRef main_func = LLVMGetNamedFunction( module, "main");
	LLVMRunFunctionAsMain( exec_engine, main_func, 1, main_func_args, NULL );

	// Write to file
	// if( LLVMVerifyModule( module, LLVMPrintMessageAction, NULL ) )
	// 	exit(1);

    

    return 0;

	// return 0;
}