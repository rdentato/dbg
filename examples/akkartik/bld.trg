##  (C) by Remo Dentato (rdentato@gmail.com)
##
## This software is distributed under the terms of the MIT license:
##  https://opensource.org/licenses/MIT
##

_all () {
  bld foo
  bld bar
}

_default () {
  bld -d ../../tools -D DBGFLAGS="-DDEBUG=DBGLVL_INFO" dbgstat
  CCFLAGS="$CCFLAGS -DDEBUG=DBGLVL_TEST"
  bld_old $1$_EXE $1.c && {
    cc_exe $1$_EXE $1.o
  }
}


_clean () {
  bld_cmd $RM *.o
  bld_cmd $RM foo*$_EXE
  bld_cmd $RM bar*$_EXE
}

_cleanall () {
  _clean
  bld -d ../../ clean
}
