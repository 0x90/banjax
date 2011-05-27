#!/usr/bin/gnuplot

set term postscript color enhanced eps
set out "ELC.eps"

set xlabel "Time (s)"
set ylabel "Link Capacity bits/us"

plot "ELC.data" using 1:2 with linespoints title "ELC", \
	  "ELC.data" using 1:3 with linespoints title "ELC-MRR"