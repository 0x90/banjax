#!/bin/awk

BEGIN{ x=0; n=0; }
{ x+=$1; ++n; }
END { if(0 < n) print x/n; else print "-"; }

