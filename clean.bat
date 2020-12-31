@echo off

set /p NAME=<name.txt

del /F /Q %NAME%.exe *.exp *.lib *.ilk *.pdb

