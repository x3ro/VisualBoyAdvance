bin_PROGRAMS = VisualBoyAdvance

noinst_PROGRAMS = TestEmu

VisualBoyAdvance_SOURCES = \
	SDL.c			\
	debugger.c		\
	debugger.h		\
	../2xSaI.c		\
	../AutoBuild.h		\
	../Cheats.c		\
	../Cheats.h		\
	../EEprom.c		\
	../EEprom.h		\
	../Flash.c		\
	../Flash.h		\
	../GBA.c		\
	../GBA.h		\
	../GBAinline.h		\
	../Gfx.c		\
	../Gfx.h		\
	../Globals.c		\
	../Globals.h		\
	../Mode0.c		\
	../Mode1.c		\
	../Mode2.c		\
	../Mode3.c		\
	../Mode4.c		\
	../Mode5.c		\
	../NLS.h		\
	../Port.h		\
	../RTC.c		\
	../RTC.h		\
	../Sound.c		\
	../Sound.h		\
	../Sram.c		\
	../Sram.h		\
	../System.h		\
	../Text.c		\
	../Text.h		\
	../Util.c		\
	../Util.h		\
	../admame.c		\
	../agbprint.c		\
	../agbprint.h		\
	../arm-new.h		\
	../armdis.c		\
	../armdis.h		\
	../bilinear.c		\
	../bios.c		\
	../bios.h		\
	../elf.c		\
	../elf.h		\
	../expr-lex.c		\
	../expr.c		\
	../expr.c.h		\
	../exprNode.c		\
	../exprNode.h		\
	../getopt.c		\
	../getopt.h		\
	../getopt1.c		\
	../hq2x.c		\
	../hq2x.h		\
	../interframe.c	\
	../interp.h		\
	../lq2x.h		\
	../memgzio.c		\
	../memgzio.h		\
	../motionblur.c	\
	../pixel.c		\
	../remote.c		\
	../scanline.c		\
	../simple2x.c		\
	../thumb.h		\
	../unzip.c		\
	../unzip.h

VisualBoyAdvance_LDADD = @VBA_LIBS@ @SDL_LIBS@

VisualBoyAdvance_DEPENDENCIES = @VBA_LIBS@

TestEmu_SOURCES = \
	TestEmu.c		\
	debugger.c		\
	debugger.h		\
	../2xSaI.c		\
	../AutoBuild.h		\
	../Cheats.c		\
	../Cheats.h		\
	../EEprom.c		\
	../EEprom.h		\
	../Flash.c		\
	../Flash.h		\
	../GBA.c		\
	../GBA.h		\
	../GBAinline.h		\
	../Gfx.c		\
	../Gfx.h		\
	../Globals.c		\
	../Globals.h		\
	../Mode0.c		\
	../Mode1.c		\
	../Mode2.c		\
	../Mode3.c		\
	../Mode4.c		\
	../Mode5.c		\
	../NLS.h		\
	../Port.h		\
	../RTC.c		\
	../RTC.h		\
	../Sound.c		\
	../Sound.h		\
	../Sram.c		\
	../Sram.h		\
	../System.h		\
	../Text.c		\
	../Text.h		\
	../Util.c		\
	../Util.h		\
	../admame.c		\
	../agbprint.c		\
	../agbprint.h		\
	../arm-new.h		\
	../armdis.c		\
	../armdis.h		\
	../bios.c		\
	../bios.h		\
	../elf.c		\
	../elf.h		\
	../expr-lex.c		\
	../expr.c		\
	../expr.c.h		\
	../exprNode.c		\
	../exprNode.h		\
	../hq2x.c		\
	../hq2x.h		\
	../interp.h		\
	../lq2x.h		\
	../memgzio.c		\
	../memgzio.h		\
	../motionblur.c	\
	../pixel.c		\
	../remote.c		\
	../scanline.c		\
	../simple2x.c		\
	../thumb.h		\
	../unzip.c		\
	../unzip.h

TestEmu_LDADD = @VBA_LIBS@ @SDL_LIBS@

TestEmu_DEPENDENCIES = @VBA_LIBS@

AM_CPPFLAGS = \
	-I$(top_srcdir)/src		\
	-DSDL				\
	-DSYSCONFDIR=\"$(sysconfdir)\"

AM_CXXFLAGS = -fno-exceptions @SDL_CFLAGS@

../expr.c: ../expr.y
	$(YACC) -o ../expr.c ../expr.y
../expr-lex.c: ../expr.l
	$(LEX) -o ../expr-lex.c ../expr.l

