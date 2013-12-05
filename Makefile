.SUFFIXES: .bc .c .o .h

LLVM_PATH=~/dev/llvm/llvm-3.2/bin/
#LLVM_PATH=

CC=$(LLVM_PATH)clang
CFLAGS=-c `$(LLVM_PATH)llvm-config --cflags` -Wall
LDFLAGS=`sdl-config --libs` `$(LLVM_PATH)llvm-config --libs --cflags --ldflags all`
SOURCES=main.c 
DEPENDENCIES=parser.h ch8vm.h
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=ch8vm

#BC_CC=$(CC)
BC_CC=~/dev/emscripten/emcc 
BC_LNK=$(LLVM_PATH)llvm-link
#BC_CFLAGS=-c `sdl-config --cflags`
BC_CFLAGS=-c `~/dev/emscripten/system/bin/sdl-config --cflags`
BC_SOURCES=parser.c ch8vm.c ch8vm_sdl.c
BC_OBJECTS=$(BC_SOURCES:.c=.bc)
BC_TARGET=ch8vmlib.bc

all: $(SOURCES) $(EXECUTABLE) $(DEPENDENCIES) $(BC_SOURCES) $(BC_TARGET)

$(BC_TARGET): $(BC_OBJECTS)
	$(BC_LNK) -o $(BC_TARGET) $(BC_OBJECTS)

.c.bc:
	$(BC_CC) -emit-llvm $(BC_CFLAGS) $< -o $@
	
$(EXECUTABLE): $(OBJECTS) $(DEPENDENCIES)
	$(CC)++ -O3 $(OBJECTS) $(LDFLAGS) -o $@ -v

.c.o: $(DEPENDENCIES)
	$(CC) -O3 $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) $(BC_OBJECTS) $(BC_TARGET)

