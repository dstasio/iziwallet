@echo off
set CommonCompilerFlags=-diagnostics:column -MT -EHa- -GL -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4456 -wd4505 -DWINDY_INTERNAL=1 -DWINDY_DEBUG=0
set CommonLinkerFlags=-incremental:no -opt:ref user32.lib

IF NOT EXIST ..\release mkdir ..\release
pushd ..\release

cl %CommonCompilerFlags% ..\code\win32_layer.cpp ..\code\sqlite3.c -I..\imgui /link %CommonLinkerFlags% d3d11.lib

popd

