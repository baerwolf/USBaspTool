ifeq ($(OS),Windows_NT)
	USBFLAGS =
	USBLIBS = -lusb
	EXE_SUFFIX = .exe
	CFLAGS += $(USBFLAGS)
	LDFLAGS+= $(USBLIBS)
else
	USBFLAGS = `libusb-config --cflags`
	USBLIBS = `libusb-config --libs` #-framework CoreFoundation
	EXE_SUFFIX =
	CFLAGS += $(USBFLAGS) -fPIC
	LDFLAGS+= $(USBLIBS)
endif
########################################################################################

CFLAGS+=-ggdb -g3 -O0 -DDEBUG=3 -DMYDEBUG

all: usbasploader$(EXE_SUFFIX)
depclean: deepclean
deepclean: clean
	$(RM) *~
clean:
	$(RM) *.o
	$(RM) usbasploader$(EXE_SUFFIX)

usbhelper.o: usbhelper.c usbasp.h usbhelper.h Makefile
	$(CC) -c -o usbhelper.o usbhelper.c $(CFLAGS)

usbasploader.o: usbasploader.c usbasp.h usbhelper.h Makefile
	$(CC) -c -o usbasploader.o usbasploader.c $(CFLAGS)

usbasploader$(EXE_SUFFIX): usbasploader.o usbhelper.o usbasp.h usbhelper.h Makefile
	$(CC) -o usbasploader$(EXE_SUFFIX) usbasploader.o usbhelper.o $(CFLAGS) $(LDFLAGS)
