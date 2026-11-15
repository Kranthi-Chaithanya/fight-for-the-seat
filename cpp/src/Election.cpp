#include "Election.hpp"

namespace seat {

void Election::init(float d) {
    D = d;
    for (int r = 0; r < NREGION; ++r)
        for (int c = 0; c < NCAND; ++c) score[r][c] = REGIONS[r].init[c];
    rerollNoise();
}

void Election::rerollNoise() {
    for (int r = 0; r < NREGION; ++r)
        for (int c = 0; c < NCAND; ++c) noise[r][c] = frand(-6.f, 6.f);
}

Shares Election::exact(int r) const {
    Shares s{};
    float t = 0;
    for (int c = 0; c < NCAND; ++c) t += score[r][c];
    for (int c = 0; c < NCAND; ++c) s[c] = score[r][c] / t * 100.f;
    return s;
}

Shares Election::shown(int r, bool ex) const {
    Shares e = exact(r);
    if (ex) return e;
    float t = 0;
    for (int c = 0; c < NCAND; ++c) {
        e[c] = std::max(1.f, e[c] + noise[r][c]);
        t += e[c];
    }
    for (int c = 0; c < NCAND; ++c) e[c] = e[c] / t * 100.f;
    return e;
}

Shares Election::overall(bool ex) const {
    Shares o{};
    float W = 0;
    for (int r = 0; r < NREGION; ++r) {
        Shares s = shown(r, ex);
        float w = REGIONS[r].voters * REGIONS[r].turnout;
        W += w;
        for (int c = 0; c < NCAND; ++c) o[c] += s[c] * w;
    }
    for (int c = 0; c < NCAND; ++c) o[c] /= W;
    return o;
}

int Election::leader(int r, bool ex) const {
    Shares s = shown(r, ex);
    return (int)(std::max_element(s.begin(), s.end()) - s.begin());
}

void Election::addYou(int r, float pts, float steal) {
    float* sc = score[r];
    if (pts < 0) {
        sc[YOU] = std::max(1.f, sc[YOU] + pts);
        return;
    }
    sc[YOU] += pts;
    float t = sc[PD] + sc[LKM] + sc[IND];
    for (int c = PD; c <= IND; ++c) sc[c] = std::max(1.f, sc[c] - pts * steal * sc[c] / t);
}

void Election::hit(int c, int r, float pts) {
    for (int i = 0; i < NREGION; ++i)
        if (r < 0 || r == i) score[i][c] = std::max(1.f, score[i][c] - pts);
}

void Election::boost(int c, int r, float pts) {
    for (int i = 0; i < NREGION; ++i)
        if (r < 0 || r == i) score[i][c] += pts;
}

void Count::build(const RegionResult res[NREGION]) {
    round = 0;
    cum.assign(N + 1, Tally{});
    for (int r = 0; r < NREGION; ++r) {
        int s = W[r][0], e = W[r][1], n = e - s + 1;
        for (int k = 0; k <= NCAND; ++k) {
            long long tot = res[r].votes[k];
            std::vector<float> w(n);
            float sw = 0;
            for (auto& x : w) sw += (x = frand(.65f, 1.35f));
            float acc = 0;
            for (int i = 1; i <= N; ++i) {
                if (i >= s && i <= e) acc += tot * w[i - s] / sw;
                cum[i][k] += i >= e ? tot : (long long)std::llround(acc);
            }
        }
    }
    prev = cum[0];
}

std::vector<int> Count::active() const {
    std::vector<int> v;
    for (int r = 0; r < NREGION; ++r)
        if (round >= W[r][0] && round <= W[r][1]) v.push_back(r);
    return v;
}
std::vector<int> Count::done() const {
    std::vector<int> v;
    for (int r = 0; r < NREGION; ++r)
        if (round > W[r][1]) v.push_back(r);
    return v;
}

}  // namespace seat
