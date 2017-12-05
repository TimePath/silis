#!/bin/sh
find src -name '*.c' | while read line;
    do echo "#include \"$line\"";
done > silis.inc
$CC -x c -E -P silis.inc > silis.c
rm silis.inc

echo '#define _Bool bool' > silis.cpp
echo '#define NULL 0' >> silis.cpp
perl -pe 's(\Q((void *) 0))(NULL)' silis.c >> silis.cpp
