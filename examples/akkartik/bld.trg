##  (C) by Remo Dentato (rdentato@gmail.com)
##
## This software is distributed under the terms of the MIT license:
##  https://opensource.org/licenses/MIT
##

_all () {
  bld foo
}

_foo () {
  bld -d ../../tools -D DBGFLAGS="-DDEBUG=DBGLVL_ERROR" dbgstat
  CCFLAGS="$CCFLAGS -DDEBUG=DBGLVL_TEST"
  bld_old foo$_EXE foo.c && {
    cc_exe foo$_EXE foo.o
  }
}

_foo_example () {
  echo  
}

_clean () {
  bld_cmd $RM *.o
  bld_cmd $RM foo*$_EXE
  bld_cmd $RM *.log
}

_cleanall () {
  _clean
  bld -d ../../ clean
}
