#include "Game.hpp"

namespace seat {

static std::vector<CrowdMember> titleCrowd;

// ---------------------------------------------------------------- lifecycle
void Game::init() {
    ui::load();
    sfx::init();
    for (int i = 0; i < 70; ++i) {
        CrowdMember m;
        m.look = randomLook(chance(.4f), irand(18, 65), (Bloc)irand(0, NBLOC - 1));
        m.p = {frand(0, 1280), frand(600, 720)};
        m.s = .8f + (m.p.y - 600) / 120.f * .5f;
        m.phase = frand(0, 6);
        m.pose = chance(.5f) ? PO_CHEER : PO_STAND;
        titleCrowd.push_back(m);
    }
    std::sort(titleCrowd.begin(), titleCrowd.end(), [](auto& a, auto& b) { return a.p.y < b.p.y; });
}

void Game::shutdown() {
    sfx::unload();
    ui::unload();
}

void Game::frame() {
    dt = std::min(GetFrameTime(), .05f);
    t += dt;
    if (IsKeyPressed(KEY_F1) || (IsKeyPressed(KEY_H) && sc != SC_SETUP)) showHelp = !showHelp;
    if (showHelp && IsKeyPressed(KEY_ESCAPE)) showHelp = false;
    BeginDrawing();
    ClearBackground(Pal::bg);
    ui::blocked = !dlgs.empty() || showHelp;
    switch (sc) {
        case SC_TITLE: title(); break;
        case SC_SETUP: setup(); break;
        case SC_MORNING: morning(); break;
        case SC_OFFICE: office(); break;
        case SC_STREET: street(); break;
        case SC_NIGHT: nightScreen(); break;
        case SC_POLL: polling(); break;
        case SC_COUNT: counting(); break;
        case SC_RESULT: results(); break;
    }
    if (!dlgs.empty() && !showHelp) {
        ui::blocked = false;
        drawDialog();
    }
    if (showHelp) {
        ui::blocked = false;
        drawHelp();
    }
    EndDrawing();
}

// ---------------------------------------------------------------- dialogs
Snap Game::snap() const { return {C.rep, C.hype, C.funds, C.yourShare(), C.stamina}; }

void Game::info(const std::string& kicker, const std::string& title, const std::string& body, std::function<std::string()> after) {
    Dialog d;
    d.kicker = kicker;
    d.title = title;
    d.body = body;
    d.opts.push_back({"Continue", "", true, after, true});
    push(d);
}

Dialog Game::resultDialog(const std::string& msg, const Snap& b) {
    Snap a = snap();
    std::vector<std::string> fx;
    auto add = [&](float v, const char* lab, const std::string& txt) {
        if (std::fabs(v) >= .05f) fx.push_back(std::string(lab) + " " + (v > 0 ? "+" : "-") + txt);
    };
    float ds = a.share - b.share;
    if (std::fabs(ds) >= .05f) fx.push_back(strf("Your vote share %s%.1f%%", ds > 0 ? "+" : "-", std::fabs(ds)));
    add(a.funds - b.funds, "Funds", fmtL(std::fabs(a.funds - b.funds)));
    if (std::fabs(a.rep - b.rep) >= .5f) add(a.rep - b.rep, "Reputation", strf("%.0f", std::fabs(a.rep - b.rep)));
    if (std::fabs(a.hype - b.hype) >= .5f) add(a.hype - b.hype, "Hype", strf("%.0f", std::fabs(a.hype - b.hype)));
    if (std::fabs(a.stamina - b.stamina) >= 1) add(a.stamina - b.stamina, "Stamina", strf("%.0f", std::fabs(a.stamina - b.stamina)));
    std::string chips;
    for (auto& f : fx) chips += (chips.empty() ? "" : "   " DOT "   ") + f;
    Dialog d;
    d.kicker = strf("Day %d " DOT " %s", C.day, clockStr(C.minute).c_str());
    d.title = "What happened";
    d.body = msg + (chips.empty() ? "" : "\n\n" + chips);
    d.opts.push_back({"Continue", "", true, nullptr, true});
    bool good = ds >= 0 && a.rep >= b.rep - .5f;
    C.addLog(msg, good ? T_GOOD : T_BAD);
    return d;
}

void Game::drawDialog() {
    Dialog d = dlgs.front();
    DrawRectangle(0, 0, 1280, 720, Fade(BLACK, .55f));
    bool portrait = d.citizen >= 0 && sc == SC_STREET && d.citizen < (int)people[region].size();
    float W = portrait ? 860.f : 660.f;
    float left = portrait ? 220.f : 0.f;
    float tw = W - 48 - left;
    float titleH = ui::wrappedHeight(d.title, tw, 30, true, 1.15f);
    float bodyH = ui::wrappedHeight(d.body, tw, 18);
    float barsH = d.bars ? (float)NCAND * 30.f + 10 : 0;
    float optsH = 0;
    for (auto& o : d.opts) optsH += (o.sub.empty() ? 44 : 54) + 8;
    float H = 22 + (d.kicker.empty() ? 0 : 24) + titleH + 8 + bodyH + barsH + 18 + optsH + 12;
    if (portrait) H = std::max(H, 380.f);
    Rectangle R{(1280 - W) / 2, std::max(10.f, (720 - H) / 2), W, H};
    DrawRectangleRounded(R, .03f, 8, Color{30, 25, 42, 252});
    DrawRectangleRoundedLinesEx(R, .03f, 8, 1.5f, Pal::line);
    DrawRectangleRec({R.x + 14, R.y, W - 28, 5}, d.accent);
    if (portrait) {
        Citizen& z = people[region][d.citizen];
        Rectangle pr{R.x + 16, R.y + 18, 200, H - 36};
        DrawRectangleRounded(pr, .05f, 8, Color{44, 38, 60, 255});
        DrawRectangleGradientV((int)pr.x, (int)pr.y, (int)pr.width, 220, Fade(gfx::skyTop(C.minute), .5f), Fade(gfx::skyBottom(C.minute), .3f));
        gfx::drawPerson(z.look, {pr.x + 100, pr.y + 225}, 2.3f, 0, false, 1, z.pose, t, z.mood());
        float ty = pr.y + 236;
        ui::textCentered(z.name + strf(", %d", z.age), pr.x + 100, ty, 19, Pal::text, true);
        ui::textCentered(z.job, pr.x + 100, ty + 24, 15, Pal::muted);
        const char* moodTxt = z.support > .5f ? "Strong supporter" : z.support > .2f ? "Leaning your way" : z.support > -.2f ? "Undecided" : z.support > -.5f ? "Sceptical" : "Hostile";
        Color mc = z.support > .2f ? Pal::good : z.support > -.2f ? Pal::marigold : Pal::bad;
        ui::textCentered(moodTxt, pr.x + 100, ty + 48, 15, mc, true);
        ui::bar({pr.x + 30, ty + 72, 140, 8}, (z.support + 1) / 2, mc);
        if (z.influencer) ui::textCentered("INFLUENCER " DOT " sways neighbours", pr.x + 100, ty + 88, 12, Pal::accent, true);
        ui::textCentered(std::string("Cares about: ") + ISSUES[z.issue].label, pr.x + 100, ty + (z.influencer ? 106 : 90), 13, Pal::muted, true);
    }
    float x = R.x + 24 + left, y = R.y + 20;
    if (!d.kicker.empty()) {
        ui::text(d.kicker, x, y, 14, Pal::marigold, true);
        y += 24;
    }
    y += ui::textWrapped(d.title, {x, y, tw, 400}, 30, Pal::text, true, 1.15f);
    y += 8;
    y += ui::textWrapped(d.body, {x, y, tw, 400}, 18, Pal::text);
    if (d.bars) {
        y += 8;
        std::array<int, NCAND> order{0, 1, 2, 3};
        std::sort(order.begin(), order.end(), [&](int a, int b) { return d.barValues[a] > d.barValues[b]; });
        for (int k : order) {
            ui::text(k == YOU ? C.name : RIVALS[k].name, x, y, 16, Pal::text, true);
            ui::bar({x + 190, y + 4, tw - 260, 12}, d.barValues[k] / 60.f, candColor(k));
            ui::textRight(strf("%.0f%%", d.barValues[k]), x + tw, y, 16, Pal::text, true);
            y += 30;
        }
    }
    y += 18;
    for (size_t i = 0; i < d.opts.size(); ++i) {
        const DlgOpt& o = d.opts[i];
        float h = o.sub.empty() ? 44.f : 54.f;
        std::string lab = d.opts.size() > 1 ? strf("%d   ", (int)i + 1) + o.label : o.label;
        int key = d.opts.size() == 1 ? KEY_ENTER : (i < 9 ? KEY_ONE + (int)i : 0);
        if (ui::button({x, y, tw, h}, lab, o.enabled, o.primary, o.sub, key)) {
            Snap b = snap();
            auto fn = o.fn;
            dlgs.pop_front();
            std::string r = fn ? fn() : "";
            C.normalize();
            if (!r.empty()) dlgs.push_front(resultDialog(r, b));
            afterAction();
            return;
        }
        y += h + 8;
    }
}

void Game::afterAction() {
    if (C.ended == Campaign::DQ && sc != SC_RESULT) {
        dlgs.clear();
        sc = SC_RESULT;
        sfx::play(sfx::BOO);
        return;
    }
    if ((sc == SC_OFFICE || sc == SC_STREET) && smode == SM_WALK && C.minute >= DAY_END && dlgs.empty()) {
        info(strf("Day %d " DOT " %s", C.day, clockStr(C.minute).c_str()), "The day is over",
             "It's past 10 PM. Your team packs up the loudspeakers, the drivers want to go home, and your voice is almost gone.",
             [this] {
                 endDayNow();
                 return std::string();
             });
    }
}

void Game::runAction(const std::function<std::string()>& fn) {
    Snap b = snap();
    std::string r = fn();
    C.normalize();
    if (!r.empty()) dlgs.push_front(resultDialog(r, b));
    afterAction();
}

// ---------------------------------------------------------------- shared HUD pieces
void Game::drawCandidateAt(Vector2 f, float s, int pose, bool walking, float phase, int facing) {
    gfx::drawPerson(candidateLook(YOU), f, s, phase, walking, facing, pose, t, 1);
}

void Game::drawHud() {
    DrawRectangle(0, 0, 1280, 54, Color{22, 18, 32, 245});
    DrawRectangle(0, 54, 1280, 2, Pal::accentDeep);
    auto label = [&](float x, const char* k) { ui::text(k, x, 5, 12, Pal::muted, true); };
    label(16, "DAY");
    ui::text(strf("%d/%d", C.day, Campaign::maxDay), 16, 20, 22, Pal::text, true);
    ui::text(C.silence() ? "silence" : strf("poll in %d", Campaign::maxDay - C.day + 1), 74, 27, 13, C.silence() ? Pal::bad : Pal::muted);
    label(150, "TIME");
    ui::text(clockStr(C.minute), 150, 20, 22, C.minute > DAY_END - 60 ? Pal::marigold : Pal::text, true);
    label(270, "FUNDS");
    ui::text(fmtL(C.funds), 270, 20, 22, Pal::text, true);
    ui::text(strf("spent %s/%s", fmtL(C.spent).c_str(), fmtL(EC_LIMIT).c_str()), 340, 27, 13, C.spent > EC_LIMIT * .85f ? Pal::bad : Pal::muted);
    auto meter = [&](float x, const char* k, float v, float max, Color c) {
        label(x, k);
        ui::bar({x, 30, 96, 9}, v / max, c);
        ui::textRight(strf("%.0f", v), x + 96, 5, 12, Pal::text, true);
    };
    meter(486, "REPUTATION", C.rep, 100, Pal::good);
    meter(600, "HYPE", C.hype, 60, Pal::marigold);
    meter(714, "STAMINA", C.stamina, 100, Color{110, 170, 240, 255});
    label(828, "EC NOTICES");
    for (int i = 0; i < 3; ++i) {
        Rectangle b{828.f + i * 20, 27, 14, 14};
        if (i < C.strikes) DrawRectangleRec(b, Pal::bad);
        else DrawRectangleLinesEx(b, 2, Pal::line);
    }
    Shares ov = C.el.overall(C.surveyed());
    int rank = 1;
    for (int k = PD; k <= IND; ++k) rank += ov[k] > ov[YOU];
    label(920, C.surveyed() ? "SEAT (SURVEY)" : "SEAT (ESTIMATE)");
    ui::text(strf("%s%.0f%%", C.surveyed() ? "" : "~", ov[YOU]), 920, 20, 22, candColor(YOU), true);
    ui::text(strf("rank %d of 4", rank), 980, 27, 13, Pal::muted);
    if (ui::button({1072, 12, 62, 30}, showFeed ? "Feed" : "Feed", true, showFeed, "", KEY_TAB)) showFeed = !showFeed;
    if (ui::button({1140, 12, 56, 30}, "Help")) showHelp = true;
    if (ui::button({1202, 12, 66, 30}, sfx::muted ? "Muted" : "Sound")) sfx::muted = !sfx::muted;
}

void Game::drawFeed(Rectangle r) {
    DrawRectangleRounded(r, .03f, 8, Fade(Color{22, 18, 32, 255}, .9f));
    ui::text("Chandanpur online", r.x + 14, r.y + 10, 18, Pal::text, true);
    ui::text("What voters are posting", r.x + 14, r.y + 32, 13, Pal::muted);
    BeginScissorMode((int)r.x, (int)r.y + 56, (int)r.width, (int)r.height - 60);
    float y = r.y + 60;
    for (auto& p : C.feed) {
        if (y > r.y + r.height) break;
        Color hc = p.tone == T_GOOD ? Pal::good : p.tone == T_BAD ? Pal::bad : p.tone == T_RIVAL ? Color{120, 200, 196, 255} : Pal::accent;
        ui::text(p.handle, r.x + 14, y, 14, hc, true);
        ui::textRight(strf("Day %d", p.day), r.x + r.width - 14, y, 12, Pal::muted);
        y += 19;
        y += ui::textWrapped(p.text, {r.x + 14, y, r.width - 28, 300}, 14, Pal::text, false, 1.25f);
        ui::text(fmtIN(p.likes) + " likes", r.x + 14, y, 12, Pal::muted);
        y += 22;
        DrawLine((int)r.x + 14, (int)y - 5, (int)(r.x + r.width - 14), (int)y - 5, Fade(Pal::line, .6f));
    }
    EndScissorMode();
}

void Game::drawTicker() {
    DrawRectangle(0, 694, 1280, 26, Color{150, 30, 30, 255});
    DrawRectangle(0, 694, 150, 26, Color{20, 16, 28, 255});
    std::string s;
    int n = 0;
    for (auto& l : C.log) {
        if (n++ >= 7) break;
        s += l.text + "      " DOT "      ";
    }
    float w = ui::measure(s, 15, true).x;
    tickerX -= dt * 80;
    if (tickerX < 150 - w) tickerX = 1280;
    BeginScissorMode(150, 694, 1130, 26);
    ui::text(s, tickerX, 698, 15, WHITE, true);
    EndScissorMode();
    ui::text("CP TIMES LIVE", 14, 698, 15, Pal::marigold, true);
}

void Game::drawHelp() {
    DrawRectangle(0, 0, 1280, 720, Fade(BLACK, .7f));
    Rectangle R{240, 70, 800, 580};
    ui::panel(R, Color{30, 25, 42, 252});
    ui::text("How to play", R.x + 30, R.y + 24, 32, Pal::text, true);
    const char* lines[] = {
        "You have 14 days, 8 AM to 10 PM each day, to win Chandanpur's 1,72,000 voters.",
        "CAMPAIGN OFFICE: click an area on the map to travel there (1 hour). Office work:",
        "   promises, endorsements, press conferences, social media, fundraising, surveys, rest.",
        "ON THE STREET: walk with A/D or the arrow keys, W/S to step up or down, Shift to hurry.",
        "   E = talk to the person nearest you.  F = fold your hands and greet them.",
        "   Every citizen has a name, a job, an issue, a political leaning and a memory.",
        "   Hecklers from rival camps will come at you. How you respond is filmed.",
        "   R = hold a rally (pick 4 speech lines, the crowd reacts to each).",
        "   T = roadshow   B = train booth agents   G = \"gifts\" (illegal)   M = back to office",
        "Stamina drops as you work; tired candidates persuade fewer people. Sleep restores it.",
        "Keep declared spending under " RUPEE "95 lakh. Three EC notices and you're disqualified.",
        "Tab toggles the social feed. H or F1 toggles this help.",
    };
    float y = R.y + 80;
    for (auto l : lines) {
        y += ui::textWrapped(l, {R.x + 30, y, R.width - 60, 100}, 17, Pal::text) + 6;
    }
    if (ui::button({R.x + R.width - 170, R.y + R.height - 60, 140, 40}, "Close", true, true, "", KEY_ENTER)) showHelp = false;
}

// ---------------------------------------------------------------- title and setup
void Game::title() {
    gfx::drawSky(1095, t);
    gfx::drawFar(OLD, t * 15, 1095, t);
    DrawRectangle(0, 560, 1280, 160, Color{60, 45, 50, 255});
    for (auto& m : titleCrowd) {
        int pose = std::fmod(t + m.phase, 4.f) < 2 ? PO_CHEER : m.pose;
        gfx::drawPerson(m.look, m.p, m.s, 0, false, 1, pose, t + m.phase, 1, true);
    }
    for (int i = 0; i < 6; ++i) {
        int cand = i % 4;
        gfx::drawFlag({110.f + i * 200 + std::sin(t + i) * 4, 640}, 140, flagColor(cand), gfx::symbolOf(cand, inSym), t + i);
    }
    DrawRectangleGradientV(0, 0, 1280, 420, Fade(BLACK, .55f), Fade(BLACK, 0));
    ui::text("GENERAL ELECTION " DOT " 07 CHANDANPUR CONSTITUENCY", 80, 70, 18, Pal::marigold, true);
    ui::text("FIGHT FOR", 84, 104, 96, Fade(BLACK, .5f), true);
    ui::text("FIGHT FOR", 80, 100, 96, Pal::text, true);
    ui::text("THE SEAT", 84, 194, 96, Fade(BLACK, .5f), true);
    ui::text("THE SEAT", 80, 190, 96, Pal::accent, true);
    ui::textWrapped("Fourteen days. 1,72,000 voters. Three rivals who have done this before. Walk the streets, face the hecklers, hold the rallies and win a seat in the Lok Sabha.",
                    {84, 300, 620, 200}, 20, Pal::text);
    if (ui::button({84, 390, 230, 50}, "New campaign", true, true, "", KEY_ENTER)) sc = SC_SETUP;
    if (ui::button({326, 390, 150, 50}, "How to play")) showHelp = true;
#ifndef __EMSCRIPTEN__
    if (ui::button({488, 390, 100, 50}, "Quit")) quit = true;
#endif
    ui::text("All candidates, parties and places in this game are fictional.", 84, 456, 14, Fade(Pal::text, .8f));
}

void Game::setup() {
    DrawRectangleGradientV(0, 0, 1280, 720, Color{28, 22, 40, 255}, Pal::bg);
    ui::text("FORM 2A " DOT " NOMINATION PAPER", 60, 34, 15, Pal::marigold, true);
    ui::text("Who is contesting?", 60, 56, 38, Pal::text, true);

    auto field = [&](int id, const char* lab, std::string& val, float x) {
        ui::text(lab, x, 118, 15, Pal::muted, true);
        Rectangle b{x, 140, 300, 44};
        bool f = focus == id;
        DrawRectangleRounded(b, .2f, 8, Pal::panel2);
        DrawRectangleRoundedLinesEx(b, .2f, 8, f ? 2.f : 1.f, f ? Pal::accent : Pal::line);
        ui::text(val + (f && std::fmod(t, 1.f) < .5f ? "|" : ""), b.x + 12, b.y + 11, 19, Pal::text);
        if (ui::clickedIn(b)) focus = id;
    };
    field(0, "CANDIDATE NAME", inName, 60);
    field(1, "PARTY NAME", inParty, 380);
    if (IsKeyPressed(KEY_TAB)) focus = (focus + 1) % 2;
    std::string& cur = focus == 0 ? inName : inParty;
    for (int ch = GetCharPressed(); ch > 0; ch = GetCharPressed())
        if (ch >= 32 && ch < 127 && cur.size() < 24) cur.push_back((char)ch);
    if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && !cur.empty()) cur.pop_back();

