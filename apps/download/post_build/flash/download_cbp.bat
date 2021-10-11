
cd  %~dp0

@echo off

if "%1" == "pi32" (
    echo "download pi32" %1 %2
    set OBJDUMP=C:\JL\pi32\bin\llvm-objdump
    set OBJCOPY=C:\JL\pi32\bin\llvm-objcopy
    )

if "%1" =="pi32_lto" (
    echo "download pi32_lto" %1  %2
    set OBJDUMP=C:\JL\pi32\bin\llvm-objdump
    set OBJCOPY=C:\JL\pi32\bin\llvm-objcopy
    )

::cd tools\%2
::call download.bat %2 %OBJDUMP% %OBJCOPY% 

echo %EXE_NAME%
echo %OBJDUMP%
echo %OBJCOPY%

REM %OBJDUMP% -disassemble %2.exe > %2.lst
%OBJCOPY% -O binary -j .text  %2.exe  %2.bin
%OBJCOPY% -O binary -j .data  %2.exe  data.bin
%OBJCOPY% -O binary -j .nvdata %2.exe  nvdata.bin 
%OBJDUMP% -section-headers  %2.exe

copy %2.bin/b + data.bin/b + nvdata.bin/b sdk.app

call download.bat %2 %OBJDUMP% %OBJCOPY%

if exist %2.bin del %2.bin

if exist %2.lst del %2.lst

if exist data.bin del data.bin

if exist nvdata.bin  del nvdata.bin 

if exist *.bc  del *.bc 