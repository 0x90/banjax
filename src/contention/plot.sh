#!/bin/bash

for p in $*; do

	c="${p/.pcap/.cw}"
	d="${p/.pcap/.data}"
	e="${p/.pcap/.eps}"
	./analyse -i "$p" 2> "$c" | awk '{ print $3; }' | sort -n | uniq -c > "$d"

	gnuplot <<EOF
#!/usr/bin/gnuplot

set term postscript color enhanced eps
set out "$e"

set style fill solid
set style histogram
set style data histograms

set xlabel "delay (us)"
set ylabel "frequency"

plot "$d" using 2:1 with boxes title "frequency"

EOF

done
