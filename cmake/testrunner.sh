#!/bin/sh
bindir=$PWD
srcdir=$1
shift
name=$1
shift
dashdash=$1
shift

testdir="_tests"

suffix_expect=".txt"
suffix_actual=".actual.txt"

mkdir -p "${bindir}/${testdir}"
exec 3>&1
ret=$({ {
  "$@"
  echo $? >&4
} | tee "${bindir}/${testdir}/${name}${suffix_actual}" >&3; } 4>&1)
if [ "$ret" -ne 0 ]; then
  exit "$ret"
fi
if [ -f "${srcdir}/${testdir}/${name}${suffix_expect}" ]; then
  echo diff "${srcdir}/${testdir}/${name}${suffix_expect}" "${bindir}/${testdir}/${name}${suffix_actual}"
  diff "${srcdir}/${testdir}/${name}${suffix_expect}" "${bindir}/${testdir}/${name}${suffix_actual}"
else
  mkdir -p "${srcdir}/${testdir}"
  cp "${bindir}/${testdir}/${name}${suffix_actual}" "${srcdir}/${testdir}/${name}${suffix_expect}"
fi
