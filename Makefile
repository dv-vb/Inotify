CC=$(across)-gcc
CFLAGS=-Wall -g -fprofile-arcs -ftest-coverage -lpthread 
LD=gcc
LDFLAGS= -fprofile-arcs -ftest-coverage 
DEPENDFLAG=
INCLUDES=./include 
LIBS= -L/usr/lib/

CFLAGS:=$(CFLAGS) -I $(INCLUDES)
LDFLAGS:=$(LIBS) 
LDFLAGS+=$(CFLAGS) 

SRCDIRS= lib checkcomment httptrans ftptrans listernsig .

PROGRAMS= test

SRCS=$(shell find . -name "*.c")
#SRCS=$(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))

OBJS=$(SRCS:.c=.o)

#DEPENDS=$(SRCS:.c=.d)
GCOV_OBJS:=$(SRCS:.c=.gcno)
GCOV_OBJS+=$(SRCS:.c=.gcda)

#all: $(PROGRAM)

#%.d:%.c
#        $(CC) $(DEPENDFLAG) $(CFLAGS)  $< |\
#        sed "s?\\(.*\\):?$(basename $<).o $(basename $<).d :?g" \
#        > $@ || $(RM) $@#libtool --mode=link
$(PROGRAMS): $(OBJS)
	
	$(CC) $(LDFLAGS) -o $@ $(filter %.o ,$+)


#include $(DEPENDS)


clean:
	rm $(OBJS)
	-rm $(PROGRAMS)
	-rm $(GCOV_OBJS)




