@echo off

set CompilerFlags= -FC -Zi -WX -W4 -nologo -EHa -wd4100 -wd4189 -wd4702 /std:c++14
set LinkerFlags=

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

cl %CompilerFlags% ..\code\XMCyber\main.cpp ^
                   ..\code\XMCyber\FileSort.cpp ^
                   ..\code\XMCyber\FileManager.cpp ^
                   ..\code\XMCyber\WordsArray.cpp ^
                   ..\code\XMCyber\Word.cpp ^
                   /link %LinkerFlags%

popd
