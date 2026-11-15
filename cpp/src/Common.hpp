// Fight for the Seat - shared types and helpers
#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include <deque>
#include <array>
#include <random>
#include <functional>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdarg>

namespace seat {

enum Cand { YOU = 0, PD = 1, LKM = 2, IND = 3, NCAND = 4, NOTA = 4 };
enum RegionId { UNI = 0, TOWN = 1, OLD = 2, MILL = 3, GAON = 4, NREGION = 5 };
enum Issue { JOBS, PRICES, WATER, ROADS, FARM, EDUCATION, SAFETY, CORRUPTION, NISSUE };
enum Bloc { FARMERS, TRADERS, YOUTH, WORKERS, WOMEN, MIDDLE, NBLOC };
enum Tone { T_INFO, T_GOOD, T_BAD, T_RIVAL };

struct Rng {
    std::mt19937 g{std::random_device{}()};
};
Rng& rng();
float frand(float a, float b);
int irand(int a, int b);
bool chance(float p);
float gauss();
template <class T> const T& pick(const std::vector<T>& v) { return v[irand(0, (int)v.size() - 1)]; }

std::string strf(const char* fmt, ...);
std::string fmtIN(long long n);      // Indian digit grouping: 1,72,000
std::string fmtL(float lakh);        // "₹12.5L"
std::string clockStr(float minute);  // "4:30 PM"

inline float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
inline float lerpf(float a, float b, float t) { return a + (b - a) * t; }
// Stable pseudo-random value in [0,1) for procedural scenery.
inline float hash01(int i, int salt) {
    unsigned int x = (unsigned int)(i * 374761393 + salt * 668265263);
    x = (x ^ (x >> 13)) * 1274126177u;
    x ^= x >> 16;
    return (x & 0xFFFFFF) / float(0x1000000);
}

// UTF-8 snippets used in UI text
#define RUPEE "\xE2\x82\xB9"
#define DOT "\xC2\xB7"
#define DASH "\xE2\x80\x94"

namespace Pal {
inline const Color bg{20, 16, 28, 255};
inline const Color panel{28, 23, 40, 235};
inline const Color panel2{42, 35, 58, 255};
inline const Color line{70, 60, 92, 255};
inline const Color text{238, 233, 245, 255};
inline const Color muted{170, 160, 188, 255};
inline const Color accent{167, 131, 242, 255};
inline const Color accentDeep{96, 52, 178, 255};
inline const Color marigold{240, 168, 40, 255};
inline const Color good{91, 196, 140, 255};
inline const Color bad{242, 113, 102, 255};
inline const Color paper{244, 240, 230, 255};
inline const Color ink{30, 24, 38, 255};
}  // namespace Pal

Color candColor(int c);  // bright UI colour
Color flagColor(int c);  // saturated in-world colour

}  // namespace seat
