UTIL_SRC	= $(wildcard *.c)
UTIL_OBJ 	= $(patsubst %.c, %.o, $(UTIL_SRC))
UTIL_OBJ 	= $(patsubst %.c, %_32.o, $(UTIL_SRC))
UTIL_LIB	= $(PREFIX)/libutil.a

all: libutil libutil32

libutil: $(UTIL_OBJ)

libutil32: $(UTIL_OBJ)
	

%.o: %.c
	@ echo "  CC $<"
	@ $(CC) $< $(CFLAGS) -o $@

%_32.o: %.c
	@ echo "  CC $<"
	@ ARCH = i386-elf
	@ $(CC) $< $(CFLAGS) -o $@
