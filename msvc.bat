::  (C) by Remo Dentato (rdentato@gmail.com)
::
:: Build with MS Visual C (cl)
::
@echo off

copy dbg.h dbgtest.c
cl /DDEBUG /DDBGTEST /Od /Fodbgtest dbgtest.c
del dbgtest.c
del dbgtest.obj
copy dbg.h dbgstat.c
cl /DDEBUG /DDBGSTAT /Od /Fodbgstat dbgstat.c
del dbgstat.c
del dbgstat.obj

dbgtest 2> dbgtest.log
dbgstat  < dbgtest.log
