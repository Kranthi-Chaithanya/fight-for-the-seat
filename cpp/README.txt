FIGHT FOR THE SEAT
An Indian election campaign game in C++ (raylib 5.5), compiled to WebAssembly.
All candidates, parties and places are fictional.

PLAY
  Double-click "Fight for the Seat.html". It runs in any modern browser, offline, no install.
  Click the game once so it gets the keyboard.

  A / D or arrows   walk the street        E   talk to the nearest citizen
  F                 namaste (greet)        R   start a rally where you stand
  H                 help                   Mouse   everything else

  You have 14 days to win the Chandanpur seat against three rivals.
  Each day: read the morning paper, pick actions on the map (travel, rallies,
  roadshows, booth training, social media, press, promises, endorsements, fundraising,
  surveys, rest), walk the streets talking to voters and handling hecklers, and watch the
  public react in the feed. Stay under the Election Commission spending limit.
  Then it's polling day (EVM + VVPAT) and a round-by-round count.

  Difficulty: Councillor (easy), MLA (fair, tough), Chanakya (hard).

WHAT'S IN THIS FOLDER
  Fight for the Seat.html     the whole game in one file
  Fight-for-the-Seat-source.zip
    cpp\src\                  C++20 source (campaign model, citizens, events, rendering, UI, audio, results)
    cpp\assets\fonts\         Hind font (SIL Open Font License, see OFL.txt)
    cpp\web\                  multi-file web build (serve with "node serve.js", open http://localhost:8766)
    cpp\build-web.bat         rebuild the web version (needs Emscripten + raylib webassembly)
    cpp\build-single.bat      rebuild the one-file HTML
    cpp\build.bat             native Windows build (w64devkit g++ + raylib mingw)
    cpp\build-selftest.bat    headless balance test: node build\selftest.js --selftest 300
    fight-for-the-seat.html   the first, simpler JavaScript version of the game

TOOLCHAIN
  Build scripts look for the toolchain in %USERPROFILE%\ffs-toolchain
  (or set FFS_TOOLCHAIN). It holds w64devkit, raylib 5.5 (mingw + webassembly) and emsdk 7.0.0.
  Note: Windows Smart App Control blocks the unsigned native .exe; the browser build is not affected.
