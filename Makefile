.SUFFIXES: .bc .c .o .h

CC=clang
CFLAGS=-c `llvm-config --cflags` -Wall
LDFLAGS=`sdl-config --libs` `llvm-config --libs --cflags --ldflags all`
SOURCES=main.c ch8vm_llvm.c parser.c
DEPENDENCIES=parser.h ch8vm.h ch8vm_llvm.h
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=ch8vm

BC_LNK=llvm-link
BC_CFLAGS=-c `sdl-config --cflags`
BC_SOURCES=ch8vm.c ch8vm_sdl.c
BC_OBJECTS=$(BC_SOURCES:.c=.bc)
BC_TARGET=ch8vmlib.bc

all: $(SOURCES) $(EXECUTABLE) $(DEPENDENCIES) $(BC_SOURCES) $(BC_TARGET)

$(BC_TARGET): $(BC_OBJECTS)
	$(BC_LNK) -o $(BC_TARGET) $(BC_OBJECTS)

.c.bc:
	$(CC) -emit-llvm $(BC_CFLAGS) $< -o $@
	
$(EXECUTABLE): $(OBJECTS) $(DEPENDENCIES)
	$(CC)++ -O3 $(OBJECTS) $(LDFLAGS) -o $@ -v

.c.o: $(DEPENDENCIES)
	$(CC) -O3 $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) $(BC_OBJECTS) $(BC_TARGET)

