# Copyright 2011 NICTA
# 

bin_PROGRAMS = elc

elc_SOURCES = \
	elc_metric.cpp \
	elc_mrr_metric.cpp \
	metric_demux.cpp \
	metric_group.cpp \
	metric.cpp \
	main.cpp

elc_CPPFLAGS = -I@top_srcdir@/lib

elc_LDADD = @top_srcdir@/lib/libbanjax.la -lpcap

