#!/bin/bash

if [ $# != 1 ]; then
	echo "usage: $0 path-to-pcaps" 1>&2
	exit 1
fi

p="$1"
if [ ! -d $p ]; then
	echo "error: bad path to pcap directory" 1>&2
	exit 1
fi

o="${p/test\//results/}"
[ ! -d "$o" ] && mkdir -p "$o"

OPTS=""
[ "$CW" != "" ] && OPTS+="--cw ${CW} "
[ "$RUNTIME" != "" ] && OPTS+="--runtime ${RUNTIME} "
[ "$ACKTIMEOUT" != "" ] && OPTS+="--acktimeout ${ACKTIMEOUT} "

for r in 6 9 12 18 24 36 48 54; do
	files="${p}/*load${r}*.pcap"
	for f in $files; do
		t="${f/test\//results/}"
		d="${t/28/38}"
		t="${t/.pcap/.data}"
		if [ "" != "$RUNTIME" ]; then
			 d="${d/.pcap/.dead}.${RUNTIME}"
			 if [ -s "$d" ]; then
				  x=`cat "$d"`
				  let x=x/$RUNTIME
				  [ "$x" != "\
" ] && x="--dead $x"
			 fi
		fi
		./elc --ticks --linkrate $r --input "$f" $x ${OPTS} | sed 's/nan/0/g' | awk -f "plot.awk" > "$t"
	done
done

exit 0
