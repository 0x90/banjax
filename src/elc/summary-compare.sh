#!/bin/bash

case "$#" in
	 1)
		  p="$1"
		  m="ELC"
		  fn="rmse"
		  ;;
	 2)
		  p="$1"
		  m="$2"
		  fn="rmse"
		  ;;
	 3)
		  p="$1"
		  m="$2"
		  fn="$3"
		  ;;
	 *)
		  echo "usage: summary-compare path [metric] [fn]" 2>&1
		  exit 1
		  ;;
esac

if [ ! -d "$p" ]; then
	 echo "$p is not a directory!" 2>&1
	 exit 2
fi


echo "# Source: $p"
echo "# Function: $fn"
echo "# Metric: $m"
echo "#"

for r in 6 9 12 18 24 36 48 54; do
	files="${p}/*load${r}*.data"
	for f in $files; do
		a=`echo $f | sed 's/.*att//' | sed 's/_load.*//'`;
		echo -n "$r $a ";
		./extract.scm Goodput "${m}" < $f | awk '{ print $1 * 8, $2 * 8; }' | awk -f "${fn}.awk"
	done
done

exit 0
