bin_PROGRAMS = gVisualBoyAdvance

noinst_LIBRARIES = libgba.a

gVisualBoyAdvance_SOURCES = \
	filters.h		\
	input.cpp		\
	input.h			\
	intl.h			\
	main.cpp		\
	screenarea.cpp		\
	screenarea.h		\
	system.cpp		\
	tools.cpp		\
	tools.h			\
	window.cpp		\
	window.h

gVisualBoyAdvance_LDADD = libgba.a @VBA_LIBS@ @GTKMM_LIBS@ @LIBINTL@ @SDL_LIBS@

gVisualBoyAdvance_CPPFLAGS = \
	-DPKGDATADIR=\"$(pkgdatadir)\"			\
	-DLOCALEDIR=\"$(datadir)/locale\"		\
	-DGETTEXT_PACKAGE=\"$(GETTEXT_PACKAGE)\"

gVisualBoyAdvance_CXXFLAGS = @GTKMM_CFLAGS@ @SDL_CFLAGS@

libgba_a_SOURCES = \
	../2xSaI.cpp		\
	../AutoBuild.h		\
	../Cheats.cpp		\
	../Cheats.h		\
	../EEprom.cpp		\
	../EEprom.h		\
	../Flash.cpp		\
	../Flash.h		\
	../GBA.cpp		\
	../GBA.h		\
	../GBAinline.h		\
	../Gfx.cpp		\
	../Gfx.h		\
	../Globals.cpp		\
	../Globals.h		\
	../Mode0.cpp		\
	../Mode1.cpp		\
	../Mode2.cpp		\
	../Mode3.cpp		\
	../Mode4.cpp		\
	../Mode5.cpp		\
	../NLS.h		\
	../Port.h		\
	../RTC.cpp		\
	../RTC.h		\
	../Sound.cpp		\
	../Sound.h		\
	../Sram.cpp		\
	../Sram.h		\
	../System.h		\
	../Text.cpp		\
	../Text.h		\
	../Util.cpp		\
	../Util.h		\
	../admame.cpp		\
	../agbprint.cpp		\
	../agbprint.h		\
	../arm-new.h		\
	../armdis.cpp		\
	../armdis.h		\
	../bilinear.cpp		\
	../bios.cpp		\
	../bios.h		\
	../elf.cpp		\
	../elf.h		\
	../getopt.c		\
	../getopt.h		\
	../getopt1.c		\
	../hq2x.cpp		\
	../hq2x.h		\
	../interframe.cpp	\
	../interp.h		\
	../lq2x.h		\
	../memgzio.c		\
	../memgzio.h		\
	../motionblur.cpp	\
	../pixel.cpp		\
	../remote.cpp		\
	../scanline.cpp		\
	../simple2x.cpp		\
	../thumb.h		\
	../unzip.cpp		\
	../unzip.h

libgba_a_CPPFLAGS = -DSDL

libgba_a_CXXFLAGS = -fno-exceptions

dist_pkgdata_DATA = vba.glade
