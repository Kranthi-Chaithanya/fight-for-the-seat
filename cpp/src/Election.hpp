// Voter model: support scores per area, polling-day votes, and the counting schedule.
#pragma once
#include "Data.hpp"

namespace seat {

using Shares = std::array<float, NCAND>;
using Tally = std::array<long long, NCAND + 1>;  // + NOTA

struct RegionResult {
    float turnout = 0;
    long long polled = 0;
    Tally votes{};
};

struct Election {
    float score[NREGION][NCAND]{};
    float noise[NREGION][NCAND]{};
    float D = 1.f;

    void init(float difficulty);
    void rerollNoise();
    Shares exact(int r) const;
    Shares shown(int r, bool ex) const;  // with survey noise unless ex
    Shares overall(bool ex) const;
    int leader(int r, bool ex) const;

    // Your support grows; a share of it is taken from rivals in proportion to their strength.
    void addYou(int r, float pts, float steal = .3f);
    void hit(int c, int r, float pts);    // r = -1 for all areas
    void boost(int c, int r, float pts);  // r = -1 for all areas
};

// Counting happens area by area, so leads can swing between rounds.
struct Count {
    int round = 0;
    int N = 15;
    int W[NREGION][2] = {{7, 13}, {9, 15}, {2, 9}, {5, 12}, {1, 8}};
    std::vector<Tally> cum;
    Tally prev{};
    void build(const RegionResult res[NREGION]);
    std::vector<int> active() const;
    std::vector<int> done() const;
};

}  // namespace seat
