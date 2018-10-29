#!/bin/bash
for i in 15 16 18 19 31 32 33 40 50 62 64 66 68 80 115 122 128 130 190 250 256 300; do
#for i in 32 64; do
	echo $i;
	eval "./lilang ../../compilers-fall-2018/benchmarks/prime.lil --print-ir=a.ll -width=$i &> /dev/null && clang -O3 a.ll runtime.o &> /dev/null && /usr/bin/time -f "%e" ./a.out"
	echo "---------"
done
