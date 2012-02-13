#!/bin/awk

function abs(value) { return(value < 0 ? -value : value); }

BEGIN{ x=0; y=0; z=0; n=0; }
{ x+=abs($1-$2); y+=$1; z+=(($1-$2)**2); ++n; }
END{ print "RMSE:", sqrt((1/n) * z), "VARIATION:", 100 * (x / y), "%"; }
