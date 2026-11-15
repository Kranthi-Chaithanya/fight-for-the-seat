# Fight for the Seat

An Indian election campaign game written in C++20 with [raylib](https://www.raylib.com/) 5.5 and compiled to WebAssembly.
You have 14 days to win the fictional Chandanpur seat against three rival candidates.
All candidates, parties and places are fictional.

## Play

Download [`play/fight-for-the-seat.html`](play/fight-for-the-seat.html) and double-click it.
It is the whole game in one file and runs offline in any modern browser.
Click the game once so it gets the keyboard.

| Key | Action |
| --- | --- |
| A / D or arrows | Walk the street |
| E | Talk to the nearest citizen |
| F | Namaste (greet) |
| R | Start a rally where you stand |
| H | Help |
| Mouse | Everything else |

Each day you read the morning paper and pick actions on the constituency map.
Actions include rallies, roadshows, booth training, social media, press conferences, promises, endorsements, fundraising, surveys and rest.
You walk the streets talking to voters and handling hecklers, and you watch the public react in the online feed.
You must stay under the Election Commission spending limit.
The game ends with polling day (EVM and VVPAT) and a round-by-round count.

Difficulty: **Councillor** (easy), **MLA** (fair, tough), **Chanakya** (hard).

## Layout

| Path | What it is |
| --- | --- |
| `cpp/src/` | Game source: campaign model, election maths, citizens and dialogue, events, rendering, UI, procedural audio, polling and counting, balance self-test |
| `cpp/assets/fonts/` | Hind font by Indian Type Foundry (SIL Open Font License, see `OFL.txt`) |
| `cpp/web/` | Multi-file web build. Run `node cpp/serve.js` and open http://localhost:8766 |
| `play/` | One-file playable build |
| `fight-for-the-seat.html` | The first, simpler JavaScript version of the game |

## Build

The scripts expect a toolchain folder at `%USERPROFILE%\ffs-toolchain`, or wherever `FFS_TOOLCHAIN` points.
That folder holds w64devkit, raylib 5.5 (mingw and webassembly builds) and emsdk.

| Script | Builds |
| --- | --- |
| `cpp/build-web.bat` | `cpp/web/` (game.js, game.wasm, game.data) and the Node self-test |
| `cpp/build-single.bat` | `cpp/dist/Fight for the Seat.html`, the one-file build |
| `cpp/build.bat` | Native Windows executable (w64devkit g++) |
| `cpp/build-selftest.bat` | Headless balance test: `node cpp/build/selftest.js --selftest 300` |
