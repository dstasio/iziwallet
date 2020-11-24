@echo off
set CommonCompilerFlags=-diagnostics:column -utf-8 -MTd -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4456 -wd4505 -DWINDY_INTERNAL=1 -DWINDY_DEBUG=1 -FAsc -Z7
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib
REM set RendererExports=-EXPORT:win32_load_d3d11 -EXPORT:d3d11_reload_shader -EXPORT:d3d11_load_wexp
REM user32.lib gdi32.lib winmm.lib

IF NOT EXIST ..\build mkdir ..\build
IF NOT EXIST ..\rundata mkdir ..\rundata
pushd ..\build

REM 64-bit build
del *.pdb > NUL 2> NUL
REM cl %CommonCompilerFlags% /c ..\code\sqlite3.c
REM lib sqlite3.obj
cl %CommonCompilerFlags% ..\code\win32_layer.cpp -Fmwin32_windy.map -I..\imgui /link %CommonLinkerFlags% d3d11.lib sqlite3.lib

IF NOT EXIST sqlite3.dll copy ..\libs\sqlite3.dll .\
popd

