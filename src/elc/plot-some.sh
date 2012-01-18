#!/bin/bash

h=`dirname $0`

if [ $# -lt 2 ]; then
	 echo "usage: plot-some.sh file.pcap [field*]"  2>&1
	 exit 1
fi

p="$1"
shift
fields=$*

d="${p/.pcap/.data}"
t="${p/.pcap/.extract}"
o="${p/.pcap/.eps}"

declare -A axis
axis["ELC"]=""
axis["ELC(Legacy)"]=""
axis["ELC(Classic)"]=""
axis["Goodput"]=""
axis["Octets"]="axes x1y2"
axis["Packets"]="axes x1y2"
axis["Frames"]="axes x1y2"
axis["TXC"]="axes x1y2"

# write the extract file
if [ "$p" -nt "$d" ]; then
	 $h/elc -m 1086 -i $p | sed 's/,//g' | sed 's/nan/0/g' | awk -f $h/plot.awk > $d
fi
$h/extract.scm Time $fields < "$d" > "$t"

# prepare the plot string
if [ -s "$t" ]; then
	 s=""
    d=""
	 let n=1
	 for f in $*; do
		  s="${s}${d}\"${t}\" using 1:(Mb(${n})) with lp title \"${f}\""
		  [ "${axis[$f]}" != "" ] && s="${s} ${axis[$f]}"
		  d=", "
		  let n=n+1
    done

	 [ "$Y1RANGE" != "" ] && Y1RANGE="set yrange [:$Y1RANGE]"
	 gnuplot <<EOF
set term postscript color enhanced eps
set out "$o"

# function to convert MB/s -> Mb/s
Mb(x)=x * 8

set grid xtics ytics
set ytics nomirror
set y2tics
set xlabel "Time (s)"
set ylabel "Traffic (Mb)"
$Y1RANGE
set y2label "Count"
set y2range [:12]

plot $s 
EOF
	 if [ -f "${t}" ]; then
		  rm "$t"
	 fi

	 if [[ -f "${o}" && ! -s "${o}" ]]; then
		  rm "$o"
	 fi
else
	 echo "warning: extract failed for ${p}" 2>&1
	 exit 1
fi

exit 0
