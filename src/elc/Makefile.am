# Copyright 2011 NICTA
# 

bin_PROGRAMS = elc

elc_SOURCES = \
	abstract_metric.cpp \
	elc_metric.cpp \
	elc_mrr_metric.cpp \
	etx_metric.cpp \
	legacy_elc_metric.cpp \
	metric_demux.cpp \
	metric_group.cpp \
	metric.cpp \
	main.cpp \
	utilization_metric.cpp

elc_CPPFLAGS = -I@top_srcdir@/lib

elc_LDADD = @top_srcdir@/lib/libbanjax.la -lpcap -lboost_program_options
