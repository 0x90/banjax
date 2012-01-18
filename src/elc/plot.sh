#!/bin/bash

h=`dirname $0`
for f in $*; do
	 $h/plot-some.sh "${f}" ELC "ELC(Legacy)" Goodput TXC
done

exit 0
