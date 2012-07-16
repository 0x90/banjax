#!/bin/bash

for p in $*; do

	 o="${p/test/results}"
	 if [ -d "$p" ]; then
		  odir="$o"
	 else
		  odir=`dirname "$o"`
	 fi
	 [ ! -d "$odir" ] && mkdir -p "$odir"

	 c="${o/.pcap/.cw}"
	 d="${o/.pcap/.data}"
	 e="${o/.pcap/.slots.eps}"
	 ./analyse -i "$p" --ta $TA 2> "$c" | awk '{ print int(($3 - 34)/9); }' | sort -n | uniq -c > "$d"

	 gnuplot <<EOF
#!/usr/bin/gnuplot

set term postscript color enhanced eps
set out "$e"

set style fill solid
set style histogram
set style data histograms

set xlabel "slot"
set ylabel "frequency"

set xrange [:256]

plot "$d" using 2:1 with impulses title "frequency"

EOF

done
exit 0