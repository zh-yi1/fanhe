@echo off
cd /d %~dp0
set proj_name=app
cd ..\..\

cd Output\bin\
@REM @echo on

set "found=0"
set "target_folder="

@REM echo: finding Downloader.exe...
for /r . /d %%d in (*) do (
    if exist "%%d\Downloader.exe" (
        set "target_folder=%%d"
        set "found=1"
        goto :find_end
    )
)

:find_end
if "%found%"=="1" (
    cd /d "%target_folder%"
    echo: enter target folder:
    echo: %target_folder%
    echo: current path: %cd%
) else (
    @REM echo: not find Downloader.exe!
    goto err
)

copy "..\ui.bin" ".\" /y
copy "..\app.dcf" ".\" /y
xcopy "..\Settings" ".\Settings\" /s /e /y /i /h /k
.\Downloader.exe -o .\app_ble_ota_code.fot -i .\app.dcf -s .\Settings\watch-ai-glass.setting -b
xcopy ".\app_ble_ota_code.fot" "..\" /e /i /c /y

if exist ".\app_ble_ota_all.fot" (
	DEL app_ble_ota_all.fot
)

for /f "delims=" %%i in ("ui.bin") do (
	echo: %%~zi>"ui_size.cfg"
)
for /f "delims=" %%i in ("app_ble_ota_code.fot") do (
	echo: %%~zi>"code_size.cfg"
)

if exist ".\app_ble_ota.cfg" (
	DEL app_ble_ota.cfg
)
@REM echo: DUF-UIAB>"app_ble_ota.cfg"
powershell -Command "[System.IO.File]::WriteAllText('app_ble_ota.cfg', 'DUF-UIAB')"

copy /b app_ble_ota.cfg + ui_size.cfg + code_size.cfg + *.bin + *.fot  app_ble_ota_all.fot
@REM ren ".\app_ble_ota_all.bin" ".\app_ble_ota_all.fot"
xcopy ".\app_ble_ota_all.fot" "..\" /e /i /c /y

DEL ui_size.cfg
DEL code_size.cfg
DEL app.dcf
DEL ui.bin
DEL app_ble_ota_all.fot
DEL app_ble_ota_code.fot
DEL app_ble_ota.cfg
rd /s /q "./Settings"
rd /s /q "../DebugDatas"
rd /s /q "../dlls"
rd /s /q "../en"
exit /b 0
:err
@echo off
@REM if "%1"=="" pause
@REM echo: ota generate error!!
exit /b 0
