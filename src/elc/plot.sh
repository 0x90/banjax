#!/bin/bash

h=`dirname $0`
for f in $*; do
	 $h/plot-some.sh "${f}" Goodput ELC Airtime-NS3
done

exit 0
