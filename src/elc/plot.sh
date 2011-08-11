#!/bin/bash

D=`dirname $0`

i="$1"
if [ -f $i ]; then
   o="${i/.pcap/.eps}"
   ./elc -i $1 | sed 's/,//g' | awk -f $D/plot.awk > plot.data
   # gnuplot $D/plot.gp
   gnuplot $D/plot-against-txc.gp
   mv plot.eps "$o"
   rm plot.data
exit 0
fi