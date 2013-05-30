# Copyright 2011 NICTA
# 

bin_PROGRAMS = sync

sync_SOURCES = \
	sync.cpp 

sync_CPPFLAGS = -I@top_srcdir@/lib

sync_LDADD = @top_srcdir@/lib/libbanjax.la -lpcap -lboost_program_options
