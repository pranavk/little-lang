#!/bin/bash
if [[ -z "$1" ]]; then
	echo "Filename expected as first argument.";
	exit 1;
fi
DIR="$(cd "$(dirname "$0")" && pwd -P)"
# Below overrides the --print-ir flag if provided to the compiler
${DIR}/lilang $@ --print-ir=a.ll && \
llc a.ll -filetype=obj && \
echo "Creating executable a.out" && \
clang ${DIR}/runtime.o a.o && \
rm a.o a.ll
