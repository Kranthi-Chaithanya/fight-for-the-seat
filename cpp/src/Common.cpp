#include "Common.hpp"

namespace seat {

Rng& rng() {
    static Rng r;
    return r;
}
float frand(float a, float b) { return std::uniform_real_distribution<float>(a, b)(rng().g); }
int irand(int a, int b) { return std::uniform_int_distribution<int>(a, b)(rng().g); }
bool chance(float p) { return frand(0.f, 1.f) < p; }
float gauss() { return std::normal_distribution<float>(0.f, 1.f)(rng().g); }

std::string strf(const char* fmt, ...) {
    char buf[2048];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    return buf;
}

std::string fmtIN(long long n) {
    bool neg = n < 0;
    if (neg) n = -n;
    std::string s = std::to_string(n);
    if (s.size() > 3) {
        std::string head = s.substr(0, s.size() - 3), tail = s.substr(s.size() - 3), out;
        int c = 0;
        for (int i = (int)head.size() - 1; i >= 0; --i) {
            out.insert(out.begin(), head[i]);
            if (++c % 2 == 0 && i > 0) out.insert(out.begin(), ',');
        }
        s = out + "," + tail;
    }
    return (neg ? "-" : "") + s;
}

std::string fmtL(float v) {
    float r = std::round(v * 10.f) / 10.f;
    if (std::fabs(r - std::round(r)) < 0.01f) return strf(RUPEE "%dL", (int)std::round(r));
    return strf(RUPEE "%.1fL", r);
}

std::string clockStr(float m) {
    int mm = (int)m;
    int h = (mm / 60) % 24, mi = mm % 60;
    int h12 = h % 12;
    if (h12 == 0) h12 = 12;
    return strf("%d:%02d %s", h12, mi, h < 12 ? "AM" : "PM");
}

Color candColor(int c) {
    switch (c) {
        case YOU: return Color{167, 131, 242, 255};
        case PD: return Color{61, 184, 179, 255};
        case LKM: return Color{238, 108, 156, 255};
        case IND: return Color{209, 168, 70, 255};
        default: return Color{158, 151, 172, 255};
    }
}
Color flagColor(int c) {
    switch (c) {
        case YOU: return Color{108, 56, 206, 255};
        case PD: return Color{16, 128, 122, 255};
        case LKM: return Color{200, 44, 104, 255};
        case IND: return Color{168, 120, 20, 255};
        default: return Color{120, 116, 130, 255};
    }
}

}  // namespace seat
