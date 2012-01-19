#!/bin/bash

h=`dirname $0`

if [ $# -lt 2 ]; then
	 echo "usage: plot-create.sh file.pcap [field*]"  2>&1
	 exit 1
fi

p="$1"
shift
fields=$*

b=`basename ${p}`
t="${b/.pcap/.extract}"
o="${p/.pcap/.gp}"
e="${b/.pcap/.eps}"


declare -A axis
axis["Octets"]="axes x1y2"
axis["Packets"]="axes x1y2"
axis["Frames"]="axes x1y2"
axis["TXC"]="axes x1y2"

# prepare the plot string
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
cat > "${o}" <<EOF
set term postscript color enhanced eps
set out "$e"

# function to convert MB/s -> Mb/s
Mb(x)=x * 8

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

exit 0
