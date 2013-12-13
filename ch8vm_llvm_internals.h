#ifndef CH8VM_LLVM_INTERNALS_H
#define CH8VM_LLVM_INTERNALS_H

typedef struct JumpCalls 
{
	LLVMBasicBlockRef 	block_from;
	uint16_t 			address_to;
	struct JumpCalls	*next;
} JumpCalls;

void ch8_ll_computeJumps();

#endif // CH8VM_LLVM_INTERNALS_H