#!/bin/bash

if [[ $# != 1 && ! -d $1 ]]; then
	 echo "usage: summary.sh path" 1>&2
	 exit 1
fi

t=$1
r=${t/test\//results\/}
[ ! -d "${r}" ] && mkdir -p "${r}"
s="${r}/summary.txt"

OPTS=""
[ "$BEACON" != "" ] && OPTS+="--beacon ${BEACON} "
[ "$CW" != "" ] && OPTS+="--cw $CW "
[ "$MPDU" != "" ] && OPTS+="--mpdu ${MPDU} "
[ "$RUNTIME" != "" ] && OPTS+="--runtime ${RUNTIME} "

echo > "$s"
for r in 6 9 12 18 24 36 48 54; do
	 for f in ${t}/*load${r}*; do
		  echo -n "File: $f, " >> "$s"
		  echo -n "$f" | sed 's/.*att\([0-9]*\)_load\([0-9]*\).*/Att: \1, Load: \2, /' >> "$s"
		  ./elc --linkrate $r $OPTS --input "$f" >> "$s"
	 done
done

exit 0
