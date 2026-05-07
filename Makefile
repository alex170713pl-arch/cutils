SRC = src/
CC ?= gcc
CFLAGS ?= -c -ansi -Os
MODE ?= STATIC
INCLUDE = include
PREF = /usr/local

OBJS = one_owner.o rtti.o shared_ptr.o str.o signal.o gc.o dict.o

ifeq ($(MODE),SHARED)
    CFLAGS += -fPIC
endif

all: libAurora.a
ifeq ($(MODE),SHARED)
all: libAurora.so
endif

libAurora.a: $(OBJS)
	ar rcs $@ $(OBJS)

libAurora.so: $(OBJS)
	$(CC) -shared $(OBJS) -o $@
signal.o : $(SRC)signal.c
	$(CC) $(CFLAGS) -o $@ $<
one_owner.o: $(SRC)one_owner.c
	$(CC) $(CFLAGS) -o $@ $<

rtti.o: $(SRC)rtti.c
	$(CC) $(CFLAGS) -o $@ $<

shared_ptr.o: $(SRC)shared_ptr.c
	$(CC) $(CFLAGS) -o $@ $<

str.o: $(SRC)str.c
	$(CC) $(CFLAGS) -o $@ $<
gc.o : $(SRC)gc.c
	$(CC) $(CFLAGS) -o $@ $<
	
dict.o : $(SRC)dict.c
	$(CC) $(CFLAGS) -o $@ $<

install: libAurora.so
	mkdir -p $(PREF)/lib
	mkdir -p $(PREF)/include/Aurora
	cp libAurora.so $(PREF)/lib/
	cp $(INCLUDE)/*.h $(PREF)/include/Aurora/

uninstall:
	rm -f $(PREF)/lib/libAurora.a
	rm -f $(PREF)/lib/libAurora.so
	rm -rf $(PREF)/include/Aurora
	
clean:
	rm -f $(OBJS) libAurora.a libAurora.so