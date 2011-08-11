#!/usr/bin/awk
START { f=0; }
/TIME:/{ t=$2; if(0 == f) { f = t; } }
/ELC:/{ print t - f, $3 * 8, $5 * 8, $7 * 8, $9 * 8, $11 * 8, $13, $15, $17; }
