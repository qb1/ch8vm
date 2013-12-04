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

	/* Read file contents into buffer */
	fseek(file_program, 0, SEEK_END);
	fileLen=ftell(file_program);
	fseek(file_program, 0, SEEK_SET);
	fread(state.M+0x200, fileLen, 1, file_program);
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
    // LLVMDumpModule( module ); 

   	// ArrayType* ArrayTy_5 = ArrayType::get(IntegerType::get(mod->getContext(), 8), 16);
   	// ArrayType* ArrayTy_4 = ArrayType::get(IntegerType::get(mod->getContext(), 8), 256);
   	// ArrayType* ArrayTy_6 = ArrayType::get(IntegerType::get(mod->getContext(), 16), 16);
   	// PointerType* PointerTy_7 = PointerType::get(IntegerType::get(mod->getContext(), 16), 0);
 	
   	// Add the global state's initialiser
   	LLVMContextRef context = LLVMGetModuleContext(module);
   	printf( "before\n" );
   	LLVMValueRef state_var = LLVMGetNamedGlobal( module, "global_state" );
   	LLVMTypeRef state_type = LLVMGetTypeByName( module, "struct.CH8_STATE" );

   	LLVMTypeRef state_type_int8x16 = LLVMArrayType( LLVMInt8TypeInContext(context), 16);
   	LLVMTypeRef state_type_int8x256 = LLVMArrayType( LLVMInt8TypeInContext(context), 256);
   	LLVMTypeRef state_type_int16x16 = LLVMArrayType( LLVMInt16TypeInContext(context), 16);
   	LLVMTypeRef state_type_int8x3 = LLVMArrayType( LLVMInt8TypeInContext(context), 3);
   	LLVMTypeRef state_type_p_int16 =  LLVMPointerType( LLVMInt16TypeInContext(context), 0);

   	LLVMValueRef state_const_fields[] = 
   	{
   		// LLVMConstStringInContext( context, (char*)state.M, sizeof(state.M), 1 ),
   		LLVMConstNull( LLVMArrayType( LLVMInt8TypeInContext(context), 0x10000) ),
   		LLVMConstNull( state_type_int8x256 ),

   		LLVMConstInt( LLVMInt16TypeInContext(context), 1, 0 ),
   		LLVMConstNull( state_type_int8x16 ),

   		LLVMConstInt( LLVMInt8TypeInContext(context), 0, 0 ),
   		LLVMConstInt( LLVMInt8TypeInContext(context), 0, 0 ),

   		LLVMConstInt( LLVMInt16TypeInContext(context), 0, 0 ),
   		LLVMConstNull( state_type_int16x16 ),
   		LLVMConstPointerNull( state_type_p_int16 ),
   		LLVMConstInt( LLVMInt8TypeInContext(context), 0, 0 ),

   		LLVMConstNull( state_type_int8x16 ),
   		LLVMGetUndef( state_type_int8x3 )
   	};
   	
	LLVMValueRef state_const = LLVMConstNamedStruct( state_type, state_const_fields, sizeof(state_const_fields)/sizeof(*state_const_fields) );
	LLVMSetInitializer( state_var, state_const );

	//LLVMDumpModule( module ); 

	LLVMPassManagerRef passmgr = LLVMCreatePassManager( );
	LLVMRunPassManager( passmgr, module );
	LLVMDisposePassManager( passmgr );

	LLVMWriteBitcodeToFile( module, "output.bc" );

   	// if(LLVMCreateExecutionEngineForModule(&exec_engine, module, &err) )
   	// {
    //     fprintf (stderr, "error: %s\n", err);
    //     LLVMDisposeMessage (err);
    //     return 1;
    // }

	// Write to file
	// if( LLVMVerifyModule( module, LLVMPrintMessageAction, NULL ) )
	// 	exit(1);

    

    return 0;

 //    // Call the VM
 //    const char * const main_func_args [] = {"ch8vm"};
 //    LLVMValueRef main_func = LLVMGetNamedFunction( module, "main");

	// LLVMRunFunctionAsMain( exec_engine, main_func, 1, main_func_args, NULL );

	// return 0;
}