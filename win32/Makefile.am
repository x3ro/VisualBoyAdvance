EXTRA_DIST = \
	gba_sdl.dsp gba_sdl.dsw GBA.dsp GBA.dsw SDL-1.2.2 lib include

dist-hook:
	rm -rf `find $(distdir)/SDL-1.2.2 -name CVS`
	rm -rf `find $(distdir)/lib -name CVS`
	rm -rf `find $(distdir)/include -name CVS`
