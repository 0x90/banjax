#!/bin/bash

h=`dirname $0`
for f in $*; do
	 $h/plot-some.sh "${f}" Goodput ELC TXC
done

exit 0
