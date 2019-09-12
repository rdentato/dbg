##  (C) by Remo Dentato (rdentato@gmail.com)
##
## This software is distributed under the terms of the MIT license:
##  https://opensource.org/licenses/MIT
##

_all () {
  bld -d utl clean
  bld -d test all
  bld -d tools all
}

_clean () {
  bld -d utl clean
  bld -d test clean
  bld -d tools clean
}
