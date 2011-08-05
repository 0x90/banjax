#!/bin/bash

i="$1"
o="${i/.pcap/.eps}"
./elc -i $1 | awk -f plot.awk > plot.data
gnuplot plot.gp
mv plot.eps "$o"
rm plot.data
exit 0