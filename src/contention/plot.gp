#!/usr/bin/gnuplot

set term postscript color enhanced eps
set out "plot.eps"

set style fill solid
set style histogram
set style data histograms

set xlabel "delay (us)"
set ylabel "frequency"

plot "plot.data" using 1:2 with boxes title "frequency"
