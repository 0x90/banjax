#!/usr/bin/awk
START { f=0; }
/TIME:/ { t=$2; if(0 == f) { f=t; } }
/^ff:ff:ff:ff:ff:ff/ { }
/^0.:..:..:..:..:../ { print "TIME:", t-f, "MAC:", $0; }
