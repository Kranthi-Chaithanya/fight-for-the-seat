@echo off
rem Builds build\FightForTheSeat.exe with the portable w64devkit compiler and raylib 5.5.
rem The toolchain lives at a short path (%USERPROFILE%\ffs-toolchain) because GCC's headers
rem break Windows' 260-character path limit inside deep folders. Override with FFS_TOOLCHAIN.
setlocal
set ROOT=%~dp0
if "%FFS_TOOLCHAIN%"=="" set FFS_TOOLCHAIN=%USERPROFILE%\ffs-toolchain
set PATH=%FFS_TOOLCHAIN%\w64devkit\bin;%PATH%
set RAYLIB=%FFS_TOOLCHAIN%\raylib-5.5_win64_mingw-w64
if not exist "%ROOT%build" mkdir "%ROOT%build"
pushd "%ROOT%"
g++ -std=c++20 -O2 -Wall -Wextra -Wno-missing-field-initializers ^
  src/Audio.cpp src/Campaign.cpp src/Citizen.cpp src/Common.cpp src/Data.cpp src/Election.cpp ^
  src/Events.cpp src/Game.cpp src/main.cpp src/Render.cpp src/Results.cpp src/SelfTest.cpp src/Street.cpp src/Ui.cpp ^
  -I"%RAYLIB%\include" -L"%RAYLIB%\lib" ^
  -lraylib -lopengl32 -lgdi32 -lwinmm ^
  -static -static-libgcc -static-libstdc++ ^
  -o build/FightForTheSeat.exe %*
set ERR=%errorlevel%
popd
if not "%ERR%"=="0" (
  echo Build failed.
  exit /b 1
)
echo Built build\FightForTheSeat.exe
