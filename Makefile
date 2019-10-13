#compiler
CC = gcc

#compiler flags
FLAGS = -g -Wall

#linker flags
LINKS = -lSDL2 -lSDL2main -lm

#input files
INPUT = texEdit.o graphics.o utility.o

#output file
OUTPUT = texEdit

#make instructions
all: $(INPUT)
	$(CC) $(INPUT) $(FLAGS) $(LINKS) -o $(OUTPUT)
	
texEdit.o: texEdit.c
	$(CC) texEdit.c $(FLAGS) -c
	
graphics.o: graphics.c
	$(CC) graphics.c $(FLAGS) $(LINKS) -c
	
utility.o: utility.c
	$(CC) utility.c $(FLAGS) -c
	
clean:
	rm -f $(INPUT)
	
cleanall:
	rm -f $(INPUT) $(OUTPUT)
