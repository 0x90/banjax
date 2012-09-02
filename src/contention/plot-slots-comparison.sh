#!/bin/bash

if [ $# != 1 ]; then
		  echo "usage: $0 path-to-pcaps" 1>&2
		  exit 1
fi

p="$1"
if [ ! -d $p ]; then
	 echo "error: bad path to results directory" 1>&2
	 exit 1
fi

for r in 6 9 12 18 24 36 48 54; do
	 files="${p}/*load${r}*.data"
	 for f in $files; do
		  e="${f/.data/.eps}"
		  c="${f/.data/.distrib}"
		  c="${c/38/28}"
		  if [[ -s "$f" && -s "$c" ]]; then
				gnuplot <<EOF
#!/usr/bin/gnuplot

set term postscript color enhanced eps
set out "$e"

set key off
set style fill solid
set style histogram
set style data histograms

set xlabel "Slot"
set ylabel "Count"

# set xrange [0:256]

plot "$f" using 2:1 with impulses title "actual", \
     "$c" using 1:2 with impulses title "theoretic"
EOF
		  fi
	 done
done
exit 0
