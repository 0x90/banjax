#!/bin/bash

h=`dirname $0`
for f in $*; do
	 $h/plot-some.sh "${f}" Goodput ELC Legacy Airtime Airtime-Linux Simple-ELC
done

exit 0
