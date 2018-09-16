#!/bin/sh
find src -name '*.c' | while read line;
    do echo "#include \"$line\"";
done > silis.inc
{
    echo '#define NULL 0'
    echo '#ifdef __cplusplus'
    echo '#define _Bool bool'
    echo '#endif'
    $CC -x c -E -P -I $PWD/src silis.inc | perl -pe 's(\Q((void *) 0))(NULL)'
} > silis.c
rm silis.inc
