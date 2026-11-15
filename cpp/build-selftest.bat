@echo off
rem Quick rebuild of just the Node self-test (build\selftest.js)
setlocal
if "%FFS_TOOLCHAIN%"=="" set FFS_TOOLCHAIN=%USERPROFILE%\ffs-toolchain
call "%FFS_TOOLCHAIN%\emsdk-main\emsdk_env.bat" >nul
set RAYLIB=%FFS_TOOLCHAIN%\raylib-5.5_webassembly
pushd "%~dp0"
call em++ src/Audio.cpp src/Campaign.cpp src/Citizen.cpp src/Common.cpp src/Data.cpp src/Election.cpp src/Events.cpp src/Game.cpp src/main.cpp src/Render.cpp src/Results.cpp src/SelfTest.cpp src/Street.cpp src/Ui.cpp -std=c++20 -O2 -I"%RAYLIB%\include" -L"%RAYLIB%\lib" -lraylib -sUSE_GLFW=3 -sALLOW_MEMORY_GROWTH=1 -sEXPORTED_RUNTIME_METHODS=HEAPF32 -DPLATFORM_WEB -sENVIRONMENT=node -o build/selftest.js
popd
