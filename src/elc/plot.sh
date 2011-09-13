#!/bin/bash

p=`dirname $0`
i="$1"
if [ -f $i ]; then
   o="${i/.pcap/.eps}"
   d="${i/.pcap/.data}"
   $p/elc -m 1086 -i $1 | sed 's/,//g' | sed 's/nan/0/g' | awk -f $p/plot.awk > plot.data
   gnuplot $p/plot.gp
   mv plot.eps "$o"
   mv plot.data "$d"
   exit 0
fi
exit 1
