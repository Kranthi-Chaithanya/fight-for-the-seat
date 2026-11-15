// Headless balance check: an automated candidate plays whole campaigns using the real game logic.
// Run: FightForTheSeat.exe --selftest [games]
#include <cstdio>
#include "Citizen.hpp"
#include "Events.hpp"
#include "Game.hpp"

namespace seat {

static int weakestRegion(Campaign& C) {
    int best = 0;
    float bv = -1e9f;
    for (int r = 0; r < NREGION; ++r) {
        float v = REGIONS[r].voters * (60 - C.el.exact(r)[YOU]) / (1 + C.visits[r] * .15f);
        if (v > bv) bv = v, best = r;
    }
    return best;
}

static void playEvents(Campaign& C) {
    auto takeFirst = [&](const EventDef& e) {
        C.used.push_back(e.id);
        int r = e.region ? e.region(C) : -1;
        if (e.onShow) e.onShow(C, r);
        for (auto& ch : e.choices) {
            if (ch.cond && !ch.cond(C)) continue;
            if (C.funds < ch.cost || ch.offbook) continue;
            if (ch.cost > 0) C.spend(ch.cost);
            if (ch.hours > 0) C.spendTime(ch.hours * 60, 3);
            ch.fn(C, r);
            return;
        }
    };
    if (C.day == 8) {
        C.spendTime(180, 4);
        for (auto& q : debate()) q.answers[irand(0, 2)].fx(C);
        return;
    }
    if (C.day == 5 || C.day == 10 || C.silence()) return;
    if (const EventDef* e = rollEvent(C)) takeFirst(*e);
}

static float simRally(Campaign& C, int r) {
    unsigned mask = 0, kinds = 0;
    float sum = 0;
    for (int round = 0; round < 4; ++round) {
        auto opts = C.speechOptions(r, mask);
        // a reasonable player picks what seems best, with imperfect judgement
        int bi = 0;
        float bv = -9;
        for (int i = 0; i < (int)opts.size(); ++i) {
            float guess = C.speechReaction(r, opts[i], kinds) + frand(-.35f, .35f);
            if (guess > bv) bv = guess, bi = i;
        }
        const SpeechLine& L = opts[bi];
        float rv = C.speechReaction(r, L, kinds);
        C.speechSideEffects(r, L);
        if (L.kind == SP_ISSUE) mask |= 1u << L.arg;
        else if (L.kind == SP_ATTACK) mask |= 1u << (8 + L.arg);
        else mask |= 1u << (L.kind == SP_EMOTION ? 12 : L.kind == SP_FREEBIE ? 13 : 14);
        kinds |= 1u << L.kind;
        sum += rv;
    }
    return clampf(.6f + sum / 4 * .8f, .2f, 1.4f);
}

int runSelfTest(int games) {
    std::printf("Fight for the Seat self-test: %d simulated campaigns per difficulty\n", games);
    for (int diff = 0; diff < 3; ++diff) {
        int wins = 0, dq = 0, petitions = 0;
        float shareSum = 0, spentSum = 0;
        int bgWins[4]{};
        for (int g = 0; g < games; ++g) {
            Campaign C;
            C.start("Bot", "Test Party", 0, g % 4, diff);
            std::vector<Citizen> people[NREGION];
            for (int r = 0; r < NREGION; ++r) people[r] = generateCitizens(r, C, 26, 3200);
            while (C.ended == Campaign::NONE) {
                playEvents(C);
                int talksHere = 0;
                while (C.minute < DAY_END - 20 && C.ended == Campaign::NONE) {
                    bool sil = C.silence();
                    if (!sil && C.nPromises < 3 && C.location < 0) {
                        int bestK = 0, bw = -1;
                        for (int k = 0; k < NISSUE; ++k) {
                            if (C.promised[k]) continue;
                            int w = 0;
                            for (int r = 0; r < NREGION; ++r) w += REGIONS[r].issues[k] * REGIONS[r].voters / 1000;
                            if (w > bw) bw = w, bestK = k;
                        }
                        C.promise(bestK);
                        continue;
                    }
                    if (!sil && C.nEndorse < NGROUP && C.location < 0 && !C.lateFor(120)) {
                        int pickG = -1;
                        for (int gi = 0; gi < NGROUP; ++gi)
                            if (!C.endorsed[gi] && (C.promised[GROUPS[gi].wants] || (C.funds > 14 && C.spent + 5 <= EC_LIMIT - 10))) {
                                pickG = gi;
                                break;
                            }
                        if (pickG >= 0) {
                            C.endorse(pickG);
                            continue;
                        }
                    }
                    if (C.funds < 8 && C.location < 0 && !C.lateFor(120)) {
                        C.fundSmall();
                        continue;
                    }
                    int r = weakestRegion(C);
                    if (C.location != r) {
                        if (C.lateFor(60)) break;
                        C.travel(r);
                        talksHere = 0;
                    }
                    if (!sil && C.funds >= 6 && C.spent + 6 <= EC_LIMIT - 3 && !C.lateFor(180) && C.visits[r] < 4) {
                        C.spend(6);
                        float q = simRally(C, r);
                        C.spendTime(180, 5);
                        C.rallyFinish(r, q, false, C.crowdSize(r, false));
                        continue;
                    }
                    if (C.booth[r] < 2 && C.funds >= 4 && C.spent + 2 <= EC_LIMIT - 3 && !C.lateFor(60)) {
                        C.trainBooth(r);
                        continue;
                    }
                    if (talksHere >= 10) {
                        C.location = -1;  // drive back and rethink
                        C.spendTime(60, 2);
                        continue;
                    }
                    Citizen& z = people[r][irand(0, (int)people[r].size() - 1)];
                    int id = C.promised[z.issue] ? TK_PROMISE : z.support > .25f ? (z.influencer && !z.convinced ? TK_RECRUIT : TK_VOTE) : TK_LISTEN;
                    applyOutcome(z, C, resolveTalk(z, C, id));
                    C.spendTime(8, 3);  // walking between people
                    talksHere++;
                }
                if (C.ended == Campaign::DQ) break;
                if (C.day == Campaign::maxDay) {
                    C.computePolling();
                    C.finishResult();
                    break;
                }
                C.endDay();
            }
            if (C.ended == Campaign::WIN) wins++, bgWins[C.bg]++;
            if (C.ended == Campaign::DQ) dq++;
            if (C.ended == Campaign::PETITION) petitions++;
            shareSum += C.share;
            spentSum += C.spent;
        }
        std::printf("  %-10s wins %3d/%d (%.0f%%)  avg share %.1f%%  avg spend %s  DQ %d  overturned %d  wins by background [act %d, biz %d, star %d, ias %d]\n", DIFFS[diff].label, wins, games,
                    wins * 100.f / games, shareSum / games, fmtL(spentSum / games).c_str(), dq, petitions, bgWins[0], bgWins[1], bgWins[2], bgWins[3]);
    }
    return 0;
}

}  // namespace seat
