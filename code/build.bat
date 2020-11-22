@echo off
REM TODO - can we just build both with one exe?

set CommonCompilerFlags=-diagnostics:column -MTd -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4456 -wd4505 -DWINDY_INTERNAL=1 -DWINDY_DEBUG=1 -FAsc -Z7
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib
REM set RendererExports=-EXPORT:win32_load_d3d11 -EXPORT:d3d11_reload_shader -EXPORT:d3d11_load_wexp
REM user32.lib gdi32.lib winmm.lib

IF NOT EXIST ..\build mkdir ..\build
IF NOT EXIST ..\rundata mkdir ..\rundata
IF NOT EXIST ..\rundata\assets mkdir ..\rundata\assets
pushd ..\build

REM 32-bit build
REM cl %CommonCompilerFlags% ..\code\win32_handmade.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

REM 64-bit build
del *.pdb > NUL 2> NUL
cl %CommonCompilerFlags% ..\code\windy.cpp -Fmwindy.map -LD /link -incremental:no /PDB:windy_%random%.pdb -EXPORT:WindyUpdateAndRender
cl %CommonCompilerFlags% ..\code\win32_layer.cpp -Fmwin32_windy.map /link %CommonLinkerFlags% d3d11.lib
popd

pushd ..\rundata\assets
set name=phong
fxc -nologo -Tvs_5_0 -DVERTEX_HLSL=1 -DPIXEL_HLSL=0 ..\..\code\%name%.hlsl -Fo%name%v.tmp
fxc -nologo -Tps_5_0 -DVERTEX_HLSL=0 -DPIXEL_HLSL=1 ..\..\code\%name%.hlsl -Fo%name%p.tmp
move /Y %name%v.tmp %name%.vsh
move /Y %name%p.tmp %name%.psh

set name=fonts
fxc -nologo -Tvs_5_0 -DVERTEX_HLSL=1 -DPIXEL_HLSL=0 ..\..\code\%name%.hlsl -Fo%name%v.tmp
fxc -nologo -Tps_5_0 -DVERTEX_HLSL=0 -DPIXEL_HLSL=1 ..\..\code\%name%.hlsl -Fo%name%p.tmp
move /Y %name%v.tmp %name%.vsh
move /Y %name%p.tmp %name%.psh

set name=debug
fxc -nologo -Tvs_5_0 -DVERTEX_HLSL=1 -DPIXEL_HLSL=0 ..\..\code\%name%.hlsl -Fo%name%v.tmp
fxc -nologo -Tps_5_0 -DVERTEX_HLSL=0 -DPIXEL_HLSL=1 ..\..\code\%name%.hlsl -Fo%name%p.tmp
move /Y %name%v.tmp %name%.vsh
move /Y %name%p.tmp %name%.psh
popd

