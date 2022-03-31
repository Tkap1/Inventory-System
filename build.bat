@echo off
cls

@REM You will have to do something like:
	@REM call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86_amd64
@REM before running this script

set compiler=-W4 -nologo
set linker=..\libs\raylib.lib user32.lib shell32.lib gdi32.lib Winmm.lib

pushd build
	cl ..\src\main.cpp %compiler% -link %linker%
popd