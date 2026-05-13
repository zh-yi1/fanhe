@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"
echo [ASR_LOG] ASR_SELECT_GO.

@REM ----------------------------------------------------
@REM Direct call prebuild_asr.bat
@REM Param[0] ASR_NULL
@REM Param[1] ASR_WS
@REM Param[2] ASR_YJ
@REM set "ASR_SELECT=ASR_NULL"
@REM ----------------------------------------------------

@REM ----------------------------------------------------
@REM Parameter call prebuild_asr.bat
@REM set "ASR_SELECT=%~1"
@REM ----------------------------------------------------

@REM ----------------------------------------------------
@REM SET CONFIG.h
@REM ----------------------------------------------------
set "USE_DEV_TYPE="
set "CONFIG_FILE="
for /f "tokens=3 delims= " %%a in ('findstr /c:"#define USE_DEV_TYPE" "..\..\config.h"') do (
	echo [ASR_LOG] USE_DEV_TYPE IS:%%a
	set "USE_DEV_TYPE=%%a"
)

if "%USE_DEV_TYPE%" == "CONFIG_WATCH_DEV_V1_0" (
	set "CONFIG_FILE=config_watch_dev_v1_0.h"
) else if "%USE_DEV_TYPE%" == "CONFIG_CAMERA_DEV_V1_2" (
	set "CONFIG_FILE=config_camera_dev_v1_2.h"
) else if "%USE_DEV_TYPE%" == "CONFIG_IVI_DEV_V1_0" (
	set "CONFIG_FILE=config_ivi_dev_v1_0.h"
) else (
	echo [ASR_LOG] CONFIG_FILE ERR
)
echo [ASR_LOG] CONFIG_FILE IS:%CONFIG_FILE%

@REM ----------------------------------------------------
@REM Read config.h call prebuild_asr.bat
set "ASR_SELECT="
for /f "tokens=3 delims= " %%a in ('findstr /c:"#define ASR_SELECT" "..\..\!CONFIG_FILE!"') do (
	echo [ASR_LOG] ASR_SELECT IS:%%a
	set "ASR_SELECT=%%a"
)
@REM ----------------------------------------------------

set "modules=..\..\..\..\platform\modules"

if "%ASR_SELECT%" == "ASR_WS"  (
    @REM echo [ASR_LOG] ASR_SELECT IS ASR_WS
	call :asr_delete true
	xcopy "!modules!\ws\mp3" "res/asr" /e /i /c /y
	xcopy "!modules!\asr_using\voice.bin" "ui/asr" /e /i /c /y
	copy  "!modules!\ws\libasr.a" "!modules!\asr_using\libasr.a" /y /v
	for /f "tokens=3 delims= " %%a in ('findstr /c:"#define ASR_PREFETCH_EN" "..\..\!CONFIG_FILE!"') do (
		echo [ASR_LOG] ASR_PREFETCH_EN IS:%%a
		if %%a == 1 (
			xcopy "!modules!\ws\weight\weight.bin" "ui/asr" /e /i /c /y
		)
	)
	goto :asr_done
) else if "%ASR_SELECT%" == "ASR_YJ" (
    @REM echo [ASR_LOG] ASR_SELECT IS ASR_YJ
	call :asr_delete true
	xcopy "!modules!\yj\mp3" "res/asr" /e /i /c /y
	xcopy "!modules!\asr_using\voice.bin" "ui/asr" /e /i /c /y
	xcopy "!modules!\yj\weight\weights.bin" "ui/asr" /e /i /c /y
	copy  "!modules!\yj\libetasr.a" "!modules!\asr_using\libasr.a" /y /v
	goto :asr_done
) else if "%ASR_SELECT%" == "ASR_WS_AIR" (
	@REM echo [ASR_LOG] ASR_SELECT IS ASR_WS_AIR
	call :asr_delete true
	xcopy "!modules!\ws_air\mp3" "res/asr" /e /i /c /y
	xcopy "!modules!\asr_using\voice.bin" "ui/asr" /e /i /c /y
	copy  "!modules!\ws_air\libasr.a" "!modules!\asr_using\libasr.a" /y /v
	for /f "tokens=3 delims= " %%a in ('findstr /c:"#define IR_AIR_FUNC" "..\..\..\..\platform\header\config_extra.h"') do (
		echo [ASR_LOG] IR_AIR_FUNC IS:%%a
		if %%a == 1 (
			copy  "!modules!\ws_air\air\libwsir.a" "!modules!\asr_using\libasr_air.a" /y /v
		)
	)
	for /f "tokens=3 delims= " %%a in ('findstr /c:"#define ASR_AIR_PREFETCH_EN" "..\..\!CONFIG_FILE!"') do (
		echo [ASR_LOG] ASR_AIR_PREFETCH_EN IS:%%a
		if %%a == 1 (
			xcopy "!modules!\ws_air\weight\weight.bin" "ui/asr" /e /i /c /y
		)
	)
	goto :asr_done
) else if "%ASR_SELECT%" == "ASR_NULL" (
	@REM echo [ASR_LOG] ASR_SELECT IS ASR_NULL
	call :asr_delete true
	goto :asr_done
) else if "%ASR_SELECT%" == "" (
	echo [ASR_LOG] ASR_PARAM_NULL Please re-enter the parameters
	goto :asr_err
) else (
    echo [ASR_LOG] ASR_PARAM_ERROR Please re-enter the parameters
    goto :asr_err
)

:asr_delete
if "%~1" == "true" (
	set "del_paths[0]=res/asr"
	set "del_paths[1]=ui/asr"
	for /l %%i in (0,1,1) do (
		echo [ASR_LOG] del !del_paths[%%i]! file
		rd /s /q "!del_paths[%%i]!"
		md "!del_paths[%%i]!"
	)

	for /f "delims=" %%a in ('dir /b /a-d "!modules!\asr_using\*.a" 2^>nul') do (
		for %%f in ("!modules!\asr_using\%%a") do (
			if not %%~zf==0 (
				echo [ASR_LOG] clr  file: %%a
				type nul > "%%f"
			) else (
				echo [ASR_LOG] skip file: %%a
			)
		)
	)

	goto :eof
)

:asr_done
echo [ASR_LOG] ASR_SELECT_SUCCEED.
exit /b 0

:asr_err
echo [ASR_LOG] ASR_SELECT_ERROR.
exit /b 1
