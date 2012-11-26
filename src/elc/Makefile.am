# Copyright 2011 NICTA
# 

bin_PROGRAMS = elc

elc_SOURCES = \
	abstract_metric.cpp \
	airtime_metric_actual.cpp \
	airtime_metric_linux.cpp \
	airtime_metric_ns3.cpp \
	elc_metric.cpp \
	elc_mrr_metric.cpp \
	etx_metric.cpp \
	fdr_metric.cpp \
	goodput_metric.cpp \
	iperf_metric_wrapper.cpp \
	legacy_elc_metric.cpp \
	metric_damper.cpp \
	metric_decimator.cpp \
	metric_demux.cpp \
	metric_group.cpp \
	metric.cpp \
	main.cpp \
	pdr_metric.cpp \
	pktsz_metric.cpp \
	residual.cpp \
	simple_elc_metric.cpp \
	txc_metric.cpp 

elc_CPPFLAGS = -I@top_srcdir@/lib

elc_LDADD = @top_srcdir@/lib/libbanjax.la -lpcap -lboost_program_options