    ui::text("ELECTION SYMBOL", 60, 204, 15, Pal::muted, true);
    for (int i = 0; i < NSYMBOL; ++i) {
        Rectangle b{60.f + i * 104, 226, 96, 92};
        bool on = inSym == i;
        DrawRectangleRounded(b, .15f, 8, on ? Fade(Pal::accentDeep, .6f) : Pal::panel2);
        DrawRectangleRoundedLinesEx(b, .15f, 8, on ? 2.f : 1.f, on ? Pal::accent : Pal::line);
        gfx::drawSymbol(i, {b.x + 48, b.y + 40}, 46, false, WHITE);
        ui::textCentered(SYMBOL_NAMES[i], b.x + 48, b.y + 68, 14, Pal::text, true);
        if (ui::clickedIn(b)) inSym = i;
    }
    ui::text("BACKGROUND", 60, 334, 15, Pal::muted, true);
    for (int i = 0; i < 4; ++i) {
        Rectangle b{60.f + i * 158, 356, 150, 92};
        bool on = inBg == i;
        DrawRectangleRounded(b, .12f, 8, on ? Fade(Pal::accentDeep, .6f) : Pal::panel2);
        DrawRectangleRoundedLinesEx(b, .12f, 8, on ? 2.f : 1.f, on ? Pal::accent : Pal::line);
        ui::text(BGS[i].label, b.x + 10, b.y + 8, 16, Pal::text, true);
        ui::textWrapped(BGS[i].sub, {b.x + 10, b.y + 32, b.width - 20, 60}, 13, Pal::muted, false, 1.2f);
        if (ui::clickedIn(b)) inBg = i;
    }
    ui::text("DIFFICULTY", 60, 464, 15, Pal::muted, true);
    for (int i = 0; i < 3; ++i) {
        Rectangle b{60.f + i * 210, 486, 200, 64};
        bool on = inDiff == i;
        DrawRectangleRounded(b, .15f, 8, on ? Fade(Pal::accentDeep, .6f) : Pal::panel2);
        DrawRectangleRoundedLinesEx(b, .15f, 8, on ? 2.f : 1.f, on ? Pal::accent : Pal::line);
        ui::text(DIFFS[i].label, b.x + 12, b.y + 8, 17, Pal::text, true);
        ui::text(DIFFS[i].sub, b.x + 12, b.y + 34, 13, Pal::muted);
        if (ui::clickedIn(b)) inDiff = i;
    }
    if (ui::button({60, 590, 240, 50}, "Start the campaign", !inName.empty() && !inParty.empty(), true, "", KEY_ENTER)) startCampaign();
    if (ui::button({312, 590, 120, 50}, "Back")) sc = SC_TITLE;

