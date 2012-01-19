# Copyright 2011 NICTA
# 

bin_PROGRAMS = strip

strip_SOURCES = \
	strip.cpp 

strip_CPPFLAGS = -I@top_srcdir@/lib

strip_LDADD = @top_srcdir@/lib/libbanjax.la -lpcap -lboost_program_options
