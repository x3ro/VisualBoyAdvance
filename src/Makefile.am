SUBDIRS = gb @VBA_EXTRA@
EXTRA_SUBDIRS = gp i386 prof
DIST_SUBDIRS = gb $(EXTRA_SUBDIRS)
bin_PROGRAMS = VisualBoyAdvance TestEmu

VisualBoyAdvance_SOURCES = \
	SDL.cpp 2xSaI.cpp pixel.cpp motionblur.cpp admame.cpp \
	simple2x.cpp bilinear.cpp interframe.cpp unzip.cpp \
	Cheats.cpp EEprom.cpp Flash.cpp GBA.cpp Gfx.cpp Globals.cpp \
	Mode0.cpp Mode1.cpp Mode2.cpp Mode3.cpp Mode4.cpp Mode5.cpp \
	Sound.cpp Sram.cpp Util.cpp armdis.cpp bios.cpp elf.cpp \
	expr.cpp expr-lex.cpp exprNode.cpp debugger.cpp remote.cpp \
	AutoBuild.h Cheats.h EEprom.h Flash.h GBA.h GBAinline.h \
	Gfx.h Globals.h NLS.h Port.h Sound.h Sram.h System.h Util.h armdis.h \
	bios.h debugger.h elf.h expr.cpp.h exprNode.h unzip.h arm-new.h \
	thumb.h RTC.cpp RTC.h agbprint.cpp agbprint.h scanline.cpp \
	getopt.c getopt1.c getopt.h memgzio.h memgzio.c \
	hq2x.cpp hq2x.h lq2x.h Text.cpp Text.h interp.h

VisualBoyAdvance_LDADD = gb/libgb.a @VBA_PROF@ @VBA_GP@ @VBA_MMX@
VisualBoyAdvance_LDFLAGS = `sdl-config --libs`

TestEmu_SOURCES = \
	TestEmu.cpp 2xSaI.cpp pixel.cpp motionblur.cpp admame.cpp \
	simple2x.cpp unzip.cpp \
	Cheats.cpp EEprom.cpp Flash.cpp GBA.cpp Gfx.cpp Globals.cpp \
	Mode0.cpp Mode1.cpp Mode2.cpp Mode3.cpp Mode4.cpp Mode5.cpp \
	Sound.cpp Sram.cpp Util.cpp armdis.cpp bios.cpp elf.cpp \
	expr.cpp expr-lex.cpp exprNode.cpp debugger.cpp remote.cpp \
	AutoBuild.h Cheats.h EEprom.h Flash.h GBA.h GBAinline.h \
	Gfx.h Globals.h NLS.h Port.h Sound.h Sram.h System.h Util.h armdis.h \
	bios.h debugger.h elf.h expr.cpp.h exprNode.h unzip.h arm-new.h \
	thumb.h RTC.cpp RTC.h agbprint.cpp agbprint.h scanline.cpp \
	memgzio.c memgzio.h hq2x.cpp hq2x.h lq2x.h Text.cpp Text.h \
	interp.h

TestEmu_LDADD = gb/libgb.a @VBA_PROF@ @VBA_GP@ @VBA_MMX@
TestEmu_LDFLAGS = 

EXTRA_DIST = \
	win32 VisualBoyAdvance.cfg \
	expr.l expr.y qt \
	CheatSearch.cpp CheatSearch.h

dist-hook:
	rm -rf `find $(distdir)/gp -name CVS`
	rm -rf `find $(distdir)/i386 -name CVS`
	rm -rf `find $(distdir)/win32 -name CVS`
	rm -rf `find $(distdir)/qt -name CVS`
	rm -rf `find $(distdir)/prof -name CVS`
