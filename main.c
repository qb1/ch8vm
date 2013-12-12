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
   	
   	// LLVMValueRef rommem_var = LLVMGetNamedGlobal( module, "rom_memory" );
   	// LLVMValueRef rommem_var = LLVMAddGlobal( module,  type, "rom_memory" );
   	// LLVMDeleteGlobal( rommem_var );
   	LLVMValueRef rommem_var = LLVMAddGlobal( module, rommem_type, "rom_memory");
   	LLVMSetInitializer( rommem_var, LLVMConstStringInContext( context, (char*)rom_memory, fileLen, 1 ) );

   	// LLVMValueRef romsize_var = LLVMGetNamedGlobal( module, "rom_memory_size" );
   	// LLVMValueRef romsize_var = LLVMAddGlobal( module,  LLVMInt16TypeInContext(context), "rom_memory_size" );
   	// LLVMSetInitializer( romsize_var, LLVMConstInt( LLVMInt16TypeInContext(context), fileLen, 0 ) );

   	// LLVMDumpModule( module ); 
   	// LLVMVerifyModule( module, LLVMPrintMessageAction, NULL );

   	// Create a new function
   	LLVMTypeRef vmprog_type = LLVMFunctionType( LLVMVoidTypeInContext(context), NULL, 0, 0 );
   	LLVMValueRef vmprog_func = LLVMAddFunction( module, "vm_program", vmprog_type );
   	LLVMAddFunctionAttr( vmprog_func, LLVMNoUnwindAttribute );

   	// Fill it with a simple program

   	LLVMBuilderRef builder = LLVMCreateBuilderInContext( context );
   	LLVMBasicBlockRef bb = LLVMAppendBasicBlockInContext( context, vmprog_func, "" );
   	LLVMPositionBuilder (builder, bb, LLVMGetFirstInstruction(bb));

   	LLVMValueRef mem_ptr = LLVMBuildGEP( builder, rommem_var, NULL, 0, "" );

   	LLVMValueRef args[2];

   	args[0] = mem_ptr;
   	args[1] = LLVMConstInt( LLVMInt16TypeInContext(context), fileLen, 0 );

   	LLVMBuildCall( builder, LLVMGetNamedFunction( module, "ch8_InitVM"), args, 2, "" );

    LLVMValueRef op_arg[3];
   	op_arg[0] = LLVMConstInt( LLVMInt16TypeInContext(context), 1, 0 );
   	op_arg[1] = LLVMConstInt( LLVMInt16TypeInContext(context), 0, 0 );
   	op_arg[2] = LLVMConstInt( LLVMInt16TypeInContext(context), 0, 0 );
   	LLVMValueRef call = LLVMBuildCall( builder, LLVMGetNamedFunction( module, "ch8_CLS"), op_arg, 3, "" );
   	LLVMSetFunctionCallConv( call, LLVMCCallConv );

   	LLVMBuildRetVoid( builder );

	LLVMDumpModule( module ); 

	// LLVMPassManagerRef passmgr = LLVMCreatePassManager( );
	// LLVMRunPassManager( passmgr, module );
	// LLVMDisposePassManager( passmgr );

	// LLVMWriteBitcodeToFile( module, "output.bc" );

	// LLVMVerifyModule( module, LLVMPrintMessageAction, NULL );

	// Call the VM
   	if(LLVMCreateExecutionEngineForModule(&exec_engine, module, &err) )
   	{
        fprintf (stderr, "error: %s\n", err);
        LLVMDisposeMessage (err);
        return 1;
    }

 //    const char * const main_func_args [] = {"ch8vm"};
 //    LLVMValueRef main_func = LLVMGetNamedFunction( module, "main");
	// LLVMRunFunctionAsMain( exec_engine, main_func, 1, main_func_args, NULL );

	LLVMRunFunction( exec_engine, vmprog_func, 0, NULL );

	// Write to file
	// if( LLVMVerifyModule( module, LLVMPrintMessageAction, NULL ) )
	// 	exit(1);

    

    return 0;

	// return 0;
}