CC      = cc
CPPFLAGS += -D_DEFAULT_SOURCE -D_BSD_SOURCE -DNDEBUG
#CPPFLAGS += -D_DEFAULT_SOURCE -D_BSD_SOURCE
CFLAGS += -O2 -std=c11 -Wall -pedantic
#CFLAGS += -O2 -std=c11 -Wall -pedantic -g
LIBS    = -lncurses
LD      = cc
CP      = cp
MV      = mv
RM      = rm

OBJ     = yabe.o lisp.o

yabe : $(OBJ)
	$(LD) -o yabe $(OBJ) $(LIBS)

yabe.o: yabe.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c yabe.c

lisp.o: lisp.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c lisp.c

clean:
	-$(RM) yabe *.o

install:
	-$(CP) yabe $(HOME)/bin/yabe
