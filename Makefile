CC=clang
CFLAGS=-c `sdl-config --cflags`
LDFLAGS=`sdl-config --libs`
SOURCES=main.c parser.c ch8vm.c ch8vm_sdl.c
DEPENDENCIES=parser.h ch8vm.h
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=ch8vm

all: $(SOURCES) $(EXECUTABLE) $(DEPENDENCIES)
	
$(EXECUTABLE): $(OBJECTS) $(DEPENDENCIES)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ -v

.cpp.o: $(DEPENDENCIES)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

