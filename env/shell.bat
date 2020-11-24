@echo off
set proj=iziwallet
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
set path=X:\%proj%\env;X:\lib;%path%
set DIRCMD=/o
set HOME=X:\%proj%\env\vimconfig
set XDG_CONFIG_HOME=X:\%proj%\env\vimconfig