    // preview: you on a small stage with a volunteer and your flag
    Rectangle pv{760, 110, 460, 530};
    DrawRectangleRounded(pv, .04f, 8, Color{36, 30, 52, 255});
    DrawRectangleGradientV((int)pv.x, (int)pv.y + 6, (int)pv.width, 300, Color{90, 70, 140, 255}, Color{36, 30, 52, 255});
    Look me = candidateLook(YOU);
    gfx::drawPerson(me, {950, 470}, 2.6f, 0, false, 1, (int)std::fmod(t / 2.5f, 2.f) ? PO_NAMASTE : PO_WAVE, t, 1);
    gfx::drawFlag({1070, 470}, 200, flagColor(YOU), inSym, t);
    gfx::drawPoster({800, 300, 90, 120}, YOU, inSym, inName.substr(0, std::min<size_t>(10, inName.size())));
    ui::textCentered(inName.empty() ? "Your name" : inName, 990, 500, 26, Pal::text, true);
    ui::textCentered(inParty + "  " DOT "  " + SYMBOL_NAMES[inSym], 990, 534, 17, Pal::accent, true);
    ui::textCentered(BGS[inBg].label, 990, 566, 15, Pal::muted, true);
}

void Game::startCampaign() {
    C.start(inName, inParty, inSym, inBg, inDiff);
    for (auto& g : generated) g = false;
    for (auto& p : people) p.clear();
    dlgs.clear();
    smode = SM_WALK;
    nowDoing = "Planning the day at the campaign office";
    sc = SC_OFFICE;
    info("Day 1 " DOT " Nomination accepted", "Welcome to Chandanpur, " + C.name,
         "Sitting MP Gajendra Prakash (Pragati Dal) is chasing a fourth term. Adv. Sunita Rao (Lok Kalyan Morcha) owns the campuses and the township. "
         "Bhola Pehelwan rules Mill Colony and the old city.\n\nYou start at about 9%. Click an area on the map to go there and meet people. "
         "Your day runs from 8 AM to 10 PM. Press H at any time for controls.");
}

