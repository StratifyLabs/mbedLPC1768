@echo off
set PROJ=%1
set PREFIX=%2
set HOST=%3
%PREFIX%\%HOST%-objcopy -j .text -j .data -O binary %PROJ%.elf %PROJ%.bin
%PREFIX%\%HOST%-objdump -j .text -j .data -j .bss -j .sysmem -d %PROJ%.elf > %PROJ%.s
%PREFIX%\%HOST%-size %PROJ%.elf
exit /b 1
