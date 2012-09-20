#!/usr/bin/gawk

{ x += $1; v[NR] = $1; }
END { mean = x / NR; for(i = 1; i <= NR; i++) { sumsq += (v[i] - mean) ** 2; } print mean, sqrt(sumsq / NR)}
