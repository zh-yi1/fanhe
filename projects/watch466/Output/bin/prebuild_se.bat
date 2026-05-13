@echo off
cd /d %~dp0
@echo on
if "%1"=="SE_VERSION_V2" (
    echo "copy SE_VERSION_V2 file"
    riscv32-elf-xmaker -b alipay_se_v2.xm || goto err
) else if "%1"=="SE_VERSION_V3" (
    echo "copy SE_VERSION_V3 file"
    riscv32-elf-xmaker -b alipay_se_v3.xm || goto err
) else (
    echo "paramters only can be SE_VERSION_V2 or SE_VERSION_V3"
    goto end
)
exit /b 0

:err
@echo off
if "%1"=="" pause
exit /b 1

