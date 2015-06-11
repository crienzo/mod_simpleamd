include $(top_srcdir)/build/modmake.rulesam
MODNAME=mod_simpleamd

mod_LTLIBRARIES = mod_simpleamd.la
mod_simpleamd_la_SOURCES  = mod_simpleamd.c
mod_simpleamd_la_CFLAGS   = $(AM_CFLAGS)
mod_simpleamd_la_CPPFLAGS = $(AM_CPPFLAGS)
mod_simpleamd_la_LIBADD   = $(switch_builddir)/libfreeswitch.la -lsimpleamd
mod_simpleamd_la_LDFLAGS  = $(CURL_LIBS) -avoid-version -module -no-undefined -shared

