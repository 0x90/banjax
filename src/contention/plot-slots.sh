#!/bin/bash

for p in $*; do

	 o="${p/test/results}"
	 if [ ! -f "$p" ]; then
        echo "$p is not a pcap file!" 2>&1 
		  exit 1
	 fi
	 odir=`dirname "$o"`
	 [ ! -d "$odir" ] && mkdir -p "$odir"

	 c="${o/.pcap/.cw}"
	 d="${o/.pcap/.data}"
	 e="${o/.pcap/.slots.eps}"
	 ./analyse -i "$p" --ta $TA --runtime $RUNTIME 2> "$c" | awk '{ print int(($3 - 34)/9); }' | sort -n | uniq -c > "$d"

	 gnuplot <<EOF
#!/usr/bin/gnuplot

set term postscript color enhanced eps
set out "$e"

set key off
set style fill solid
set style histogram
set style data histograms

set xlabel "Slot"
set ylabel "Count"

set xrange [0:256]

plot "$d" using 2:1 with impulses

EOF

done
exit 0
