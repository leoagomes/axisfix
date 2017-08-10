OBJECTS=ini.o configurator.o fix.o padreader.o fakeboard.o

EXEC=axisfix

INI_FLAGS=-DINI_STOP_PARSING_AFTER_ERROR=1

COMPILER_FLAGS=$(INI_FLAGS) -Iinclude

LINKER_FLAGS=

CC=gcc

all: $(EXEC)

debug: COMPILER_FLAGS += -g -DDEBUG
debug: clean
debug: all

$(EXEC): main.c $(OBJECTS)
	$(CC) -o $@ $^ $(COMPILER_FLAGS) $(LINKER_FLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(COMPILER_FLAGS)

clean:
	rm -rf $(OBJECTS) $(EXEC)

.PHONY: all debug clean
