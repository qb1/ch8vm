#include <stdio.h>
#include <string.h>

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/BitReader.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Analysis.h>

#include "ch8vm_llvm.h"
#include "ch8vm_llvm_internals.h"

#include "ch8vm.h"

LLVMModuleRef module;
LLVMContextRef context;
LLVMBuilderRef builder;

LLVMValueRef vmprog_func;

JumpCalls	*jump_list_begin=NULL;
JumpCalls	*jump_list_end=NULL;

LLVMBasicBlockRef jump_addrs[CH8_MEMORY_SIZE];

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

    memset( jump_addrs, 0, sizeof(jump_addrs) );

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

   	ch8_ll_AddJump( 0x200 );
}

void ch8_ll_DumpOnStdout( )
{
	LLVMDumpModule( module );
}

void ch8_ll_WriteOnDisk( const char* filename )
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

void ch8_ll_AddOpcodeCall( const char* func_name, uint16_t param1, uint16_t param2, uint16_t param3, uint16_t address )
{	
    LLVMValueRef op_arg[3];
    char block_name[20];

    // Create its basic block
    sprintf( block_name, "addr_0x%.4X", address );
   	LLVMBasicBlockRef block = LLVMAppendBasicBlockInContext( context, vmprog_func, block_name );

   	jump_addrs[address] = block;

   	// Place the builder at the beginning of this new block
   	LLVMPositionBuilder (builder, block, LLVMGetFirstInstruction(block));   	

   	op_arg[0] = LLVMConstInt( LLVMInt16TypeInContext(context), param1, 0 );
   	op_arg[1] = LLVMConstInt( LLVMInt16TypeInContext(context), param2, 0 );
   	op_arg[2] = LLVMConstInt( LLVMInt16TypeInContext(context), param3, 0 );

   	LLVMValueRef call = LLVMBuildCall( builder, LLVMGetNamedFunction(module, func_name), op_arg, 3, "" );
   	LLVMSetFunctionCallConv( call, LLVMCCallConv );   	
}

void ch8_ll_AddJump( uint16_t address )
{
	LLVMValueRef op_arg[3];
    char block_name[20];

    JumpCalls *jump = malloc( sizeof(JumpCalls) );

    if( jump_list_begin == NULL )
    {
    	jump_list_begin = jump;
    }
    if( jump_list_end == NULL )
    {
    	jump_list_end = jump;
    }else{
    	jump_list_end->next = jump;    	
    }
    
    jump->block_from = LLVMGetInsertBlock( builder );
    jump->address_to = address;
    jump->next = NULL;
}

void ch8_ll_EndCompilation()
{
	ch8_ll_computeJumps();

	LLVMDisposeBuilder( builder );
}

void ch8_ll_computeJumps()
{
	JumpCalls *table_it;

	// Compute specifed jumps
	for( table_it = jump_list_begin; table_it != NULL; table_it = table_it->next )
	{
		if( jump_addrs[table_it->address_to] == NULL )
		{
			printf( "Error: jump address 0x%X doesn't exist.\n", table_it->address_to );
			continue;
		}

		 LLVMPositionBuilderAtEnd( builder, table_it->block_from );

		LLVMBuildBr( builder, jump_addrs[table_it->address_to] );
	}

	// Make all blocks correct
	LLVMBasicBlockRef block_it;

	for( block_it = LLVMGetFirstBasicBlock(vmprog_func); block_it != NULL; 
		 block_it =  LLVMGetNextBasicBlock(block_it) )
	{
		if( LLVMGetBasicBlockTerminator( block_it ) == NULL )
		{
			// No terminator specified: add a jump to next block
			LLVMBasicBlockRef next =  LLVMGetNextBasicBlock(block_it);

			if( next )
			{
				LLVMPositionBuilderAtEnd( builder, block_it );
				LLVMBuildBr( builder, next );
			}else{
				// Last block
				LLVMBuildRetVoid( builder );
			}
		}
	}
}