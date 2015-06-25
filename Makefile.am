AUTOMAKE_OPTIONS = foreign subdir-objects
MODNAME=mod_simpleamd

mod_LTLIBRARIES = mod_simpleamd.la
mod_simpleamd_la_SOURCES  = mod_simpleamd.c
mod_simpleamd_la_CFLAGS   = $(FREESWITCH_CFLAGS)
mod_simpleamd_la_CPPFLAGS = $(AM_CPPFLAGS)
mod_simpleamd_la_LIBADD   = -lsimpleamd
mod_simpleamd_la_LDFLAGS  = -avoid-version -module -no-undefined -shared $(FREESWITCH_LDFLAGS)

