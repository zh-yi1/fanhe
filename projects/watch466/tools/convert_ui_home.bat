@echo off
cd /d %~dp0\..
echo [1/2] gen_mode_icons.py  (Tab / green temp: pasta, g0..g9, gh, gs...)
python tools\gen_mode_icons.py || exit /b 1
echo.
echo [2/2] gen_home_icons.py  (Home/Heat/Time: 0..9, 0m..9m, w0x, nav...)
python tools\gen_home_icons.py || exit /b 1
echo.
echo Done. ui/home/*.bin ready. Next run:
echo   Output\bin\prebuild.bat
echo Then rebuild and flash ui.bin + app.bin
exit /b 0
