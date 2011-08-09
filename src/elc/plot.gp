#!/usr/bin/gnuplot

set term postscript color enhanced eps
set out "plot.eps"

set xlabel "Time (s)"
set ylabel "Data (MiB)"

plot "plot.data" using 1:2 with linespoints title "Goodput", \
	  "plot.data" using 1:3 with linespoints title "ELC", \
	  "plot.data" using 1:4 with linespoints title "ELC-MRR", \
	  "plot.data" using 1:5 with linespoints title "ELC (Legacy)"