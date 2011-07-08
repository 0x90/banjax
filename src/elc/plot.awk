#!/usr/bin/awk
START { f=0; }
/TIME:/{ t=$2; if(0 == f) { f = t; } }
/ELC:/{ print t - f ", " $3, $5; }
# /ELC:/{ print t - f ", " $3, $5, $7; }
