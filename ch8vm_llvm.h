#ifndef CH8VM_LLVM_H
#define CH8VM_LLVM_H

void ch8_ll_Init( const char* module_filename, const uint8_t *memory, uint16_t mem_size );
void ch8_ll_DumpOnDisk( const char* filename );
void ch8_ll_RunJIT( );

void ch8_ll_RunPasses();

void ch8_ll_AddOpcodeCall( const char* func_name, uint16_t param1, uint16_t param2, uint16_t param3 );

void ch8_ll_EndCompilation();

#endif // CH8VM_LLVM_H