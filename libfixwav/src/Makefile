

SOURCES:=$(wildcard /data/workspace/dvda-author/libfixwav/src/*.c)
OBJECTS:=$(subst /data/workspace/dvda-author/libfixwav/src,/data/workspace/dvda-author/libfixwav/src, $(SOURCES:.c=.o) )

CPPFLAGS += -I/data/workspace/dvda-author/libfixwav/src/include -I/data/workspace/dvda-author/libutils/src/include -I/data/workspace/dvda-author/src/include 
#for config.h
CPPFLAGS += -I/data/workspace/dvda-author


ifeq "" "yes"
	CPPFLAGS +=-I/data/workspace/dvda-author//include
	ifeq  "@HAVE_EXTERNAL_iberty@" "yes"
	  CPPFLAGS +=-I/data/workspace/dvda-author//include
	endif
endif

libfixwav.a: libfixwav.a($(OBJECTS))

#implicit rules won't work here
/data/workspace/dvda-author/libfixwav/src/%.o: /data/workspace/dvda-author/libfixwav/src/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
