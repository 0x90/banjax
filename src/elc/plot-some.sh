#!/bin/bash

f="$1"
b=`basename $f`
o="${b/.data/.eps}"
p=""
d=""

[ "$Y1RANGE" != "" ] && Y1RANGE="set yrange [:$Y1RANGE]"

if [ -f $d ]; then
	 shift
	 for arg in $*; do
		  case $arg in
				"good")
					 p="$p$d\"$f\" using 1:(Mb(\$2)) with lines title \"Goodput\""
					 d=", "
					 ;;
				"iperf")
					 p="$p$d\"$f\" using 1:(Mb(\$3)) with lines title \"iperf\""
					 d=", "
					 ;;
				"residual")
					 p="$p$d\"$f\" using 1:(Mb(\$4)) with lines title \"Residual(Goodput)\""
					 d=", "
					 ;;
				"elc")
					 p="$p$d\"$f\" using 1:(Mb(\$5)) with lines title \"Link Capacity\""
					 d=", "
					 ;;
				"mrr")
					 p="$p$d\"$f\" using 1:(Mb(\$6)) with lines title \"ELC-MRR\""
					 d=", "
					 ;;
				"old")
					 p="$p$d\"$f\" using 1:(Mb(\$7)) with lines title \"ELC-Legacy\""
					 d=", "
					 ;;
				"classic")
					 p="$p$d\"$f\" using 1:(Mb(\$8)) with lines title \"ELC (Classic)\""
					 d=", "
					 ;;
				"relc")
					 p="$p$d\"$f\" using 1:(Mb(\$9)) with lines title \"Residual (ELC-Legacy)\""
					 d=", "
					 ;;
				"packets")
					 p="$p$d\"$f\" using 1:10 with lines title \"packets\" axes x1y2"
					 d=", "
					 ;;
				"frames")
					 p="$p$d\"$f\" using 1:11 with lines title \"frames\" axes x1y2"
					 d=", "
					 ;;
				"txc")
					 p="$p$d\"$f\" using 1:12 with lines title \"TXC\" axes x1y2"
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

# function to convert MiB/s -> Mb/s
Mb(x)=(x * 8e6) / (1024 ** 2)

set ytics nomirror
set y2tics
set xlabel "Time (s)"
set ylabel "Data (Mb/s)"
$Y1RANGE
set y2label "Count"
set y2range [:12]

plot $p 
EOF

fi
exit 0
