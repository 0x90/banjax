#!/bin/bash

D=`dirname $0`

i="$1"
if [ -f $i ]; then
   o="${i/.pcap/.eps}"
	d="${i/.pcap/.data}"
   ./elc -i $1 | sed 's/,//g' | awk -f $D/plot.awk > plot.data
   # gnuplot $D/plot.gp
   gnuplot $D/plot-against-txc.gp
   mv plot.eps "$o"
   mv plot.data "$d"
	exit 0
fi