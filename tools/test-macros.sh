#!/usr/bin/env bash
# SPDX-License-Identifier: AFL-3.0

set -euo pipefail
DEBUG=0

main() {
  debug "main(): $*"
  local input=$1

  popen proc_pid proc_stdin proc_stdout "cc -E -DTEST_MACROS $input"
  fopen fd "$input" r
  dup expect_expect "$fd"
  dup expect_actual "$fd"
  dup actual_expect "$fd"
  actual_actual=$proc_stdout
  diff() {
    command diff -u \
      --label expected <(stream_zip <(tests "$expect_expect") <(tests_expect)) \
      --label actual <(stream_zip <(tests "$actual_expect") <(tests_actual)) \
      ;
  }
  tests() { stream_lines "$1" | stream_replace "^//\s*expect (.+)" 'E $1'; }
  tests_expect() { stream_lines "$expect_actual" | stream_replace "^//\s*expect [^:]*:\s*(.+)" 'A $1'; }
  tests_actual() { stream_lines "$actual_actual" | stream_replace "^#.*|^\s*$|(.+)" '$1' | stream_replace "(.+)" 'A $1'; }
  stream_lines() {
    local fd=$1

    while read -r -u "$fd" line; do
      echo "$line"
    done
  }
  stream_replace() {
    local re=$1
    local replacement=$2

    while read -r line; do
      if [[ $line =~ $re ]]; then
        set -- "${BASH_REMATCH[@]}"
        shift
        eval "echo \"$replacement\""
      fi
    done
  }
  stream_zip() {
    local a=$1
    local b=$2

    fopen a $a r
    fopen b $b r
    while read -r -u "$a" line; do
      echo "$line"
      read -r -u "$b" line
      echo "$line"
      echo
    done
  }
  diff
}

debug() {
  if [ $DEBUG -ne 0 ]; then
    echo "$@"
  fi
}

die() {
  echo "$@"
  return 1
}

dup() {
  debug "dup(): $*"
  local __ret=$1
  local fd=$2

  exec {fd2}<>"/dev/fd/$fd"

  eval "$__ret=$fd2"
}

fopen() {
  debug "fopen(): $*"
  local __ret=$1
  local pathname=$2
  local mode=$3

  case $mode in
  r)
    exec {fd}<"$pathname"
    ;;
  w)
    exec {fd}>"$pathname"
    ;;
  rw)
    exec {fd}<>"$pathname"
    ;;
  *)
    die "invalid mode: $mode"
    ;;
  esac

  eval "$__ret=$fd"
}

fclose() {
  debug "fclose(): $*"
  local fd=$1

  eval "exec $fd>&-"
}

mkfifo() {
  debug "mkfifo(): $*"
  local __ret_r=$1
  local __ret_w=$2

  tmp_dir=$(mktemp -d)
  tmp_file=$(mktemp -p "$tmp_dir" -u)
  command mkfifo "$tmp_file"

  fopen fd_dummy "$tmp_file" rw
  fopen fd_r "$tmp_file" r
  fopen fd_w "$tmp_file" w
  fclose $fd_dummy

  rm -r "$tmp_dir"

  eval "$__ret_r=$fd_r"
  eval "$__ret_w=$fd_w"
}

popen() {
  debug "popen(): $*"
  local __ret_pid=$1
  local __ret_fd_stdin=$2
  local __ret_fd_stdout=$3
  local cmd=$4
  shift 3

  mkfifo fd_in_r fd_in_w
  mkfifo fd_out_r fd_out_w
  {
    $cmd 0<&"$fd_in_r" 1>&"$fd_out_w" 2>&"$fd_out_w"
    ret=$?
    fclose "$fd_out_w"
  } &
  pid=$!
  fclose "$fd_in_r"
  fclose "$fd_out_w"

  eval "$__ret_pid=$pid"
  eval "$__ret_fd_stdin=$fd_in_w"
  eval "$__ret_fd_stdout=$fd_out_r"
}

main "$@"
