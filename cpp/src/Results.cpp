// Polling day (the EVM), counting day, and the final result.
#include "Game.hpp"

namespace seat {

static std::string candName(const Campaign& C, int k) { return k == YOU ? C.name : k == NOTA ? std::string("NOTA") : std::string(RIVALS[k].name); }
static std::string candParty(const Campaign& C, int k) { return k == YOU ? C.party : k == NOTA ? std::string("None of the above") : std::string(RIVALS[k].party); }

void Game::polling() {
    DrawRectangleGradientV(0, 0, 1280, 720, Color{40, 34, 58, 255}, Pal::bg);
    // the queue outside the booth
    static std::vector<Look> queue;
    if (queue.empty())
        for (int i = 0; i < 9; ++i) queue.push_back(randomLook(chance(.45f), irand(20, 70), (Bloc)irand(0, NBLOC - 1)));
    for (int i = 0; i < 9; ++i) gfx::drawPerson(queue[i], {70.f + i * 52, 690}, 1.f, 0, false, 1, PO_STAND, t, 0);
    ui::text("POLLING DAY " DOT " 7 AM TO 6 PM", 60, 60, 16, Pal::marigold, true);
    ui::text("Your turn to vote", 60, 84, 46, Pal::text, true);
    int v = C.votedFor;
    float y = 150;
    if (v < 0) {
        y += ui::textWrapped("Queues have formed outside every booth in Chandanpur. The polling officer has inked your left index finger. Press the blue button next to your choice.",
                             {60, y, 520, 200}, 19, Pal::text);
        // inked finger
        DrawRectangleRounded({80, y + 20, 26, 80}, .5f, 8, Color{224, 172, 105, 255});
        DrawRectangleRec({80, y + 30, 26, 6}, Color{90, 40, 170, 255});
        ui::text("Indelible ink: it won't wash off for weeks.", 124, y + 50, 15, Pal::muted);
    } else {
        std::string what = v == YOU ? "You voted for yourself, as one does." : v == NOTA ? "You pressed NOTA on your own ballot. Bold." : "You voted for " + candName(C, v) + ". Your campaign team is speechless.";
        y += ui::textWrapped("Beep. Your vote is recorded. " + what, {60, y, 520, 200}, 19, Pal::text) + 16;
        if (evmT > 2.5f) {
            long long polled = 0, voters = 0;
            for (int r = 0; r < NREGION; ++r) polled += C.res[r].polled, voters += REGIONS[r].voters;
            ui::text(strf("Polls have closed. Turnout: %.1f%% (%s of %s voters)", polled * 100.f / voters, fmtIN(polled).c_str(), fmtIN(voters).c_str()), 60, y, 17, Pal::text, true);
            y += 32;
            for (int r = 0; r < NREGION; ++r) {
                ui::text(REGIONS[r].name, 60, y, 15, Pal::muted, true);
                ui::bar({220, y + 4, 280, 11}, C.res[r].turnout, Pal::accent);
                ui::textRight(strf("%.0f%%", C.res[r].turnout * 100), 560, y, 15, Pal::text, true);
                y += 26;
            }
            if (ui::button({60, y + 16, 300, 50}, "Go to the counting centre", true, true, "", KEY_ENTER)) {
                count.build(C.res);
                autoCount = false;
                tween = 1;
                sc = SC_COUNT;
            }
        }
    }
    // ballot unit
    Rectangle B{690, 70, 480, 470};
    DrawRectangleRounded(B, .06f, 8, Color{205, 209, 216, 255});
    DrawRectangleRoundedLinesEx(B, .06f, 8, 3, Color{170, 175, 185, 255});
    ui::text("BALLOT UNIT " DOT " 07 CHANDANPUR", B.x + 20, B.y + 14, 13, Color{90, 90, 100, 255}, true);
    DrawCircleV({B.x + B.width - 60, B.y + 22}, 6, v < 0 ? Color{47, 191, 91, 255} : Color{60, 70, 60, 255});
    ui::text(v < 0 ? "READY" : "BUSY", B.x + B.width - 48, B.y + 14, 13, Color{90, 90, 100, 255}, true);
    int order[5] = {PD, LKM, IND, YOU, NOTA};
    evmT += dt;
    for (int i = 0; i < 5; ++i) {
        int k = order[i];
        float ry = B.y + 46 + i * 82;
        DrawLine((int)B.x + 10, (int)ry - 4, (int)(B.x + B.width - 10), (int)ry - 4, Color{170, 175, 185, 255});
        ui::text(strf("%d", i + 1), B.x + 20, ry + 24, 18, Color{90, 90, 100, 255}, true);
        Rectangle nm{B.x + 46, ry + 6, 230, 60};
        DrawRectangleRounded(nm, .1f, 6, WHITE);
        ui::text(candName(C, k), nm.x + 10, nm.y + 8, 17, Pal::ink, true);
        ui::text(candParty(C, k), nm.x + 10, nm.y + 32, 13, Color{100, 100, 110, 255});
        Rectangle sy{B.x + 284, ry + 6, 64, 60};
        DrawRectangleRounded(sy, .1f, 6, WHITE);
        gfx::drawSymbol(gfx::symbolOf(k, C.symbol), {sy.x + 32, sy.y + 32}, 40, false, WHITE);
        bool lit = v == k && evmT < 1.2f;
        DrawCircleV({B.x + 368, ry + 36}, 9, lit ? Color{255, 60, 50, 255} : Color{90, 30, 28, 255});
        if (lit) DrawCircleV({B.x + 368, ry + 36}, 16, Fade(Color{255, 60, 50, 255}, .3f));
        Rectangle bb{B.x + 392, ry + 20, 64, 34};
        bool hov = v < 0 && ui::hovered(bb);
        DrawRectangleRounded(bb, 1.f, 8, hov ? Color{60, 110, 230, 255} : Color{31, 79, 184, 255});
        if (v < 0 && ui::clickedIn(bb)) {
            C.votedFor = k;
            C.res[OLD].votes[k] += 1;
            C.res[OLD].polled += 1;
            evmT = 0;
            sfx::play(sfx::EVM, .7f);
        }
    }
    // VVPAT window
    Rectangle W{690, 556, 480, 110};
    DrawRectangleRounded(W, .1f, 8, Color{50, 48, 60, 255});
    ui::text("VVPAT", W.x + 16, W.y + 10, 13, Pal::muted, true);
    if (v >= 0 && evmT < 7) {
        float drop = evmT > 6 ? (evmT - 6) * 60 : 0;
        Rectangle slip{W.x + 140, W.y + 18 + drop, 200, 76};
        BeginScissorMode((int)W.x, (int)W.y, (int)W.width, (int)W.height);
        DrawRectangleRec(slip, WHITE);
        ui::text(strf("Sl. %d", v == PD ? 1 : v == LKM ? 2 : v == IND ? 3 : v == YOU ? 4 : 5), slip.x + 10, slip.y + 8, 14, Pal::ink, true);
        ui::text(candName(C, v), slip.x + 10, slip.y + 30, 15, Pal::ink, true);
        gfx::drawSymbol(gfx::symbolOf(v, C.symbol), {slip.x + 170, slip.y + 40}, 36, false, WHITE);
        EndScissorMode();
    } else
        ui::text(v < 0 ? "A paper slip of your vote will show here for 7 seconds." : "The slip has dropped into the sealed box.", W.x + 16, W.y + 50, 15, Pal::muted);
}

void Game::counting() {
    DrawRectangleGradientV(0, 0, 1280, 720, Color{30, 26, 44, 255}, Pal::bg);
    ui::text("COUNTING CENTRE " DOT " GOVERNMENT INTER COLLEGE", 60, 36, 15, Pal::marigold, true);
    ui::text("Counting day", 60, 58, 46, Pal::text, true);
    ui::textRight(strf("Round %d of %d", count.round, count.N), 1220, 74, 26, Pal::text, true);
    std::string where;
    if (count.round == 0) where = "Strong rooms are opened in front of the candidates' agents.";
    else {
        for (int r : count.active()) where += (where.empty() ? "Counting EVMs from: " : ", ") + std::string(REGIONS[r].name);
        if (where.empty()) where = "Final rounds.";
    }
    ui::text(where, 60, 120, 17, Pal::muted);
    tween = std::min(1.f, tween + dt * 1.6f);
    const Tally& cur = count.cum[count.round];
    float e = 1 - std::pow(1 - tween, 3.f);
    std::array<long long, NCAND> shown{};
    for (int k = 0; k < NCAND; ++k) shown[k] = (long long)std::llround(count.prev[k] + (cur[k] - count.prev[k]) * e);
    std::array<int, NCAND> ord{0, 1, 2, 3};
    std::sort(ord.begin(), ord.end(), [&](int a, int b) { return cur[a] > cur[b]; });
    long long maxv = std::max<long long>(1, cur[ord[0]]);
    for (int i = 0; i < NCAND; ++i) {
        int k = ord[i];
        Rectangle R{60, 156.f + i * 100, 1160, 88};
        DrawRectangleRounded(R, .12f, 8, k == YOU ? Color{44, 34, 70, 255} : Pal::panel2);
        if (k == YOU) DrawRectangleRounded({R.x, R.y, 6, R.height}, 1, 4, Pal::accent);
        gfx::drawSymbol(gfx::symbolOf(k, C.symbol), {R.x + 50, R.y + 42}, 50, false, WHITE);
        ui::text(candName(C, k), R.x + 96, R.y + 14, 22, Pal::text, true);
        ui::text(candParty(C, k), R.x + 96, R.y + 44, 15, Pal::muted);
        ui::textRight(fmtIN(shown[k]), R.x + 880, R.y + 20, 34, Pal::text, true);
        bool lead = i == 0 && count.round > 0;
        long long gap = lead ? cur[k] - cur[ord[1]] : cur[ord[0]] - cur[k];
        std::string st = count.round == 0 ? "Awaiting" : lead ? "Leading by " + fmtIN(gap) : "Trailing by " + fmtIN(gap);
        ui::chip(R.x + 910, R.y + 26, st, lead ? Fade(Pal::good, .25f) : Fade(Pal::muted, .2f), lead ? Pal::good : Pal::muted, 16);
        ui::bar({R.x + 96, R.y + 72, 1040, 6}, cur[k] / (float)maxv, candColor(k));
    }
    ui::text("NOTA so far: " + fmtIN(cur[NOTA]), 60, 560, 16, Pal::muted, true);
    auto next = [&] {
        if (count.round >= count.N) return;
        count.prev = count.cum[count.round];
        count.round++;
        tween = 0;
        sfx::play(sfx::BEEP, .4f);
        if (count.round == count.N) autoCount = false;
    };
    if (autoCount) {
        autoT -= dt;
        if (autoT <= 0) {
            next();
            autoT = 1.3f;
        }
    }
    float by = 600;
    if (count.round < count.N) {
        if (ui::button({60, by, 220, 50}, "Count next round", true, true, "", KEY_N)) {
            autoCount = false;
            next();
        }
        if (ui::button({292, by, 170, 50}, autoCount ? "Pause" : "Auto-count", true, false, "", KEY_A)) {
            autoCount = !autoCount;
            autoT = 0;
        }
        if (ui::button({474, by, 190, 50}, "Skip to result", true, false, "", KEY_S)) {
            count.prev = count.cum[count.N];
            count.round = count.N;
        }
    } else if (ui::button({60, by, 260, 50}, "Declare the result", true, true, "", KEY_ENTER)) {
        C.finishResult();
        sc = SC_RESULT;
        sfx::play(C.ended == Campaign::WIN ? sfx::CHEER : sfx::BOO, .7f);
        if (C.ended == Campaign::WIN) petals({640, 200}, 120);
    }
}

void Game::results() {
    DrawRectangle(0, 0, 1280, 720, Color{48, 40, 36, 255});
    Rectangle P{60, 24, 1160, 672};
    DrawRectangleRec(P, Pal::paper);
    Color ink = Pal::ink;
    ui::text("CHANDANPUR TIMES " DOT " COUNTING DAY SPECIAL", P.x + 30, P.y + 18, 15, Color{150, 30, 30, 255}, true);
    DrawRectangle((int)P.x + 30, (int)P.y + 42, (int)P.width - 60, 3, ink);
    std::string head, deck;
    if (C.ended == Campaign::DQ) {
        head = "EC disqualifies " + C.name;
        deck = "After three notices under the Model Code of Conduct, the Election Commission has barred the " + C.party +
               " candidate from the Chandanpur race. Gajendra Prakash is expected to coast to a fourth term. Cash, muscle and inflammatory speeches all win votes, until they don't.";
    } else if (C.ended == Campaign::WIN) {
        head = C.name + " wins Chandanpur";
        deck = "The " + C.party + " candidate beats " + candName(C, C.order[1]) + " by " + fmtIN(C.margin) + " votes" + (C.margin < 2000 ? " after a nail-biting count. " : ". ") +
               (C.strikes ? "The EC notices will follow you to Delhi." : "A clean campaign, and a seat in the Lok Sabha.");
    } else if (C.ended == Campaign::PETITION) {
        head = C.name + " wins, then loses in court";
        deck = "You won by " + fmtIN(C.margin) + " votes, but declared spending of " + fmtL(C.spent) + " crossed the " + fmtL(EC_LIMIT) +
               " limit. On an election petition, the High Court sets your election aside.";
    } else {
        head = candName(C, C.order[0]) + " wins Chandanpur";
        deck = candParty(C, C.order[0]) + " takes the seat by " + fmtIN(C.margin) + " votes. " + C.name + strf(" finishes %s with %.1f%% of the vote.", C.pos == 2 ? "second" : C.pos == 3 ? "third" : "fourth", C.share) +
               (C.pos == 2 && C.margin < 3000 ? " So close." : "");
    }
    float y = P.y + 56;
    y += ui::textWrapped(head, {P.x + 30, y, P.width - 60, 200}, 48, ink, true, 1.05f) + 6;
    y += ui::textWrapped(deck, {P.x + 30, y, P.width - 60, 200}, 19, ink) + 6;
    if (C.votedFor >= 0 && C.votedFor != YOU) {
        ui::text("Unconfirmed reports say the candidate did not vote for themself.", P.x + 30, y, 15, Color{120, 110, 100, 255});
        y += 24;
    }
    DrawRectangle((int)P.x + 30, (int)y + 4, (int)P.width - 60, 1, ink);
    y += 18;
    if (C.ended != Campaign::DQ) {
        // tally
        float tx = P.x + 30;
        ui::text("FINAL TALLY", tx, y, 14, Color{150, 30, 30, 255}, true);
        float ty = y + 24;
        for (int i = 0; i < NCAND; ++i) {
            int k = C.order[i];
            ui::text(candName(C, k), tx, ty, 17, ink, i == 0);
            ui::textRight(fmtIN(C.total[k]), tx + 400, ty, 17, ink, true);
            ui::textRight(strf("%.1f%%", C.total[k] * 100.f / C.valid), tx + 480, ty, 17, ink);
            ty += 28;
        }
        ui::text("NOTA", tx, ty, 17, ink);
        ui::textRight(fmtIN(C.total[NOTA]), tx + 400, ty, 17, ink, true);
        ty += 40;
        ui::text("AREA BY AREA", tx, ty, 14, Color{150, 30, 30, 255}, true);
        ty += 24;
        for (int r = 0; r < NREGION; ++r) {
            auto& v = C.res[r].votes;
            int top = (int)(std::max_element(v.begin(), v.begin() + NCAND) - v.begin());
            ui::text(REGIONS[r].name, tx, ty, 16, ink);
            ui::textRight(strf("%.0f%% turnout", C.res[r].turnout * 100), tx + 250, ty, 15, Color{110, 100, 100, 255});
            ui::textRight(fmtIN(v[YOU]) + " for you", tx + 400, ty, 15, ink);
            ui::textRight(top == YOU ? std::string("You won it") : std::string(RIVALS[top].shortName) + " won", tx + 520, ty, 15, top == YOU ? Color{30, 120, 70, 255} : ink, true);
            ty += 26;
        }
    }
    // right column: certificate or stamp, and campaign stats
    float rx = P.x + 620, ry = y;
    if (C.ended == Campaign::WIN) {
        Rectangle cert{rx, ry, 510, 170};
        DrawRectangleRec(cert, WHITE);
        DrawRectangleLinesEx(cert, 3, Pal::accentDeep);
        DrawRectangleLinesEx({cert.x + 8, cert.y + 8, cert.width - 16, cert.height - 16}, 1, Pal::accentDeep);
        ui::textCentered("FORM 22 " DOT " CERTIFICATE OF ELECTION", cert.x + cert.width / 2, cert.y + 18, 13, Pal::muted, true);
        ui::textCentered("Chandanpur Parliamentary Constituency", cert.x + cert.width / 2, cert.y + 40, 22, Pal::accentDeep, true);
        ui::textWrapped("I, the Returning Officer, certify that " + C.name + " of the " + C.party + " has been duly elected to fill the seat in the House of the People from 07 Chandanpur.",
                        {cert.x + 24, cert.y + 76, cert.width - 48, 90}, 15, ink);
        ry += 186;
    } else {
        std::string stamp = C.ended == Campaign::DQ ? "NOMINATION CANCELLED" : C.ended == Campaign::PETITION ? "ELECTION SET ASIDE" : C.share < 100.f / 6 ? "SECURITY DEPOSIT FORFEITED" : "";
        if (!stamp.empty()) {
            Vector2 m = ui::measure(stamp, 22, true);
            Rectangle s{rx + 10, ry + 10, m.x + 30, 44};
            DrawRectangleLinesEx(s, 3, Color{180, 35, 24, 255});
            ui::text(stamp, s.x + 15, s.y + 10, 22, Color{180, 35, 24, 255}, true);
            if (C.ended == Campaign::LOSE)
                ui::textWrapped(strf("Candidates who win less than one-sixth of valid votes lose their " RUPEE "25,000 deposit. You won %.1f%%.", C.share), {rx + 10, ry + 64, 480, 60}, 15, ink);
            ry += 120;
        }
    }
    ui::text("YOUR CAMPAIGN", rx, ry, 14, Color{150, 30, 30, 255}, true);
    ry += 24;
    std::vector<std::pair<std::string, std::string>> st = {
        {fmtL(C.spent) + " / " + fmtL(EC_LIMIT), "declared spending"},
        {strf("%d", C.stats.talks), "conversations"},
        {strf("%d", C.stats.rallies), "rallies"},
        {strf("%d", C.stats.roadshows), "roadshows"},
        {strf("%d", C.stats.converted), "voters won over in person"},
        {strf("%d", C.stats.hecklers), "hecklers faced"},
        {strf("%d", C.nPromises), "manifesto promises"},
        {strf("%d", C.nEndorse), "endorsements"},
        {strf("%d", C.strikes), "EC notices"},
        {DIFFS[C.diff].label, "difficulty"},
    };
    for (size_t i = 0; i < st.size(); ++i) {
        float cx = rx + (i % 2) * 260, cy = ry + (i / 2) * 44;
        ui::text(st[i].first, cx, cy, 19, ink, true);
        ui::text(st[i].second, cx, cy + 22, 13, Color{110, 100, 100, 255});
    }
    for (auto& p : parts) {
        p.p.x += p.v.x * dt;
        p.p.y += p.v.y * dt;
        p.v.y += 60 * dt;
        p.life -= dt * .4f;
        DrawCircleV(p.p, p.size, Fade(p.c, clampf(p.life, 0, 1)));
    }
    if (ui::button({P.x + 30, P.y + P.height - 64, 220, 46}, "Contest again", true, true, "", KEY_ENTER)) {
        parts.clear();
        startCampaign();
    }
    if (ui::button({P.x + 262, P.y + P.height - 64, 180, 46}, "Main menu")) {
        parts.clear();
        sc = SC_TITLE;
    }
}

}  // namespace seat
