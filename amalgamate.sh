#!/bin/sh
{
    echo "#include \"src/system.c\""
    find src -name '*.c' | sort | while read line; do
        if [ $line = 'src/system.c' ]; then continue; fi
        if [ $line = 'src/test.c' ]; then continue; fi
        echo "#include \"$line\"";
    done
} > silis.inc
{
    echo '#define NULL 0'
    echo '#ifdef __cplusplus'
    echo '#define _Bool bool'
    echo '#endif'
    $CC -x c -E -P -I $PWD/src silis.inc | sed -e 's/((void \*)0)/NULL/g'
} > silis.c
rm silis.inc
