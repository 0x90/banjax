#!/bin/bash

h=`dirname $0`
for f in $*; do
	 $h/plot-some.sh "${f}" Goodput Simple-ELC Airtime Airtime-Linux
done

exit 0
