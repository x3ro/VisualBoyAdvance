CORE_SUBDIRS = gb

EXTRA_SUBDIRS = i386 prof sdl gtk

SUBDIRS = $(CORE_SUBDIRS) @VBA_EXTRA@

DIST_SUBDIRS = $(CORE_SUBDIRS) $(EXTRA_SUBDIRS)

EXTRA_DIST = \
	win32			\
	VisualBoyAdvance.cfg	\
	expr.l			\
	expr.y			\
	CheatSearch.cpp		\
	CheatSearch.h

dist-hook:
	rm -rf `find $(distdir)/win32 -name CVS`
