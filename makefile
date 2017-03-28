CC      = cc
CPPFLAGS += -D_DEFAULT_SOURCE -D_BSD_SOURCE -DNDEBUG
#CPPFLAGS += -D_DEFAULT_SOURCE -D_BSD_SOURCE
#CFLAGS += -O2 -std=c11 -Wall -pedantic
CFLAGS += -O2 -std=c11 -Wall -pedantic -g
LIBS    = -lncurses
LD      = cc
CP      = cp
MV      = mv
RM      = rm

OBJ     = zepl.o lisp.o

zepl : $(OBJ)
	$(LD) -o zepl $(OBJ) $(LIBS)

zepl.o: zepl.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c zepl.c

lisp.o: lisp.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c lisp.c

clean:
	-$(RM) zepl *.o

install:
	-$(CP) zepl $(HOME)/bin/zepl
