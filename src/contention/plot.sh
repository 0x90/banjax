#!/bin/bash

# arbitrary 387,d deletion is to fix range at a reasonable level

./analyse -i test.pcap | awk '{ print $2; }' | sort -n | uniq -c | awk '{ print $2, $1; }'  | sed '387,$d' > plot.data

gnuplot plot.gp
