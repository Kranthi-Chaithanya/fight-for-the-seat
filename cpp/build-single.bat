@echo off
rem Builds one self-contained HTML file (code, WebAssembly and fonts inlined) that plays by double-clicking it.
rem Output: dist\Fight for the Seat.html
setlocal
set ROOT=%~dp0
if "%FFS_TOOLCHAIN%"=="" set FFS_TOOLCHAIN=%USERPROFILE%\ffs-toolchain
call "%FFS_TOOLCHAIN%\emsdk-main\emsdk_env.bat" >nul
set RAYLIB=%FFS_TOOLCHAIN%\raylib-5.5_webassembly
pushd "%ROOT%"
if not exist build\single mkdir build\single
if not exist dist mkdir dist
set SRC=src/Audio.cpp src/Campaign.cpp src/Citizen.cpp src/Common.cpp src/Data.cpp src/Election.cpp src/Events.cpp src/Game.cpp src/main.cpp src/Render.cpp src/Results.cpp src/SelfTest.cpp src/Street.cpp src/Ui.cpp
call em++ %SRC% -std=c++20 -O2 -I"%RAYLIB%\include" -L"%RAYLIB%\lib" -lraylib -sUSE_GLFW=3 -sALLOW_MEMORY_GROWTH=1 -sEXPORTED_RUNTIME_METHODS=HEAPF32 -DPLATFORM_WEB -sSINGLE_FILE=1 -sSINGLE_FILE_BINARY_ENCODE=0 --embed-file assets/fonts@/assets/fonts -o build/single/game.js
if errorlevel 1 goto fail
node make-single.js web\index.html build\single\game.js "dist\Fight for the Seat.html"
if errorlevel 1 goto fail
popd
echo Built dist\Fight for the Seat.html
exit /b 0
:fail
popd
echo Build failed.
exit /b 1
