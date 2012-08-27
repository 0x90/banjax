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

if [ "$TA" == "" ]; then
	 echo No TA specified - assuming 00:0b:6b:0a:82:34! 2>&1
fi

OPTS=""
[ "$CW" != "" ] && OPTS+="--cw $CW "
[ "$RUNTIME" != "" ] && OPTS+="--runtime ${RUNTIME} "

for r in 6 9 12 18 24 36 48 54; do
	files="${p}/*load${r}*.pcap"
	for f in $files; do
		t="${f/test\//results/}"
		t="${t/.pcap/.dead}.${RUNTIME}"
		./beacons ${OPTS} --input "$f" > "$t"
	done
done

exit 0
