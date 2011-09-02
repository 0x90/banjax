#!/usr/bin/gnuplot

set term postscript color enhanced eps
set out "plot.eps"

set ytics nomirror
set y2tics
set xlabel "Time (s)"
set ylabel "Data (MiB/S)"
set y2label "#"

plot "plot.data" using 1:2 with lines title "Goodput", \
	  "plot.data" using 1:4 with lines title "ELC", \
	  "plot.data" using 1:5 with lines title "ELC-MRR", \
	  "plot.data" using 1:6 with lines title "ELC (Legacy)", \
	  "plot.data" using 1:7 with lines title "ELC (Classic)", \
	  "plot.data" using 1:8 with lines title "RELC (Legacy)", \
	  "plot.data" using 1:9 with lines title "packets" axes x1y2, \
	  "plot.data" using 1:10 with lines title "frames" axes x1y2, \
	  "plot.data" using 1:11 with lines title "TXC" axes x1y2



