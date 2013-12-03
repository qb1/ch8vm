CC=clang
CFLAGS=-c `sdl-config --cflags` `llvm-config --cflags`
LDFLAGS=`sdl-config --libs` `llvm-config --libs --cflags --ldflags all`
SOURCES=main.c parser.c ch8vm.c ch8vm_sdl.c
DEPENDENCIES=parser.h ch8vm.h
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=ch8vm

all: $(SOURCES) $(EXECUTABLE) $(DEPENDENCIES)
	
$(EXECUTABLE): $(OBJECTS) $(DEPENDENCIES)
	$(CC)++ $(OBJECTS) $(LDFLAGS) -o $@ -v

.cpp.o: $(DEPENDENCIES)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

