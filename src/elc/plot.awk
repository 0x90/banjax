#!/usr/bin/awk
START { f=0; }
/TIME:/ { t=$2; if(0 == f) { f=t; } }
/^ff:ff:ff:ff:ff:ff/ { }
/^0.:..:..:..:..:../ { print t-f, $3, $5, $7, $9, $11, $13, $15, $17, $19, $21; }
