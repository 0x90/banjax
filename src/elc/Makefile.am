# Copyright 2011 NICTA
# 

bin_PROGRAMS = elc

elc_SOURCES = \
	elc_link_metric.cpp \
	elc_mrr_link_metric.cpp \
	group_metric.cpp \
	link_metric.cpp \
	main.cpp

elc_CPPFLAGS = -I@top_srcdir@/lib

elc_LDADD = @top_srcdir@/lib/libbanjax.la -lpcap