// ---------------------------------------------------------------- morning newspaper
void Game::morning() {
    DrawRectangle(0, 0, 1280, 720, Color{48, 40, 36, 255});
    Rectangle P{150, 30, 980, 660};
    DrawRectangleRec({P.x + 8, P.y + 8, P.width, P.height}, Fade(BLACK, .4f));
    DrawRectangleRec(P, Pal::paper);
    Color ink = Pal::ink;
    ui::textCentered("CHANDANPUR TIMES", P.x + P.width / 2, P.y + 18, 54, ink, true);
    DrawRectangle((int)P.x + 30, (int)P.y + 84, (int)P.width - 60, 3, ink);
    ui::text(strf("Day %d of %d  " DOT "  Election special", C.day, Campaign::maxDay), P.x + 32, P.y + 92, 15, ink, true);
    ui::textRight(C.day == Campaign::maxDay ? "Campaign silence begins today" : strf("%d days to polling", Campaign::maxDay - C.day + 1), P.x + P.width - 32, P.y + 92,
                  15, ink, true);
    DrawRectangle((int)P.x + 30, (int)P.y + 116, (int)P.width - 60, 1, ink);
    std::string head = C.headline.empty() ? "Three-way fight gets tighter in Chandanpur" : C.headline;
    float y = P.y + 132;
    y += ui::textWrapped(head, {P.x + 32, y, P.width - 64, 200}, 40, ink, true, 1.1f) + 8;
    // columns
    float colW = (P.width - 96) / 3;
    auto column = [&](float x, const char* heading, const std::string& body) {
        ui::text(heading, x, y, 15, Color{150, 30, 30, 255}, true);
        ui::textWrapped(body, {x, y + 26, colW, 300}, 18, ink, false, 1.35f);
    };
    std::string rivals = "Quiet day on the rival trail.", ground = "Your team is still reading the ground.", online = "Nothing trending yet.";
    for (auto& l : C.log)
        if (l.tone == T_RIVAL) {
            rivals = l.text.substr(l.text.find(':') == std::string::npos ? 0 : l.text.find(':') + 2);
            break;
        }
    int n = 0;
    for (auto& l : C.log)
        if ((l.tone == T_GOOD || l.tone == T_BAD) && l.day >= C.day - 1 && n < 2) {
            ground = n == 0 ? l.text : ground + "\n\n" + l.text;
            n++;
        }
    const Post* best = nullptr;
    for (auto& p : C.feed)
        if (p.day >= C.day - 1 && (!best || p.likes > best->likes)) best = &p;
    if (best) online = best->handle + ": \"" + best->text + "\"  (" + fmtIN(best->likes) + " likes)";
    column(P.x + 32, "ON THE RIVAL TRAIL", rivals);
    column(P.x + 48 + colW, "ON THE GROUND", ground);
    column(P.x + 64 + colW * 2, "TRENDING ONLINE", online);
    // the street's rough read on the race
    float pulseY = P.y + P.height - 200;
    DrawRectangle((int)P.x + 30, (int)pulseY - 14, (int)P.width - 60, 1, ink);
    ui::text("CHANDANPUR PULSE  " DOT "  what the tea stalls are saying (rough estimate)", P.x + 32, pulseY, 15, Color{150, 30, 30, 255}, true);
    Shares pulse = C.el.overall(false);
    for (int k = 0; k < NCAND; ++k) {
        float by = pulseY + 30 + k * 30;
        ui::text(k == YOU ? C.name : RIVALS[k].name, P.x + 32, by, 17, ink, true);
        ui::bar({P.x + 260, by + 5, 360, 12}, pulse[k] / 60.f, flagColor(k), Color{220, 214, 200, 255});
        ui::text(strf("~%.0f%%", pulse[k]), P.x + 634, by, 17, ink, true);
    }
    if (ui::button({P.x + P.width - 260, P.y + P.height - 66, 228, 46}, "Start the day", true, true, "", KEY_ENTER)) {
        C.headline.clear();
        sc = SC_OFFICE;
        nowDoing = "Planning the day at the campaign office";
        queueMorningEvents();
    }
}

