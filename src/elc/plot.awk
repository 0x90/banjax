#!/usr/bin/awk
START { f=0; }
/Time:/ { t=$2; if(0 == f) { f=t; } }
/^MAC: ff:ff:ff:ff:ff:ff/ { }
/^MAC: 0.:..:..:..:..:../ { print "Time: " t-f ", " $0; }
