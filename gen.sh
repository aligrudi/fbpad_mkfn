#!/bin/sh
# Generate fbpad fonts

FP="/path/to/fonts"
OP="-h25 -w15"
SZ="23h120v100"
./mkfn $OP $FP/UbuntuMono-R.ttf:$SZ	>ar.tf
./mkfn $OP $FP/UbuntuMono-RI.ttf:$SZ	>ai.tf
./mkfn $OP $FP/UbuntuMono-B.ttf:$SZ	>ab.tf
