#!/bin/bash

p="$1"
d="${p/.pcap/.data}"
e="${p/.pcap/.eps}"
./analyse -i "$p" | awk '{ print $3; }' | sort -n | uniq -c | awk '{ print $2, $1; }' > "$d"

gnuplot <<EOF
#!/usr/bin/gnuplot

set term postscript color enhanced eps
set out "$e"

set style fill solid
set style histogram
set style data histograms

set xlabel "delay (us)"
set ylabel "frequency"

plot "$d" using 1:2 with boxes title "frequency"

EOF
