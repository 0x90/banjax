#!/bin/bash

./analyse -i $1 | awk '{ if($3 > (52 + 9 * (2^($4 + 4)))) print $1, $2, $3, $4; }'

exit 0