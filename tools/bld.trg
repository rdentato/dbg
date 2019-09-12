##  (C) by Remo Dentato (rdentato@gmail.com)
##
## This software is distributed under the terms of the MIT license:
##  https://opensource.org/licenses/MIT
##

_all () {
  bld dbgstat
}

_dbgstat () {
  if [ "$DBGFLAGS" = "" ] ; then DBGFLAGS="-DDEBUG=DBGLVL_TEST" ; fi
  chk_utl
  bld_old dbgstat$_EXE dbgstat.c ../utl/dbg.h && {
    cc_exe -f dbgstat$_EXE dbgstat.o
  }
}

_clean () {
  bld_cmd $RM *.o
  bld_cmd $RM dbgstat*$_EXE
  bld_cmd $RM *.log
}
