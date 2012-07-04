#!/bin/bash

h=`dirname $0`
for f in $*; do
	 $h/plot-some.sh "${f}" Goodput Simple-ELC ELC Legacy Classic TXC
done

exit 0
