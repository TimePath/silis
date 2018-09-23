#!/bin/sh
set -euo pipefail

silis=$1
test=$2

echo "args=$@"
echo "PWD=$PWD"
echo "CC=$CC"
echo "silis=$silis"
echo "test=$test"

$silis $test $test.c

$CC $test.c -o $test.exe

$test.exe || ret=$?
ret=${ret:-0}

if [ $ret -ne 0 ]; then
    echo "Exit code: ${ret}"
    exit $ret
fi

