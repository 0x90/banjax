#!/bin/awk

function min(x,y) { return(x < y ? x : y); }
function max(x,y) { return(x > y ? x : y); }

BEGIN{ init = 0; lo=0; hi=0; x=0; n=0; }
{ if(!init) { lo=$1; hi=$1; init = 1; }; lo=min(lo, $1); hi=max(hi, $1); x+=$1; n++; }
END{ print lo, hi, x/n; }