void Game::queueMorningEvents() {
    if (C.day == 5 || C.day == 10) return pollDialog();
    if (C.day == 8) {
        Dialog d;
        d.kicker = "Day 8 " DOT " Chandanpur University";
        d.title = "Debate night";
        d.body = "The Chhatra Sangh has invited every candidate to a live debate. Three questions, one stage, every phone camera in the hall pointed at you.";
        d.opts.push_back({"Take the stage", "Takes 3 hours this evening", true, [this] {
                               C.spendTime(180, 4);
                               debateDialog(0, "");
                               return std::string();
                           },
                           true});
        d.opts.push_back({"Skip it", "Rivals will mock your empty chair", true, [this] {
                              C.hype -= 6;
                              C.el.addYou(UNI, -4);
                              return std::string("An empty chair with your name on it becomes the photo of the night.");
                          }});
        push(d);
        return;
    }
    if (C.day == Campaign::maxDay) {
        info("Day 14 " DOT " Model Code of Conduct", "Campaign silence begins",
             "For the last 48 hours before polling, public campaigning stops: no rallies, roadshows, press conferences, social media campaigns or new promises.\n\n"
             "You can still walk door to door, train booth agents, raise funds and run a survey. This is also the night when \"gifts\" change hands, and the flying squads know it.");
        return;
    }
    if (const EventDef* e = rollEvent(C)) eventDialog(*e);
}

