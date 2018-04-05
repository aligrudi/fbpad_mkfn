#!/bin/sh
# Generate fbpad fonts

FP="/path/to/fonts"
OP="-h32 -w17"
SZ="16.5h135v100r-3"
./mkfn_ft $OP $FP/UbuntuMono-R.ttf:$SZ	>ar.tf
./mkfn_ft $OP $FP/UbuntuMono-RI.ttf:$SZ	>ai.tf
./mkfn_ft $OP $FP/UbuntuMono-B.ttf:$SZ	>ab.tf
