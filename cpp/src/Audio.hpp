// Procedurally generated sound effects (no audio files needed).
#pragma once

namespace seat::sfx {

enum Id { CLICK, BEEP, EVM, CHEER, BOO, CLAP, HORN, CHIME, NSFX };
extern bool muted;
void init();
void unload();
void play(Id id, float volume = 1.f);

}  // namespace seat::sfx
