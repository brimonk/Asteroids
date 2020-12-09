@echo off

REM Common Settings
REM set CFLAGS=-Wall -g3

set IDIR=-I E:\dev\include
set LDIR=-L E:\dev\lib

set CFLAGS=-Wall -g3
set LINKER=-lSDL2 -lSDL2main
SET PFLAGS=-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE

REM Core Exe
SET SOURCES=src\main.c src\font.c src\io.c
clang %IDIR% %LDIR% -o bterm.exe %CFLAGS% %PFLAGS% %SOURCES% %LINKER%
SET SOURCES=

REM END

