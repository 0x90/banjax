#!/usr/bin/awk

/TIME:/{ t=$2; }
/ELC:/{ print t, $3, $5; }