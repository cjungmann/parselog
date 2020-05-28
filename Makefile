BASEFLAGS = -Wall -Werror -m64
LIB_CFLAGS = ${BASEFLAGS} -I. -fPIC -shared
LIB_DBFLAGS = ${LIB_CFLAGS} -ggdb
LOCAL_LINK = -Wl,-R -Wl,. -lpsyslog

BASEFLAGS = -Wall -Werror -ggdb
MODULES = fields.o

CC = gcc

all : libpsyslog.so scanner

#
#
# Library and constituent object files:
#
libpsyslog.so : psyslog.c psyslog.h line_reader.o
	$(CC) ${LIB_CFLAGS} -o libpsyslog.so line_reader.o psyslog.c

line_reader.o : line_reader.c line_reader.h
	$(CC) ${LIB_CFLAGS} -c -o line_reader.o line_reader.c


#
#
# Executable and constituent object files:
#
scanner : scanner.c fields.o libpsyslog.so
	$(CC) ${BASEFLAGS} -L. -o scanner $(MODULES) scanner.c ${LOCAL_LINK}  -lclargs

fields.o : fields.c fields.h
	$(CC) ${BASEFLAGS} -c -o fields.o fields.c

#
#
# Extras:
#
debug :
	$(CC) ${BASEFLAGS} -o scanner $(MODULES) scanner.c -lclargsd

clean :
	rm -f libpsyslog.so scanner
	rm -f psyslog psyslog.o
	rm -f fields fields.o
	rm -f line_reader line_reader.o

