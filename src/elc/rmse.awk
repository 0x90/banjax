#!/bin/awk

BEGIN{ x=0; n=0; }
{ x+=(($1-$2)**2); ++n; }
END{ print "RMSE:", sqrt((1/n) * x); }
