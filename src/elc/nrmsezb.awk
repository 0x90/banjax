#!/bin/gawk

function max(x,y) {
	 return ((x > y) ? x : y);
}

BEGIN{ x=0; xmax=-1e400; n=0; }
{ t=(($1-$2)**2); x+=t; xmax=max(xmax,t); ++n; }
END{ if(0 == n) print "-"; else print (sqrt((1/n) * x) / xmax); }
