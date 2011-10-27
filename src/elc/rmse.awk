#!/bin/awk

BEGIN{ x=0; n=0; }
{ x+=(($5-$2)**2); ++n; }
END{ print "RMSE:", 8 * sqrt((1/n) * x), "Mb/s"; }
