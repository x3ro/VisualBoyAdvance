SUBDIRS = gb @VBA_EXTRA@
EXTRA_SUBDIRS = gp i386
bin_PROGRAMS = VisualBoyAdvance TestEmu

VisualBoyAdvance_SOURCES = \
	SDL.cpp 2xSaI.cpp tvmode.cpp pixel.cpp motionblur.cpp admame.cpp \
	simple2x.cpp Font.cpp unzip.cpp \
	Cheats.cpp EEprom.cpp Flash.cpp GBA.cpp Gfx.cpp Globals.cpp \
	Mode0.cpp Mode1.cpp Mode2.cpp Mode3.cpp Mode4.cpp Mode5.cpp \
	Sound.cpp Sram.cpp Util.cpp armdis.cpp bios.cpp elf.cpp \
	expr.cpp expr-lex.cpp exprNode.cpp debugger.cpp remote.cpp \
	AutoBuild.h Cheats.h EEprom.h Flash.h Font.h GBA.h GBAinline.h \
	Gfx.h Globals.h NLS.h Port.h Sound.h Sram.h System.h Util.h armdis.h \
	bios.h debugger.h elf.h expr.cpp.h exprNode.h unzip.h

VisualBoyAdvance_LDADD = gb/libgb.a @VBA_GP@ @VBA_MMX@
VisualBoyAdvance_LDFLAGS = `sdl-config --libs`

TestEmu_SOURCES = \
	TestEmu.cpp 2xSaI.cpp tvmode.cpp pixel.cpp motionblur.cpp admame.cpp \
	simple2x.cpp Font.cpp unzip.cpp \
	Cheats.cpp EEprom.cpp Flash.cpp GBA.cpp Gfx.cpp Globals.cpp \
	Mode0.cpp Mode1.cpp Mode2.cpp Mode3.cpp Mode4.cpp Mode5.cpp \
	Sound.cpp Sram.cpp Util.cpp armdis.cpp bios.cpp elf.cpp \
	expr.cpp expr-lex.cpp exprNode.cpp debugger.cpp remote.cpp \
	AutoBuild.h Cheats.h EEprom.h Flash.h Font.h GBA.h GBAinline.h \
	Gfx.h Globals.h NLS.h Port.h Sound.h Sram.h System.h Util.h armdis.h \
	bios.h debugger.h elf.h expr.cpp.h exprNode.h unzip.h

TestEmu_LDADD = gb/libgb.a @VBA_GP@ @VBA_MMX@
TestEmu_LDFLAGS = `sdl-config --libs`

EXTRA_DIST = \
	gp i386 win32 VisualBoyAdvance.cfg \
	expr.l expr.y arm-new.cpp thumb.cpp getopt.c

dist-hook:
	rm -rf `find $(distdir)/gp -name CVS`
	rm -rf `find $(distdir)/i386 -name CVS`
	rm -rf `find $(distdir)/win32 -name CVS`

