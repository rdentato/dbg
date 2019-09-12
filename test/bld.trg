##  (C) by Remo Dentato (rdentato@gmail.com)
##
## This software is distributed under the terms of the MIT license:
##  https://opensource.org/licenses/MIT
##

_all () {
  bld tests
}

setflags_ () {
  DBGFLAGS="-DDEBUG=DBGLVL_TEST"
}

_tests () {
  local f
  local t
  setflags_
  for f in $(ls ut_*.c); do
    t=${f:1}
    t=${t%.*}$_EXE
    echo "Building target: '${t%.*}'" >&2
    bld_old $t $f ../utl/dbg.h && {
      cc_exe -f $t ${f%.*}.o
    }
  done
}

_default () {
  local f
  local t
  f=u$1.c
  t=$1$_EXE
  setflags_
  echo "Building target: '${t%.*}'" >&2
  bld_old $t $f ../utl/dbg.h && {
    cc_exe -f $t ${f%.*}.o
  }
}

_clean () {
  bld_cmd $RM *.o
  bld_cmd $RM t_*$_EXE
  bld_cmd $RM *.log
}
