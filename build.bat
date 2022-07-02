@echo off

set CompilerFlags= -FC -Zi -WX -W4 -nologo -EHa -wd4100 -wd4189 -wd4702 /std:c11
set LinkerFlags=

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

REM 64-bit build
cl %CompilerFlags% ..\code\XMCyber\win32_bigfile.cpp ^
                   ..\code\XMCyber\FileSort.cpp ^
                   ..\code\XMCyber\FileManager.cpp ^
                   ..\code\XMCyber\FileChunk.cpp ^
                   /link %LinkerFlags%

popd
