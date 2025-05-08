CC      = gcc -pthread
CFLAGS  += -O2 -g -fPIC -std=gnu99
LDFLAGS += -shared
LIBS    += -lm -lpulse

CFLAGS += -fvisibility=hidden

# check missing symbols
LDFLAGS += -Wl,-z,defs

SRCS = \
	src/config.c \
	src/flashexports.c \
	src/globals.c \
	src/pulsethread.c \
	src/util.c \

OBJS    = $(SRCS:%.c=%.o)
OBJS_32 = $(SRCS:%.c=%_32.o)

default: libflashsupport.so libflashsupport_32.so

# do a full recompile if a header changes
# too lazy to do deps properly
$(OBJS) $(OBJS_32): src/*.h

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%_32.o: %.c
	$(CC) $(CFLAGS) -m32 -c $< -o $@

libflashsupport.so: $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS) $(LIBS)

libflashsupport_32.so: $(OBJS_32)
	$(CC) -m32 $^ -o $@ $(LDFLAGS) $(LIBS)

watch:
	ls src/*.[ch] | entr -cs 'make -s && size *.so'

clean:
	rm -f $(OBJS) $(OBJS_32) libflashsupport.so libflashsupport_32.so

# put a directory that's in the linker's search path here
# note, some bizarre systems (arch) don't load libraries from /usr/local by default
lib64_dir ?= /usr/local/lib/x86_64-linux-gnu
lib32_dir ?= /usr/local/lib/i686-linux-gnu

install:
	mkdir -p $(lib64_dir)
	cp libflashsupport.so $(lib64_dir)/libflashsupport.so.tmp
	mv $(lib64_dir)/libflashsupport.so.tmp $(lib64_dir)/libflashsupport.so
	mkdir -p $(lib32_dir)
	cp libflashsupport_32.so $(lib32_dir)/libflashsupport.so.tmp
	mv $(lib32_dir)/libflashsupport.so.tmp $(lib32_dir)/libflashsupport.so
	/sbin/ldconfig

install2:
	mkdir -p $(lib64_dir)
	ln -sf $$PWD/libflashsupport.so $(lib64_dir)/libflashsupport.so
	mkdir -p $(lib32_dir)
	ln -sf $$PWD/libflashsupport_32.so $(lib32_dir)/libflashsupport.so
	/sbin/ldconfig
