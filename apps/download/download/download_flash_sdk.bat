cd  %~dp0

set EXE_NAME=sdk
set OBJDUMP=C:\JL\pi32\bin\llvm-objdump

set OBJCOPY=C:\JL\pi32\bin\llvm-objcopy

cd ../post_build/flash

REM echo %EXE_NAME%
REM echo %OBJDUMP%
REM echo %OBJCOPY%

REM %OBJDUMP% -disassemble %EXE_NAME%.exe > %EXE_NAME%.lst
%OBJCOPY% -O binary -j .text  %EXE_NAME%.exe  %EXE_NAME%.bin
%OBJCOPY% -O binary -j .data  %EXE_NAME%.exe  data.bin
%OBJCOPY% -O binary -j .nvdata %EXE_NAME%.exe  nvdata.bin 
%OBJDUMP% -section-headers  %EXE_NAME%.exe

copy %EXE_NAME%.bin/b + data.bin/b + nvdata.bin/b sdk.app

call download.bat %EXE_NAME% %OBJDUMP% %OBJCOPY% 

if exist %EXE_NAME%.bin del %EXE_NAME%.bin

if exist %EXE_NAME%.lst del %EXE_NAME%.lst

if exist data.bin del data.bin

if exist nvdata.bin  del nvdata.bin 

if exist *.bc  del *.bc 

pause
