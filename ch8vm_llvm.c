#include <stdio.h>

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/BitReader.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Analysis.h>

#include "ch8vm_llvm.h"

LLVMModuleRef module;
LLVMContextRef context;
LLVMBuilderRef builder;

LLVMValueRef vmprog_func;

void ch8_ll_Init( const char* module_filename, const uint8_t *memory, uint16_t mem_size )
{
	char *err;
	LLVMMemoryBufferRef buffer;

	LLVMInitializeNativeTarget();
	LLVMLinkInJIT ();

	if( LLVMCreateMemoryBufferWithContentsOfFile(module_filename, &buffer, &err) ) 
	{
		printf ("error: %s\n", err);
    	LLVMDisposeMessage (err);
    	return;
    }

    if( LLVMParseBitcode(buffer, &module, &err) )
    {
    	printf("error: %s\n", err);
    	LLVMDisposeMessage (err);
    	return;
    }

    LLVMDisposeMemoryBuffer( buffer );

    context = LLVMGetModuleContext(module);
    builder = LLVMCreateBuilderInContext( context );

    // Add the global variable filled with ROM's content
   	LLVMTypeRef rommem_type = LLVMArrayType( LLVMInt8TypeInContext(context), mem_size);   	   	
   	LLVMValueRef rommem_var = LLVMAddGlobal( module, rommem_type, "rom_memory");
   	LLVMSetInitializer( rommem_var, LLVMConstStringInContext( context, (char*)memory, mem_size, 1 ) );

	// Create a new function
   	LLVMTypeRef vmprog_type = LLVMFunctionType( LLVMVoidTypeInContext(context), NULL, 0, 0 );
   	vmprog_func = LLVMAddFunction( module, "vm_program", vmprog_type );
   	LLVMAddFunctionAttr( vmprog_func, LLVMNoUnwindAttribute );

   	// Create its basic block
   	LLVMBasicBlockRef bb = LLVMAppendBasicBlockInContext( context, vmprog_func, "" );

   	// Place the builder at the end of this block
   	LLVMPositionBuilder (builder, bb, LLVMGetFirstInstruction(bb));   	

   	// Fill it with initialisation call to ch8_InitVM
   	LLVMValueRef args[2];
   	args[0] = LLVMBuildGEP( builder, rommem_var, NULL, 0, "" ); 	// Retrieve pointer to global var
   	args[1] = LLVMConstInt( LLVMInt16TypeInContext(context), mem_size, 0 );
   	LLVMBuildCall( builder, LLVMGetNamedFunction( module, "ch8_InitVM"), args, 2, "" );
}

void ch8_ll_DumpOnDisk( const char* filename )
{
	LLVMWriteBitcodeToFile( module, filename );
}

void ch8_ll_RunJIT( )
{
	char *err;
	LLVMExecutionEngineRef exec_engine;

	// LLVMValueRef call = LLVMBuildCall( builder, LLVMGetNamedFunction(module, "ch8_StartVM"), NULL, 0, "" );
 //   	LLVMSetFunctionCallConv( call, LLVMCCallConv );
 //   	LLVMBuildRetVoid( builder );

	LLVMVerifyModule( module, LLVMPrintMessageAction, NULL );

	// Call the VM
   	if(LLVMCreateExecutionEngineForModule(&exec_engine, module, &err) )
   	{
        fprintf (stderr, "error: %s\n", err);
        LLVMDisposeMessage (err);
        return;
    }

    LLVMRunFunction( exec_engine, vmprog_func, 0, NULL );

    LLVMDisposeExecutionEngine( exec_engine );
}

void ch8_ll_RunPasses()
{
	LLVMPassManagerRef passmgr = LLVMCreatePassManager( );
	
	LLVMRunPassManager( passmgr, module );

	LLVMDisposePassManager( passmgr );
}

void ch8_ll_AddOpcodeCall( const char* func_name, uint16_t param1, uint16_t param2, uint16_t param3 )
{	
    LLVMValueRef op_arg[3];
   	op_arg[0] = LLVMConstInt( LLVMInt16TypeInContext(context), param1, 0 );
   	op_arg[1] = LLVMConstInt( LLVMInt16TypeInContext(context), param2, 0 );
   	op_arg[2] = LLVMConstInt( LLVMInt16TypeInContext(context), param3, 0 );

   	LLVMValueRef call = LLVMBuildCall( builder, LLVMGetNamedFunction(module, func_name), op_arg, 3, "" );
   	LLVMSetFunctionCallConv( call, LLVMCCallConv );   	
}

void ch8_ll_EndCompilation()
{
	LLVMBuildRetVoid( builder );
	LLVMDisposeBuilder( builder );
}
