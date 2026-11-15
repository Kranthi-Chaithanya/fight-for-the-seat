@echo off
rem Builds the browser version (web\game.js + game.wasm + game.data) with Emscripten and raylib's WebAssembly build,
rem plus a Node build of the balance self-test (build\selftest.js). Run "node build\selftest.js --selftest 200".
setlocal
set ROOT=%~dp0
if "%FFS_TOOLCHAIN%"=="" set FFS_TOOLCHAIN=%USERPROFILE%\ffs-toolchain
call "%FFS_TOOLCHAIN%\emsdk-main\emsdk_env.bat" >nul
set RAYLIB=%FFS_TOOLCHAIN%\raylib-5.5_webassembly
pushd "%ROOT%"
if not exist build mkdir build
set SRC=src/Audio.cpp src/Campaign.cpp src/Citizen.cpp src/Common.cpp src/Data.cpp src/Election.cpp src/Events.cpp src/Game.cpp src/main.cpp src/Render.cpp src/Results.cpp src/SelfTest.cpp src/Street.cpp src/Ui.cpp
set COMMON=-std=c++20 -O2 -I"%RAYLIB%\include" -L"%RAYLIB%\lib" -lraylib -sUSE_GLFW=3 -sALLOW_MEMORY_GROWTH=1 -sEXPORTED_RUNTIME_METHODS=HEAPF32 -DPLATFORM_WEB
call em++ %SRC% %COMMON% --preload-file assets/fonts@/assets/fonts -o web/game.js
if errorlevel 1 goto fail
call em++ %SRC% %COMMON% -sENVIRONMENT=node -o build/selftest.js
if errorlevel 1 goto fail
popd
echo Built web\game.js and build\selftest.js
exit /b 0
:fail
popd
echo Build failed.
exit /b 1
