@echo off
cd /d %~dp0
@echo on
call prebuild_asr.bat
riscv32-elf-xmaker -b ui.xm || goto err
riscv32-elf-xmaker -b ui_external.xm || goto err
riscv32-elf-xmaker -b res.xm || goto err
riscv32-elf-xmaker -b xcfg.xm || goto err

if exist ui_external.bin move /y ui_external.bin app.xbf

exit /b 0

:err
@echo off
if "%1"=="" pause
exit /b 1
