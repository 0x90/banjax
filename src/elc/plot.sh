#!/bin/bash

h=`dirname $0`
for f in $*; do
	 $h/plot-some.sh "${f}" Goodput Airtime-Measured Airtime-Kernel Airtime-NS3
done

exit 0
