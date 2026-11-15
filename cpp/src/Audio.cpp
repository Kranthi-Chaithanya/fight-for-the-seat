#include "Audio.hpp"
#include "Common.hpp"

namespace seat::sfx {

bool muted = false;
static Sound sounds[NSFX];
static bool ready = false;
constexpr int RATE = 22050;
constexpr float TAU = 6.2831853f;

static Sound make(const std::vector<float>& s) {
    std::vector<short> pcm(s.size());
    for (size_t i = 0; i < s.size(); ++i) pcm[i] = (short)(clampf(s[i], -1, 1) * 30000);
    Wave w{};
    w.frameCount = (unsigned)pcm.size();
    w.sampleRate = RATE;
    w.sampleSize = 16;
    w.channels = 1;
    w.data = pcm.data();
    return LoadSoundFromWave(w);  // copies the data
}

static std::vector<float> tone(float secs, float f, int wave, float attack, float vol) {
    int n = (int)(secs * RATE);
    std::vector<float> s(n);
    for (int i = 0; i < n; ++i) {
        float t = i / (float)RATE;
        float ph = std::fmod(t * f, 1.f);
        float v = wave == 0 ? std::sin(TAU * ph) : (ph < .5f ? 1.f : -1.f) * .6f;
        float env = std::min(1.f, t / attack) * std::min(1.f, (secs - t) / 0.03f);
        s[i] = v * env * vol;
    }
    return s;
}

static std::vector<float> crowd(float secs, bool happy) {
    int n = (int)(secs * RATE);
    std::vector<float> s(n);
    float lp = 0, lp2 = 0;
    std::vector<float> whoo;  // random rising shouts
    for (int i = 0; i < n; ++i) {
        float t = i / (float)RATE;
        float noise = frand(-1, 1);
        lp += (noise - lp) * (happy ? .25f : .08f);
        lp2 += (lp - lp2) * .5f;
        float env = std::min(1.f, t / .25f) * std::min(1.f, (secs - t) / .6f);
        float v = lp2 * 1.6f;
        if (happy) {
            for (int k = 0; k < 6; ++k) {
                float st = .1f + k * .22f, len = .35f;
                if (t > st && t < st + len) {
                    float u = (t - st) / len;
                    v += std::sin(TAU * (380 + k * 70 + u * 260) * t) * .12f * std::sin(3.14159f * u);
                }
            }
        } else {
            v += std::sin(TAU * (130 + 8 * std::sin(TAU * 5 * t)) * t) * .35f + std::sin(TAU * 196 * t) * .12f;
        }
        s[i] = v * env * .8f;
    }
    return s;
}

static std::vector<float> claps(float secs) {
    int n = (int)(secs * RATE);
    std::vector<float> s(n, 0.f);
    for (int k = 0; k < 70; ++k) {
        int start = irand(0, n - 2000);
        float amp = frand(.2f, .5f);
        for (int j = 0; j < 900 && start + j < n; ++j) s[start + j] += frand(-1, 1) * amp * std::exp(-j / 160.f);
    }
    for (int i = 0; i < n; ++i) s[i] *= std::min(1.f, (n - i) / (RATE * .4f));
    return s;
}

void init() {
    InitAudioDevice();
    if (!IsAudioDeviceReady()) return;
    auto click = tone(.035f, 1300, 0, .002f, .35f);
    auto beep = tone(.08f, 990, 1, .003f, .4f);
    auto evm = tone(1.1f, 1350, 0, .01f, .45f);
    auto h1 = tone(.16f, 430, 1, .005f, .5f), h2 = tone(.2f, 540, 1, .005f, .5f);
    h1.insert(h1.end(), 1500, 0.f);
    h1.insert(h1.end(), h2.begin(), h2.end());
    auto c1 = tone(.14f, 880, 0, .005f, .4f), c2 = tone(.25f, 1320, 0, .005f, .4f);
    c1.insert(c1.end(), c2.begin(), c2.end());
    sounds[CLICK] = make(click);
    sounds[BEEP] = make(beep);
    sounds[EVM] = make(evm);
    sounds[CHEER] = make(crowd(2.0f, true));
    sounds[BOO] = make(crowd(1.4f, false));
    sounds[CLAP] = make(claps(1.6f));
    sounds[HORN] = make(h1);
    sounds[CHIME] = make(c1);
    ready = true;
}

void unload() {
    if (!ready) return;
    for (auto& s : sounds) UnloadSound(s);
    CloseAudioDevice();
}

void play(Id id, float vol) {
    if (!ready || muted) return;
    SetSoundVolume(sounds[id], vol);
    PlaySound(sounds[id]);
}

}  // namespace seat::sfx