void Game::eventDialog(const EventDef& e) {
    C.used.push_back(e.id);
    int r = e.region ? e.region(C) : -1;
    if (e.onShow) e.onShow(C, r);
    Dialog d;
    d.kicker = strf("Breaking " DOT " Day %d", C.day);
    d.title = e.title;
    d.body = e.text(C, r);
    d.accent = Pal::bad;
    for (const EventChoice& ch : e.choices) {
        if (ch.cond && !ch.cond(C)) continue;
        std::string sub;
        if (ch.cost > 0) sub = fmtL(ch.cost);
        if (!ch.sub.empty()) sub += (sub.empty() ? "" : "  " DOT "  ") + ch.sub;
        bool ok = C.funds >= ch.cost && !C.lateFor(ch.hours * 60);
        EventChoice copy = ch;
        d.opts.push_back({ch.label, sub, ok, [this, copy, r] {
                              if (copy.cost > 0) C.spend(copy.cost, copy.offbook);
                              if (copy.hours > 0) C.spendTime(copy.hours * 60, 3);
                              return copy.fn(C, r);
                          }});
    }
    bool any = false;
    for (auto& o : d.opts) any |= o.enabled;
    if (!any) d.opts.push_back({"Let it pass", "", true, [] { return std::string("You let this one pass."); }});
    push(d);
}

void Game::pollDialog() {
    Shares ex = C.el.overall(true), o{};
    float tot = 0;
    for (int k = 0; k < NCAND; ++k) tot += (o[k] = std::max(1.f, ex[k] + frand(-3, 3)));
    for (auto& v : o) v = v / tot * 100;
    int pos = 1;
    for (int k = PD; k <= IND; ++k) pos += o[k] > o[YOU];
    float leader = *std::max_element(o.begin(), o.end());
    std::string note;
    if (pos == 1) note = "You are the front-runner. Expect all three rivals to come after you.";
    else if (leader - o[YOU] < 6) note = strf("You are %s, within striking distance.", pos == 2 ? "second" : pos == 3 ? "third" : "fourth");
    else {
        note = strf("You are %s. The underdog story gives your campaign a small lift.", pos == 2 ? "second" : pos == 3 ? "third" : "fourth");
        C.hype += 3;
    }
    Dialog d;
    d.kicker = strf("Chandanpur Times " DOT " Jan Mat survey " DOT " Day %d", C.day);
    d.title = pos == 1 ? "Survey puts you ahead" : "New opinion poll is out";
    d.body = "Sample of 2,400 voters, margin of error 3 points. " + note;
    d.bars = true;
    d.barValues = o;
    d.opts.push_back({"Back to work", "", true, nullptr, true});
    C.addLog(strf("Jan Mat survey: you at %.0f%%, rank %d of 4.", o[YOU], pos), T_INFO);
    push(d);
}

void Game::debateDialog(size_t q, std::string said) {
    const auto& D = debate();
    Dialog d;
    d.kicker = strf("Chhatra Sangh debate " DOT " Question %d of %d", (int)q + 1, (int)D.size());
    d.title = D[q].q;
    d.body = D[q].ctx;
    for (size_t i = 0; i < D[q].answers.size(); ++i) {
        d.opts.push_back({D[q].answers[i].text, "", true, [this, q, said, i] {
                              const auto& D2 = debate();
                              std::string r = D2[q].answers[i].fx(C);
                              std::string s2 = said.empty() ? r : said + "; " + r;
                              sfx::play(sfx::CLAP, .6f);
                              if (q + 1 < D2.size()) {
                                  debateDialog(q + 1, s2);
                                  return std::string();
                              }
                              C.react("debate", -1, .5f);
                              C.doing("Debate at Chandanpur University");
                              return "Debate verdict: " + s2 + ". Clips of your answers dominate the evening news.";
                          }});
    }
    dlgs.push_front(d);
}

