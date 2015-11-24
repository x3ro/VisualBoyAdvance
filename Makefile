prefix=/usr/local
bindir=$(prefix)/bin
sysconfdir=$(prefix)/etc

PROGS = VisualBoyAdvance TestEmu
CFG = src/VisualBoyAdvance.cfg
GENSRCS=src/gen/expr.c src/gen/expr-lex.c
MAINSRCS=$(sort $(wildcard src/*.c)) $(GENSRCS) 
GBSRCS=$(sort $(wildcard src/gb/*.c))
SDLSRCS=src/sdl/debugger.c

VBASRCS=$(MAINSRCS) $(GBSRCS) $(SDLSRCS) src/sdl/SDL.c
VBAOBJS=$(VBASRCS:.c=.o)

TESTEMUSRCS=$(MAINSRCS) $(GBSRCS) $(SDLSRCS) src/sdl/TestEmu.c
TESTEMUOBJS=$(TESTEMUSRCS:.c=.o)

LEX=flex

-include config.mak

C99?=$(CC) -std=gnu99
CPPFLAGS+=-DSDL -DBKPT_SUPPORT -DSYSCONFDIR=\"$(sysconfdir)\"

all: $(PROGS)

install: $(PROGS:%=$(DESTDIR)$(bindir)/%) $(CFG:src/%=$(DESTDIR)$(sysconfdir)/%)

$(DESTDIR)$(bindir)/%: ./%
	install -D -m 755 $< $@

$(DESTDIR)$(sysconfdir)/%: src/%
	install -D -m 644 $< $@

VisualBoyAdvance: $(VBAOBJS)
	$(CC) $(LDFLAGS) -o $@ $(VBAOBJS) -lSDL -lpng -lz -lm

TestEmu: $(TESTEMUOBJS)
	$(CC) $(LDFLAGS) -o $@ $(TESTEMUOBJS) -lSDL -lpng -lz -lm

clean:
	rm -f $(GENSRCS)
	rm -f $(PROGS)
	rm -f $(VBAOBJS)

%.o: %.c
	$(C99) $(CPPFLAGS) $(CFLAGS) $(INC) -c -o $@ $<

src/gen/expr.c: src/gen/expr.y
	$(YACC) -o $@ $<

src/gen/expr-lex.c: src/gen/expr.l
	$(LEX) -o $@ $<


.PHONY: all clean install
