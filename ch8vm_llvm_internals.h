#ifndef CH8VM_LLVM_INTERNALS_H
#define CH8VM_LLVM_INTERNALS_H

typedef struct JumpCalls 
{
	int type;

	LLVMBasicBlockRef 	block_from;
	uint16_t 			address_to;

	LLVMValueRef		value_if;
	uint16_t 			address_else;

	struct JumpCalls	*next;
} JumpCalls;

void ch8_ll_computeJumps();
void ch8_ll_AddCondJump( LLVMValueRef value_if );
void ch8_ll_AddJumpIntoBlock( uint16_t add_to );

LLVMBasicBlockRef ch8_ll_appendBlock( uint16_t address );

#endif // CH8VM_LLVM_INTERNALS_H