#!/bin/awk

function min(x,y) {
	 return ((x < y) ? x : y);
}

function max(x,y) {
	 return ((x > y) ? x : y);
}

BEGIN{ x=0; xmin=1e400; xmax=-1e400; n=0; }
{ xmin=min(xmin, $1); xmax=max(xmax,$1); x+=$1; n++}
END {print x/n, xmin, xmax}

