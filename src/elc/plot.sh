#!/bin/bash

D=`dirname $0`

i="$1"
o="${i/.pcap/.eps}"
./elc -i $1 | awk -f $D/plot.awk > plot.data
gnuplot $D/plot.gp
mv plot.eps "$o"
rm plot.data
exit 0