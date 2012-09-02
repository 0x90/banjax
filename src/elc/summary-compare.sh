#!/bin/bash

case "$#" in
	 1)
		  p="$1"
		  sx="adj"
		  m="ELC"
		  fn="rmse"
		  ;;
	 2)
		  p="$1"
		  sx="$2"
		  m="adj"
		  fn="rmse"
		  ;;
	 4)
		  p="$1"
		  sx="$2"
		  m="$3"
		  fn="rmse"
		  ;;
	 5)
		  p="$1"
		  sx="$2"
		  m="$3"
		  fn="$4"
		  ;;
	 *)
		  echo "usage: summary-compare path [suffix] [metric] [fn]" 2>&1
		  exit 1
		  ;;
esac

if [ ! -d "$p" ]; then
	 echo "$p is not a directory!" 2>&1
	 exit 2
fi


echo "# Source: $p"
echo "# Generator: $0 $*"
echo "#"

for r in 6 9 12 18 24 36 48 54; do
	files="${p}/*load${r}*.${sx}"
	for f in $files; do
		a=`echo $f | sed 's/.*att//' | sed 's/_load.*//'`;
		echo -n "$r $a ";
		./extract.scm Goodput "${m}" < $f | awk -f Mb.awk | awk -f "${fn}.awk"
	done
done

exit 0
