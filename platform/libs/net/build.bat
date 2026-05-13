@echo off

pushd "%~dp0"

@REM set "PROJ_PATH=%~dp0"
@REM cmake -S . -B build -G Ninja -DCMAKE_MAKE_PROGRAM="%PROJ_PATH%ninja.exe" -DCMAKE_TOOLCHAIN_FILE="%PROJ_PATH%riscv.cmake" -DCMAKE_BUILD_TYPE="Release"
cmake --preset riscv-release
cmake --build build

popd
