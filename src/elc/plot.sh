#!/bin/bash

h=`dirname $0`
for f in $*; do
	 $h/plot-some.sh "${f}" Goodput ELC Legacy Classic Airtime TXC Simple-ELC
done

exit 0