// ---------------------------------------------------------------- campaign office
void Game::office() {
    DrawRectangleGradientV(0, 0, 1280, 720, Color{32, 26, 46, 255}, Pal::bg);
    drawHud();
    // map
    Rectangle M{24, 96, 560, 420};
    ui::text("Chandanpur constituency", 24, 66, 20, Pal::text, true);
    ui::textRight(C.surveyed() ? "Survey data: exact" : "Estimates, can be off by 6 points", 584, 70, 14, C.surveyed() ? Pal::good : Pal::muted);
    std::array<int, NREGION> lead{};
    std::array<float, NREGION> yours{};
    for (int r = 0; r < NREGION; ++r) {
        lead[r] = C.el.leader(r, C.surveyed());
        yours[r] = C.el.shown(r, C.surveyed())[YOU];
    }
    mapHover = ui::blocked ? -1 : gfx::mapHit(M, ui::mouse());
    gfx::drawMap(M, lead, yours, mapHover, -1, C.surveyed(), t);
    if (mapHover >= 0) {
        const RegionDef& R = REGIONS[mapHover];
        Vector2 m = ui::mouse();
        Rectangle tip{m.x + 16, m.y + 10, 270, 92};
        if (tip.x + tip.width > 590) tip.x = m.x - 290;
        ui::panel(tip, Color{20, 16, 28, 240});
        ui::text(R.name, tip.x + 10, tip.y + 8, 16, Pal::text, true);
        ui::text(R.sub, tip.x + 10, tip.y + 30, 13, Pal::muted);
        ui::text(strf("%s voters " DOT " booth teams %d/3", fmtIN(R.voters).c_str(), C.booth[mapHover]), tip.x + 10, tip.y + 50, 13, Pal::muted);
        ui::text("Click to travel there (1 hour)", tip.x + 10, tip.y + 68, 13, Pal::accent, true);
        if (ui::mousePressed()) {
            int r = mapHover;
            if (C.lateFor(60)) info("Too late", "It's too late to travel", "Your driver has gone home. End the day and start fresh tomorrow.");
            else {
                Dialog d;
                d.kicker = "Travel";
                d.title = std::string("Go to ") + R.name + "?";
                std::string issues;
                for (int k = 0; k < NISSUE; ++k)
                    if (R.issues[k] >= 2) issues += (issues.empty() ? "" : ", ") + std::string(ISSUES[k].label);
                d.body = std::string(R.sub) + ". " + fmtIN(R.voters) + " voters. What matters here: " + issues + ".\n\nThe drive takes 1 hour. On the street you can talk to people, hold a rally or a roadshow.";
                d.opts.push_back({"Go now", "", true, [this, r] {
                                      C.travel(r);
                                      enterRegion(r);
                                      return std::string();
                                  },
                                  true});
                d.opts.push_back({"Stay at the office", "", true, nullptr});
                push(d);
            }
        }
    }
    // seat bars
    Shares ov = C.el.overall(C.surveyed());
    ui::text("Whole seat, projected", 24, 530, 16, Pal::muted, true);
    for (int k = 0; k < NCAND; ++k) {
        float y = 556 + k * 30.f;
        gfx::drawSymbol(gfx::symbolOf(k, C.symbol), {36, y + 9}, 20, false, WHITE);
        ui::text(k == YOU ? C.name : RIVALS[k].name, 54, y, 15, Pal::text, true);
        ui::bar({220, y + 4, 300, 12}, ov[k] / 60.f, candColor(k));
        ui::textRight(strf("%s%.0f%%", C.surveyed() ? "" : "~", ov[k]), 584, y, 15, Pal::text, true);
    }

    // office actions
    float ax = 610, ay = 66;
    ui::text("Campaign office", ax, ay, 20, Pal::text, true);
    ui::text("Now: " + nowDoing, ax, ay + 26, 14, Pal::muted);
    ay += 54;
    bool sil = C.silence();
    struct A {
        std::string label, sub;
        bool ok;
        std::function<void()> go;
    };
    auto late = [&](float m) { return C.lateFor(m); };
    std::vector<A> acts;
    acts.push_back({"Manifesto promise", "1 hour " DOT " wins votes where the issue matters",
                    !sil && C.nPromises < NISSUE && !late(60), [this] {
                        Dialog d;
                        d.kicker = "Manifesto";
                        d.title = "Make a promise";
                        bool over = C.nPromises >= C.freePromises();
                        d.body = over ? "Voters are getting sceptical: every new promise now costs 3 reputation."
                                      : strf("Your first %d promises are believed. After that, each costs reputation.", C.freePromises());
                        for (int k = 0; k < NISSUE; ++k) {
                            if (C.promised[k]) continue;
                            std::string where;
                            for (int r = 0; r < NREGION; ++r)
                                if (REGIONS[r].issues[k] >= 3) where += (where.empty() ? "" : ", ") + std::string(REGIONS[r].name);
                            d.opts.push_back({ISSUES[k].promise, std::string(ISSUES[k].label) + " " DOT " matters most in " + (where.empty() ? "scattered pockets" : where), true,
                                              [this, k] {
                                                  nowDoing = "Writing the manifesto";
                                                  return C.promise(k);
                                              }});
                        }
                        d.opts.push_back({"Back", "", true, nullptr});
                        push(d);
                    }});
    acts.push_back({"Seek an endorsement", "2 hours " DOT " unions and associations", !sil && C.nEndorse < NGROUP && !late(120), [this] {
                        Dialog d;
                        d.kicker = "Endorsements";
                        d.title = "Which group will you meet?";
                        d.body = "Each group delivers its community across the seat. They want a promise on their issue; without it they expect you to sponsor their annual function (" RUPEE "5L and some reputation).";
                        for (int g = 0; g < NGROUP; ++g) {
                            if (C.endorsed[g]) continue;
                            bool ok = C.promised[GROUPS[g].wants];
                            d.opts.push_back({GROUPS[g].name,
                                              std::string(BLOC_NAMES[GROUPS[g].bloc]) + " " DOT " want a promise on " + ISSUES[GROUPS[g].wants].label + (ok ? ": done, free" : ": not made, " RUPEE "5L"),
                                              ok || C.funds >= 5, [this, g] {
                                                  nowDoing = std::string("Meeting the ") + GROUPS[g].name;
                                                  return C.endorse(g);
                                              }});
                        }
                        d.opts.push_back({"Back", "", true, nullptr});
                        push(d);
                    }});
    acts.push_back({"Press conference", "1.5 hours " DOT " " RUPEE "0.5L " DOT " attack a rival", !sil && C.funds >= .5f && !late(90), [this] {
                        Dialog d;
                        d.kicker = "Press conference";
                        d.title = "Who do you go after?";
                        d.body = "A good attack lowers a rival everywhere and builds hype. Push too hard and you may get a defamation notice.";
                        for (int k = PD; k <= IND; ++k)
                            d.opts.push_back({std::string(RIVALS[k].name) + " (" + RIVALS[k].party + ")", RIVALS[k].record, true, [this, k] {
                                                  nowDoing = "Facing the cameras at a press conference";
                                                  return C.press(k);
                                              }});
                        d.opts.push_back({"Back", "", true, nullptr});
                        push(d);
                    }});
    acts.push_back({"Social media blitz", "2 hours " DOT " " RUPEE "3L " DOT " reels and WhatsApp, seat-wide", !sil && C.funds >= 3 && !late(120), [this] {
                        nowDoing = "Shooting reels with the social media team";
                        runAction([this] { return C.social(); });
                    }});
    acts.push_back({"Raise funds", "2 hours " DOT " small donors or the builder lobby", !late(120), [this] {
                        Dialog d;
                        d.kicker = "Fundraising";
                        d.title = "Raise funds";
                        d.body = "You have " + fmtL(C.funds) + ". Declared spending so far is " + fmtL(C.spent) + " of the " + fmtL(EC_LIMIT) + " Election Commission limit.";
                        d.opts.push_back({"Small donors: +" + fmtL(C.smallDonation()), "More if your reputation is high", true, [this] {
                                              nowDoing = "Calling small donors";
                                              return C.fundSmall();
                                          }});
                        d.opts.push_back({"Builder lobby: +" RUPEE "12L", "Reputation -5, and he will want a favour", true, [this] {
                                              nowDoing = "Dinner with a builder";
                                              return C.fundBuilder();
                                          }});
                        d.opts.push_back({"Back", "", true, nullptr});
                        push(d);
                    }});
    acts.push_back({"Internal survey", "1 hour " DOT " " RUPEE "2L " DOT " exact numbers for 3 days", !C.surveyed() && C.funds >= 2 && !late(60), [this] {
                        nowDoing = "Reading survey data";
                        runAction([this] { return C.survey(); });
                    }});
    acts.push_back({"Rest", "2 hours " DOT " +30 stamina", !late(120), [this] {
                        nowDoing = "Resting at the office";
                        runAction([this] { return C.rest(); });
                    }});
    for (auto& a : acts) {
        if (ui::button({ax, ay, 330, 52}, a.label, a.ok, false, a.sub)) a.go();
        ay += 58;
    }
    if (ui::button({ax, ay + 6, 330, 46}, C.day == Campaign::maxDay ? "End the day: polling tomorrow" : "End the day and sleep", true, true)) {
        C.doing("Called it a day");
        endDayNow();
    }
    // today's schedule
    ay += 66;
    ui::text("Today's schedule", ax, ay, 15, Pal::muted, true);
    ay += 22;
    int start = std::max(0, (int)C.diary.size() - 4);
    for (int i = start; i < (int)C.diary.size(); ++i) {
        ui::text(C.diary[i].time, ax, ay, 13, Pal::accent, true);
        ui::text(C.diary[i].text, ax + 70, ay, 13, Pal::text);
        ay += 18;
    }
    if (showFeed) drawFeed({960, 64, 306, 622});
    drawTicker();
    if (dlgs.empty()) afterAction();
}

