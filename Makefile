BASEFLAGS = -Wall -Werror -ggdb
MODULES = fields.o

all : scanner

scanner : scanner.c fields.o
	gcc ${BASEFLAGS} -o scanner $(MODULES) scanner.c -lclargs

fields.o : fields.c fields.h
	gcc ${BASEFLAGS} -c -o fields.o fields.c

debug :
	gcc ${BASEFLAGS} -o scanner $(MODULES) scanner.c -lclargsd

