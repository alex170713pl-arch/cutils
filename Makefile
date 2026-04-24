SRC = src/
CC ?= gcc
CFLAGS ?= -c -std=c89 -Os
MODE ?= STATIC
INCLUDE = include
PREF = /usr/local

OBJS = cutils-one_owner.o cutils-rtti.o cutils-shared_ptr.o cutils-str.o

ifeq ($(MODE),SHARED)
    CFLAGS += -fPIC
endif

all: libcutils.a
ifeq ($(MODE),SHARED)
all: libcutils.so
endif

libcutils.a: $(OBJS)
	ar rcs $@ $(OBJS)

libcutils.so: $(OBJS)
	$(CC) -shared $(OBJS) -o $@

cutils-one_owner.o: $(SRC)cutils-one_owner.c
	$(CC) $(CFLAGS) -o $@ $<

cutils-rtti.o: $(SRC)cutils-rtti.c
	$(CC) $(CFLAGS) -o $@ $<

cutils-shared_ptr.o: $(SRC)cutils-shared_ptr.c
	$(CC) $(CFLAGS) -o $@ $<

cutils-str.o: $(SRC)cutils-str.c
	$(CC) $(CFLAGS) -o $@ $<

install: libcutils.so
	mkdir -p $(PREF)/lib
	mkdir -p $(PREF)/include/cutils-lib
	cp libcutils.so $(PREF)/lib/
	cp $(INCLUDE)/*.h $(PREF)/include/cutils-lib/

uninstall:
	rm -f $(PREF)/lib/libcutils.a
	rm -f $(PREF)/lib/libcutils.so
	rm -rf $(PREF)/include/cutils-lib
	
clean:
	rm -f $(OBJS) libcutils.a libcutils.so