void Game::endDayNow() {
    if (sc == SC_STREET) leaveRegion(false);
    sc = SC_NIGHT;
    smode = SM_WALK;
    C.location = -1;
}

// ---------------------------------------------------------------- night summary
void Game::nightScreen() {
    gfx::drawSky(1320, t);
    gfx::drawFar(C.location >= 0 ? C.location : OLD, 0, 1320, t);
    DrawRectangle(0, 0, 1280, 720, Fade(BLACK, .45f));
    Rectangle P{110, 50, 1060, 620};
    ui::panel(P, Color{26, 21, 38, 245});
    ui::text(strf("NIGHT OF DAY %d", C.day), P.x + 30, P.y + 24, 15, Pal::marigold, true);
    ui::text(strf(C.minute < DAY_END - 90 ? "You call it a day at %s" : "You get home at %s", clockStr(C.minute).c_str()), P.x + 30, P.y + 44, 34, Pal::text, true);
    // diary
    ui::text("What you did today", P.x + 30, P.y + 104, 17, Pal::muted, true);
    float y = P.y + 132;
    if (C.diary.empty()) ui::text("Not much. The team noticed.", P.x + 30, y, 16, Pal::text);
    int start = std::max(0, (int)C.diary.size() - 14);
    for (int i = start; i < (int)C.diary.size(); ++i) {
        ui::text(C.diary[i].time, P.x + 30, y, 15, Pal::accent, true);
        ui::text(C.diary[i].text, P.x + 116, y, 15, Pal::text);
        y += 24;
    }
    // how it went
    float rx = P.x + 560;
    ui::text("How it went", rx, P.y + 104, 17, Pal::muted, true);
    float now = C.yourShare(), delta = now - C.dayStartShare;
    std::string verdict;
    if (C.surveyed()) verdict = strf("Survey: you are at %.1f%% (%s%.1f today).", now, delta >= 0 ? "+" : "", delta);
    else {
        float felt = delta + frand(-.4f, .4f);
        verdict = felt > .6f ? "Your team feels a wave building. Calls are coming in." : felt > 0 ? "Your team thinks you gained a little ground today." : "Your team is worried. Today didn't move the needle.";
    }
    float yy = P.y + 132;
    yy += ui::textWrapped(verdict, {rx, yy, 470, 100}, 17, Pal::text) + 10;
    ui::text(strf("Stamina %.0f  " DOT "  Hype %.0f  " DOT "  Reputation %.0f", C.stamina, C.hype, C.rep), rx, yy, 15, Pal::muted, true);
    yy += 34;
    ui::text("Tonight online", rx, yy, 17, Pal::muted, true);
    yy += 26;
    std::vector<const Post*> today;
    for (auto& p : C.feed)
        if (p.day == C.day) today.push_back(&p);
    std::sort(today.begin(), today.end(), [](auto a, auto b) { return a->likes > b->likes; });
    if (today.empty()) ui::text("Nobody posted about you today.", rx, yy, 15, Pal::text);
    for (size_t i = 0; i < today.size() && i < 4; ++i) {
        Color hc = today[i]->tone == T_GOOD ? Pal::good : today[i]->tone == T_BAD ? Pal::bad : Pal::accent;
        ui::text(today[i]->handle + "  " DOT "  " + fmtIN(today[i]->likes) + " likes", rx, yy, 14, hc, true);
        yy += 20;
        yy += ui::textWrapped(today[i]->text, {rx, yy, 470, 80}, 14, Pal::text, false, 1.25f) + 8;
    }
    bool last = C.day == Campaign::maxDay;
    if (ui::button({P.x + P.width - 280, P.y + P.height - 66, 250, 46}, last ? "Sleep. Polling day tomorrow" : "Sleep", true, true, "", KEY_ENTER)) {
        if (last) {
            C.computePolling();
            C.votedFor = -1;
            evmT = 99;
            sc = SC_POLL;
        } else {
            C.endDay();
            sc = SC_MORNING;
            sfx::play(sfx::CHIME, .5f);
        }
    }
}

}  // namespace seat
