#!/bin/bash

f="$1"
o="${f/.data/.eps}"
p=""
d=""

if [ -f $d ]; then

	 shift
	 for arg in $*; do
		  case $arg in
				"good")
					 p="$p$d\"$f\" using 1:2 with lines title \"Goodput\""
					 d=", "
					 ;;
				"elc")
					 p="$p$d\"$f\" using 1:4 with lines title \"ELC\""
					 d=", "
					 ;;
				"mrr")
					 p="$p$d\"$f\" using 1:5 with lines title \"ELC-MRR\""
					 d=", "
					 ;;
				"old")
					 p="$p$d\"$f\" using 1:6 with lines title \"ELC-MRR\""
					 d=", "
					 ;;
				"pkt")
					 p="$p$d\"$f\" using 1:7 with lines title \"packets\" axes x1y2"
					 d=", "
					 ;;
				"frm")
					 p="$p$d\"$f\" using 1:8 with lines title \"frames\" axes x1y2"
					 d=", "
					 ;;
				"txc")
					 p="$p$d\"$f\" using 1:9 with lines title \"TXC\" axes x1y2"
					 d=", "
					 ;;
				*)
					 echo $arg
					 echo "What??"
					 exit 1
					 ;;
		  esac
	 done

	 gnuplot <<EOF
set term postscript color enhanced eps
set out "$o"

set ytics nomirror
set y2tics
set xlabel "Time (s)"
set ylabel "Data (MiB/s)"
set y2label "#"

plot $p 
EOF

fi
exit 0
