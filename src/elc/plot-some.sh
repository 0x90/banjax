#!/bin/bash

h=`dirname $0`

if [ $# -lt 2 ]; then
    echo "usage: plot-some.sh file.pcap [field*]"  2>&1
    exit 1
fi

p="$1"
shift
fields=$*

o="${p/test/results}"
odir=`dirname "$o"`
if [ ! -d "$odir" ]; then
    mkdir -p "$odir"
fi

d="${o/.pcap/.data}"
t="${o/.pcap/.extract}"
o="${o/.pcap/.eps}"

declare -A axis
axis["Octets"]="axes x1y2"
axis["Packets"]="axes x1y2"
axis["Frames"]="axes x1y2"
axis["TXC"]="axes x1y2"
axis["FDR"]="axes x1y2"

# write the extract file
[ "$BEACON" == "" ] && BEACON=0
if [ "$p" -nt "$d" ]; then
	 $h/elc ${CW} -b ${BEACON} -m ${MPDU} -l ${RATE} -i "$p" | sed 's/,//g' | sed 's/nan/0/g' | awk -f "${h}/plot.awk" > "$d"
fi
$h/extract.scm Time $fields < "$d" > "$t"

# prepare the plot string
if [ -s "$t" ]; then
	 s=""
    d=""
	 let n=2
	 for f in $*; do
		  if [ "${axis[$f]}" == "" ]; then
				s="${s}${d}\"${t}\" using 1:(Mb(\$${n})) with lp title \"${f}\""
		  else
				s="${s}${d}\"${t}\" using 1:${n} with lp title \"${f}\" ${axis[$f]}"
		  fi
		  d=", "
		  let n=n+1
    done
	 [ "$XRANGE" != "" ] && XRANGE="set xrange [$XRANGE]"
	 [ "$YRANGE" != "" ] && YRANGE="set yrange [$YRANGE]"
	 [ "$Y2RANGE" != "" ] && Y2RANGE="set y2range [$Y2RANGE]"
	 gnuplot <<EOF
set term postscript color enhanced eps
set out "$o"

# function to convert MB/s -> Mb/s
Mb(x)=x * 8

set key box outside
set grid xtics ytics
set ytics nomirror
set y2tics

set xlabel "Time (s)"
set ylabel "Traffic (Mb)"
set y2label "Count"

$XRANGE
$YRANGE
$Y2RANGE

plot $s 
EOF
	 # if [ -f "${t}" ]; then
	 # 	  rm "$t"
	 # fi

	 if [[ -f "${o}" && ! -s "${o}" ]]; then
		  rm "$o"
	 fi
else
	 echo "warning: extract failed for ${p}" 2>&1
	 exit 1
fi

exit 0
