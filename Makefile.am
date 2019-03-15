#
# profiling: Line by line profiling and code coverage for GAP
#
# This file is part of the build system of a GAP kernel extension.
# Requires automake.
#
ACLOCAL_AMFLAGS = -I m4

BINARCHDIR = bin/$(GAPARCH)
GAPINSTALLLIB = $(abs_top_srcdir)/$(BINARCHDIR)/profiling.so

lib_LTLIBRARIES = profiling.la

profiling_la_SOURCES = src/profiling.cc src/md5.cc
profiling_la_CPPFLAGS = $(GAP_CPPFLAGS)
profiling_la_CFLAGS = $(GAP_CFLAGS)
profiling_la_LDFLAGS = $(GAP_LDFLAGS) -module -avoid-version
if SYS_IS_CYGWIN
profiling_la_LDFLAGS += -no-undefined -version-info 0:0:0 -Wl,$(GAPROOT)/bin/$(GAPARCH)/gap.dll
endif

all-local: profiling.la
	$(mkdir_p) $(top_srcdir)/$(BINARCHDIR)
if SYS_IS_CYGWIN
	cp .libs/profiling.dll $(GAPINSTALLLIB)
else
	cp .libs/profiling.so $(GAPINSTALLLIB)
endif
	@echo "SUCCESS!"

distclean-local:
	rm -rf bin/*
	(cd doc ; ./clean)

.PHONY: doc

doc:
	($(GAPROOT)/bin/gap.sh -A makedoc.g)
