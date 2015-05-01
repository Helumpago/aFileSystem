
# Compiler info
CC=clang
CCOPTS = -c -g -Wall
LINKOPTS = -g

# Output files
EXE=out
OBJECTS=disk.o main.o fsys.o fat.o dir.o

all: $(EXE)
	
$(EXE): $(OBJECTS)
	$(CC) $(LINKOPTS) -o $@ $^

%.o: %.c
	$(CC) $(CCOPTS) -o $@ $^

run: all
	./$(EXE) virtdisk.txt