#!/bin/sh
# SPDX-License-Identifier: AFL-3.0

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
} | sed \
  -e "s|\\\\|/|g" \
  -e "s/,\b/, /g" \
  -e "s/\benum \b//g" \
  -e "s/\bstruct \b//g" \
  -e "s/\bbool\b/tier0::Boolean/g" \
  -e "s/\bunsigned char\b/tier0::Byte/g" \
  -e "s/\bshort\b/tier0::Short/g" \
  -e "s/\bint\b/tier0::Int/g" |
  tee "${bindir}/${testdir}/${name}${suffix_actual}" >&3; } 4>&1)
if [ "$ret" -ne 0 ]; then
  exit "$ret"
fi
if [ -f "${srcdir}/${testdir}/${name}${suffix_expect}" ]; then
  echo diff "${srcdir}/${testdir}/${name}${suffix_expect}" "${bindir}/${testdir}/${name}${suffix_actual}"
  diff --strip-trailing-cr "${srcdir}/${testdir}/${name}${suffix_expect}" "${bindir}/${testdir}/${name}${suffix_actual}"
else
  mkdir -p "${srcdir}/${testdir}"
  cp "${bindir}/${testdir}/${name}${suffix_actual}" "${srcdir}/${testdir}/${name}${suffix_expect}"
fi
