MAJOR := 0
MINOR := 0
PATCH := 0

PREFIX := /usr/local
BINDIR := bin
LIBDIR := lib
INCDIR := include

LIBNAME := libopenaptx.so
SONAME := $(LIBNAME).$(MAJOR)
FILENAME := $(SONAME).$(MINOR).$(PATCH)

UTILITIES := openaptxenc openaptxdec

HEADERS := openaptx.h

BUILD := $(FILENAME) $(SONAME) $(LIBNAME) $(UTILITIES)

all: $(BUILD)

install: $(BUILD)
	mkdir -p $(DESTDIR)/$(PREFIX)/$(LIBDIR)
	cp -a $(FILENAME) $(SONAME) $(LIBNAME) $(DESTDIR)/$(PREFIX)/$(LIBDIR)
	mkdir -p $(DESTDIR)/$(PREFIX)/$(BINDIR)
	cp -a $(UTILITIES) $(DESTDIR)/$(PREFIX)/$(BINDIR)
	mkdir -p $(DESTDIR)/$(PREFIX)/$(INCDIR)
	cp -a $(HEADERS) $(DESTDIR)/$(PREFIX)/$(INCDIR)

$(FILENAME): openaptx.c $(HEADERS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -I. -shared -fPIC -Wl,-soname,$(SONAME) -o $@ $<

$(SONAME): $(FILENAME)
	ln -sf $< $@

$(LIBNAME): $(SONAME)
	ln -sf $< $@

%: %.c $(LIBNAME) $(HEADERS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -I. -o $@ $< $(LIBNAME)

clean:
	$(RM) $(BUILD)
