// The street: walking an area with your volunteers, meeting citizens, hecklers, rallies and roadshows.
#include "Game.hpp"

namespace seat {

using gfx::STREET_W;

static float depthScale(float y) { return .92f + (y - 570.f) / 100.f * .22f; }
static std::string firstName(const std::string& n) { return n.substr(0, n.find(' ')); }

void Game::enterRegion(int r) {
    region = r;
    sc = SC_STREET;
    smode = SM_WALK;
    if (!generated[r]) {
        people[r] = generateCitizens(r, C, 26, STREET_W);
        generated[r] = true;
    }
    // a few rival supporters are in the mood to heckle today
    std::vector<int> cands;
    for (int i = 0; i < (int)people[r].size(); ++i) {
        Citizen& z = people[r][i];
        z.heckler = false;
        if (z.favourite() != YOU && z.support < -.2f && !z.hecklerDone && !z.convinced) cands.push_back(i);
    }
    std::shuffle(cands.begin(), cands.end(), rng().g);
    int nh = std::min((int)cands.size(), irand(0, C.diff + 1));
    for (int i = 0; i < nh; ++i) {
        Citizen& h = people[r][cands[i]];
        h.heckler = true;
        h.x = 320 + frand(450, 800) + i * 600;  // waiting a short walk ahead of where you arrive
    }
    px = 320;
    py = 625;
    pfacing = 1;
    camX = 0;
    for (int i = 0; i < 2; ++i) {
        workers[i].look = workerLook(YOU);
        workers[i].x = px - 70 - i * 55;
        workers[i].y = py + (i ? 26 : -22);
    }
    vehicles.clear();
    parts.clear();
    floats.clear();
    talksAtEnter = C.stats.talks + C.stats.greets;
    nowDoing = std::string("Walking through ") + REGIONS[r].name;
    pbubble = "Namaste, " + std::string(REGIONS[r].name) + "!";
    pbubbleT = 2.5f;
    sfx::play(sfx::HORN, .35f);
}

void Game::leaveRegion(bool toOffice) {
    int met = C.stats.talks + C.stats.greets - talksAtEnter;
    if (met > 0) C.doing(strf("Met %d people in %s", met, REGIONS[region].name));
    talksAtEnter = C.stats.talks + C.stats.greets;
    smode = SM_WALK;
    if (toOffice) {
        C.spendTime(60, 2);
        C.location = -1;
        C.doing("Drove back to the campaign office");
        nowDoing = "Back at the campaign office";
        sc = SC_OFFICE;
    }
}

void Game::floatText(const std::string& s, Vector2 p, Color c) { floats.push_back({s, p, 1.8f, c}); }

void Game::petals(Vector2 at, int n) {
    for (int i = 0; i < n; ++i) {
        Color c = chance(.5f) ? Color{250, 150, 20, 255} : (chance(.5f) ? Color{250, 200, 40, 255} : Color{240, 90, 120, 255});
        parts.push_back({{at.x + frand(-60, 60), at.y + frand(-20, 10)}, {frand(-60, 60), frand(-160, -40)}, c, frand(1.2f, 2.4f), frand(2.5f, 4.5f)});
    }
}

void Game::contagion(int idx, float base) {
    auto& P = people[region];
    for (int j = 0; j < (int)P.size(); ++j) {
        if (j == idx || std::fabs(P[j].x - P[idx].x) > 260) continue;
        Citizen& w = P[j];
        if (base > .8f && chance(.5f)) {
            w.react(EM_CLAP, PO_CLAP, 2);
            w.support = clampf(w.support + .03f, -1, 1);
        } else if (base < 0 && chance(.4f)) {
            w.react(EM_ANGRY, PO_CROSSED, 2);
            w.support = clampf(w.support - .02f, -1, 1);
        }
    }
}

// ---------------------------------------------------------------- conversations
void Game::openTalk(int idx) {
    Citizen& z = people[region][idx];
    z.facing = px > z.x ? 1 : -1;
    pfacing = z.x > px ? 1 : -1;
    ppose = PO_TALK;
    pposeT = 99;
    std::string op = opener(z, C);
    nowDoing = "Listening to " + z.name + ", " + z.job;
    Dialog d;
    d.kicker = (z.influencer ? std::string("Local influencer " DOT " ") : std::string()) + BLOC_NAMES[z.bloc] + " " DOT " " + REGIONS[region].name;
    d.title = z.name + " says:";
    d.body = "\"" + op + "\"";
    d.citizen = idx;
    for (const TalkOption& o : talkOptions(z, C)) {
        int id = o.id;
        d.opts.push_back({o.label, o.sub + "  " DOT "  20 min", true, [this, idx, id] {
                              Citizen& z2 = people[region][idx];
                              Outcome out = resolveTalk(z2, C, id);
                              applyOutcome(z2, C, out);
                              contagion(idx, out.base);
                              ppose = id == TK_SELFIE ? PO_SELFIE : (out.base >= .5f ? PO_NAMASTE : PO_STAND);
                              pposeT = 1.8f;
                              nowDoing = "Talked to " + z2.name + ", " + z2.job;
                              Vector2 head{z2.x, z2.y - 105};
                              if (out.base > .05f) floatText("+ support", head, Pal::good), sfx::play(sfx::CHIME, .25f);
                              else if (out.base < -.05f) floatText("- support", head, Pal::bad);
                              else floatText("no change", head, Pal::muted);
                              if (z2.influencer && z2.convinced) petals({z2.x, z2.y - 90}, 20);
                              return std::string();
                          }});
    }
    d.opts.push_back({"Say goodbye", "", true, [this] {
                          ppose = PO_NAMASTE;
                          pposeT = 1;
                          return std::string();
                      }});
    push(d);
}

void Game::openHeckle(int idx) {
    Citizen& z = people[region][idx];
    int fav = z.favourite() == YOU ? PD : z.favourite();
    std::string line = pick(RIVALS[fav].heckles);
    z.say(line, 4, Color{255, 220, 215, 255});
    z.react(EM_ANGRY, PO_TALK, 4);
    z.facing = px > z.x ? 1 : -1;
    pfacing = z.x > px ? 1 : -1;
    sfx::play(sfx::BOO, .35f);
    nowDoing = "Being heckled by a " + std::string(RIVALS[fav].shortName) + " supporter";
    // nearby people stop to watch
    for (auto& w : people[region])
        if (&w != &z && std::fabs(w.x - z.x) < 300) {
            w.idle = 6;
            w.facing = z.x > w.x ? 1 : -1;
        }
    Dialog d;
    d.kicker = std::string("Heckler " DOT " ") + RIVALS[fav].shortName + " supporter " DOT " phones are out";
    d.title = z.name + " is shouting at you";
    d.body = "\"" + line + "\"";
    d.citizen = idx;
    d.accent = Pal::bad;
    for (const TalkOption& o : heckleOptions()) {
        int id = o.id;
        d.opts.push_back({o.label, o.sub, true, [this, idx, id] {
                              Citizen& z2 = people[region][idx];
                              Outcome out = resolveHeckle(z2, C, id);
                              applyOutcome(z2, C, out);
                              contagion(idx, out.base);
                              z2.heckler = false;
                              ppose = id == HK_CALM ? PO_NAMASTE : id == HK_SHOUT ? PO_TALK : id == HK_CHAI ? PO_WAVE : PO_STAND;
                              pposeT = 2;
                              if (id == HK_SHOUT) {
                                  pbubble = "Who paid you to come here?";
                                  pbubbleT = 3;
                              } else if (id == HK_CALM) {
                                  pbubble = "Let me answer you with facts, bhai.";
                                  pbubbleT = 3;
                              } else if (id == HK_CHAI) {
                                  pbubble = "Come, have a cup of chai with me.";
                                  pbubbleT = 3;
                              }
                              C.doing("Faced a heckler in " + std::string(REGIONS[region].name));
                              nowDoing = "Walking on after a heckler";
                              floatText(out.base > .05f ? "+ support" : "- support", {z2.x, z2.y - 105}, out.base > .05f ? Pal::good : Pal::bad);
                              return std::string();
                          }});
    }
    push(d);
}

// ---------------------------------------------------------------- simulation
void Game::updateWorld() {
    auto& P = people[region];
    bool input = dlgs.empty() && !showHelp;
    float vx = camX;
    // citizens
    for (int i = 0; i < (int)P.size(); ++i) {
        Citizen& z = P[i];
        z.bubbleT -= dt;
        z.emoteT -= dt;
        z.poseT -= dt;
        if (z.poseT <= 0) z.pose = PO_STAND;
        if (!input) continue;
        float dx = px - z.x, dist = std::fabs(dx);
        bool walking = false;
        if (smode == SM_WALK && z.heckler && !z.hecklerDone && dist < 1300) {
            z.facing = dx > 0 ? 1 : -1;
            if (dist > 95) {
                z.x += z.facing * 75 * dt;
                z.y += clampf(py - z.y, -30 * dt, 30 * dt);
                walking = true;
            } else {
                openHeckle(i);
                return;
            }
        } else if (smode == SM_WALK && dist < 190 && std::fabs(py - z.y) < 120) {
            z.facing = dx > 0 ? 1 : -1;
            if (z.poseT <= 0 && z.bubbleT <= 0 && chance(dt * .25f)) {
                if (z.support > .45f) {
                    z.react(EM_HAPPY, PO_WAVE, 2.5f);
                    z.say(pick(std::vector<std::string>{"We're with you!", "Best of luck!", "Come to our lane too!", firstName(C.name) + " zindabad!"}), 2.5f);
                } else if (z.support < -.5f) {
                    z.react(EM_ANGRY, PO_CROSSED, 2.5f);
                    z.say(pick(std::vector<std::string>{"Election-time drama.", "Where were you for five years?", "Go back!"}), 2.5f, Color{255, 225, 220, 255});
                } else if (chance(.3f)) {
                    z.react(EM_THINK, PO_STAND, 2);
                }
            }
        } else if (z.idle > 0) {
            z.idle -= dt;
        } else {
            float dir = z.targetX > z.x ? 1.f : -1.f;
            z.x += dir * z.speed * dt;
            z.facing = (int)dir;
            walking = true;
            if (std::fabs(z.targetX - z.x) < 6) {
                z.idle = frand(1, 7);
                z.targetX = frand(100, STREET_W - 100);
            }
        }
        if (walking) z.phase += dt * 7;
        else z.phase = 0;
    }
    // volunteers follow and chant
    for (int i = 0; i < 2; ++i) {
        Worker& w = workers[i];
        float tx = smode == SM_RALLY ? rally.stageX + (i ? 290 : -290) : px - pfacing * (70 + i * 55);
        float ty = smode == SM_RALLY ? 575 : py + (i ? 26 : -22);
        if (smode == SM_ROADSHOW) tx = road.x - 120 - i * 50, ty = 668 - i * 8;
        float d = tx - w.x;
        float sp = std::min(std::fabs(d), (smode == SM_ROADSHOW ? 340.f : 230.f) * dt);
        w.x += (d > 0 ? 1 : -1) * sp;
        w.y += clampf(ty - w.y, -60 * dt, 60 * dt);
        if (std::fabs(d) > 4) {
            w.phase += dt * 9;
            w.facing = d > 0 ? 1 : -1;
        } else {
            w.phase = 0;
            w.facing = smode == SM_RALLY ? (i ? -1 : 1) : pfacing;
        }
        w.bubbleT -= dt;
    }
    chantT -= dt;
    if (chantT <= 0 && input) {
        Worker& w = workers[irand(0, 1)];
        std::string fn = firstName(C.name);
        w.bubble = pick(std::vector<std::string>{fn + " zindabad!", std::string("Vote for the ") + SYMBOL_NAMES[C.symbol] + "!",
                                                 "Desh ka neta kaisa ho? " + fn + " jaisa ho!", C.party + " aayi hai, nayi roshni laayi hai!"});
        w.bubbleT = 2.6f;
        chantT = frand(6, 11);
    }
    // traffic
    vehT -= dt;
    int cows = 0;
    for (auto& v : vehicles) cows += v.type == 5;
    if (vehT <= 0 && vehicles.size() < 7) {
        int type;
        if (region == GAON) type = pick(std::vector<int>{4, 4, 5, 1, 5});
        else if (region == OLD) type = pick(std::vector<int>{0, 0, 1, 1, 5, 2});
        else type = pick(std::vector<int>{0, 1, 2, 2, 3, 1});
        float dir = chance(.5f) ? 1.f : -1.f;
        if (type == 5 && cows >= 2) type = region == GAON ? 4 : 0;
        float sp = type == 5 ? 30 : type == 4 ? 60 : frand(90, 180);
        vehicles.push_back({dir > 0 ? vx - 160 : vx + 1440, sp * dir, chance(.5f) ? 528.f : 556.f, type});
        vehT = frand(2, 5.5f);
    }
    for (auto& v : vehicles) {
        v.x += v.speed * dt;
        if (!v.honked && std::fabs(v.x - px) < 160 && v.type != 5 && chance(.4f)) {
            sfx::play(sfx::HORN, .18f);
            v.honked = true;
        }
    }
    vehicles.erase(std::remove_if(vehicles.begin(), vehicles.end(), [&](const Vehicle& v) { return v.x < vx - 400 || v.x > vx + 1700; }), vehicles.end());
    // particles and floating text
    for (auto& p : parts) {
        p.p.x += p.v.x * dt;
        p.p.y += p.v.y * dt;
        p.v.y += 90 * dt;
        p.v.x *= .99f;
        p.life -= dt;
    }
    parts.erase(std::remove_if(parts.begin(), parts.end(), [](const Particle& p) { return p.life <= 0; }), parts.end());
    for (auto& f : floats) {
        f.p.y -= 28 * dt;
        f.life -= dt;
    }
    floats.erase(std::remove_if(floats.begin(), floats.end(), [](const FloatText& f) { return f.life <= 0; }), floats.end());
    pbubbleT -= dt;
    if (pposeT < 50) pposeT -= dt;
    if (pposeT <= 0) ppose = PO_STAND;
}

void Game::updateStreet() {
    if (!dlgs.empty() || showHelp) return;
    float mv = (float)((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) - (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)));
    float mvy = (float)((IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) - (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)));
    bool run = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    float sp = run ? 280.f : 165.f;
    px = clampf(px + mv * sp * dt, 80, STREET_W - 80);
    py = clampf(py + mvy * 95 * dt, 572, 664);
    bool moving = mv != 0 || mvy != 0;
    if (mv != 0) pfacing = mv > 0 ? 1 : -1;
    if (moving) {
        pphase += dt * (run ? 13.f : 9.f);
        C.spendTime(dt * (run ? 3.5f : 2.5f), run ? 7.f : 3.f);
        if (pposeT > 50) pposeT = 0;
        if (nowDoing.rfind("Walking", 0) != 0) nowDoing = std::string("Walking through ") + REGIONS[region].name;
    } else {
        pphase = 0;
        C.spendTime(dt * 1.2f, 1.f);
    }
    // nearest person
    nearIdx = -1;
    float best = 1e9f;
    auto& P = people[region];
    for (int i = 0; i < (int)P.size(); ++i) {
        float dx = std::fabs(P[i].x - px), dy = std::fabs(P[i].y - py);
        if (dx < 75 && dy < 50 && dx + dy < best) {
            best = dx + dy;
            nearIdx = i;
        }
    }
    if (nearIdx >= 0) {
        if (IsKeyPressed(KEY_E)) openTalk(nearIdx);
        else if (IsKeyPressed(KEY_F)) {
            Citizen& z = P[nearIdx];
            pfacing = z.x > px ? 1 : -1;
            z.facing = px > z.x ? 1 : -1;
            Outcome o = greet(z, C);
            applyOutcome(z, C, o);
            ppose = PO_NAMASTE;
            pposeT = 1.2f;
            pbubble = pick(std::vector<std::string>{"Namaste!", "Namaste ji, vote zaroor dena!", "Aapka aashirwad chahiye."});
            pbubbleT = 1.8f;
            nowDoing = "Greeting " + z.name;
        }
    }
}

// ---------------------------------------------------------------- rally
void Game::startRally(bool bussed) {
    rally = RallyState{};
    rally.before = snap();
    rally.bussed = bussed;
    C.spend(bussed ? 8.f : 6.f);
    rally.stageX = clampf(px, 460, STREET_W - 460);
    rally.crowdN = C.crowdSize(region, bussed);
    int n = (int)clampf(rally.crowdN / 40.f, 45, 160);
    for (int i = 0; i < n; ++i) {
        CrowdMember m;
        float u = frand(0, 1);
        Bloc b = YOUTH;
        float acc = 0;
        for (int k = 0; k < NBLOC; ++k)
            if (u <= (acc += REGIONS[region].blocs[k])) {
                b = (Bloc)k;
                break;
            }
        m.look = randomLook(b == WOMEN || chance(.3f), irand(18, 70), b);
        m.p = {rally.stageX + frand(-430, 430), frand(578, 692)};
        m.s = .85f + (m.p.y - 578) / 114.f * .35f;
        m.phase = frand(0, 6);
        rally.crowd.push_back(m);
    }
    rally.opts = C.speechOptions(region, 0);
    smode = SM_RALLY;
    px = rally.stageX - 40;
    py = 498;
    pfacing = 1;
    nowDoing = std::string("Addressing a rally in ") + REGIONS[region].name;
    pbubble = "Bhaiyon aur behno!";
    pbubbleT = 2.5f;
    ppose = PO_WAVE;
    pposeT = 2.5f;
    rally.reactT = 2.5f;
    sfx::play(sfx::CHEER, .55f);
    petals({rally.stageX, 470}, 30);
}

void Game::updateRally() {
    float target = clampf(rally.stageX - 640, 0, STREET_W - 1280);
    camX += (target - camX) * std::min(1.f, dt * 4);
    rally.reactT -= dt;
    for (auto& m : rally.crowd) {
        m.poseT -= dt;
        m.bubbleT -= dt;
        if (m.poseT <= 0) m.pose = PO_STAND;
    }
    if (rally.round >= rally.rounds && rally.reactT <= 0 && !rally.finished) finishRally();
}

void Game::finishRally() {
    rally.finished = true;
    float avg = rally.sum / rally.rounds;
    float q = clampf(.6f + avg * .8f, .2f, 1.4f);
    C.spendTime(180, 5);
    std::string msg = C.rallyFinish(region, q, rally.bussed, rally.crowdN);
    C.doing(std::string("Rally in ") + REGIONS[region].name);
    smode = SM_WALK;
    py = 625;
    ppose = PO_STAND;
    nowDoing = std::string("Walking through ") + REGIONS[region].name;
    dlgs.push_front(resultDialog(msg, rally.before));
    afterAction();
}

void Game::drawRallyUi() {
    Rectangle R{630, 64, 634, 214};
    DrawRectangleRounded(R, .05f, 8, Fade(Color{22, 18, 32, 255}, .93f));
    ui::text(strf("RALLY " DOT " %s " DOT " crowd about %s", REGIONS[region].name, fmtIN(rally.crowdN).c_str()), R.x + 16, R.y + 12, 14, Pal::marigold, true);
    ui::text("Crowd mood", R.x + 470, R.y + 12, 14, Pal::muted, true);
    Color mc = rally.meter > .6f ? Pal::good : rally.meter > .4f ? Pal::marigold : Pal::bad;
    ui::bar({R.x + 560, R.y + 16, 100, 10}, rally.meter, mc);
    if (rally.reactT > 0 || rally.round >= rally.rounds) {
        std::string verdict = rally.round == 0 ? "The crowd settles. Garlands go around your neck."
                              : rally.last > .45f ? "The crowd roars. Phones go up, slogans go out."
                              : rally.last > .1f  ? "Polite applause, and some nodding."
                                                  : "Murmurs. Someone at the back shouts \"Jhooth!\"";
        ui::textWrapped(rally.round > 0 ? "\"" + rally.line + "\"" : std::string("You climb onto the stage."), {R.x + 16, R.y + 44, R.width - 32, 100}, 19, Pal::text, true);
        ui::text(verdict, R.x + 16, R.y + 150, 17, mc, true);
        return;
    }
    ui::text(strf("Line %d of %d: what do you say next?", rally.round + 1, rally.rounds), R.x + 16, R.y + 36, 17, Pal::text, true);
    float y = R.y + 64;
    for (size_t i = 0; i < rally.opts.size(); ++i) {
        const SpeechLine& L = rally.opts[i];
        if (ui::button({R.x + 16, y, R.width - 32, 44}, strf("%d   ", (int)i + 1) + L.text, true, false, "", KEY_ONE + (int)i)) {
            float rv = C.speechReaction(region, L, rally.usedKinds);
            C.speechSideEffects(region, L);
            if (L.kind == SP_ISSUE) rally.usedMask |= 1u << L.arg;
            else if (L.kind == SP_ATTACK) rally.usedMask |= 1u << (8 + L.arg);
            else rally.usedMask |= 1u << (L.kind == SP_EMOTION ? 12 : L.kind == SP_FREEBIE ? 13 : 14);
            rally.usedKinds |= 1u << L.kind;
            rally.sum += rv;
            rally.last = rv;
            rally.round++;
            rally.reactT = 3.0f;
            rally.line = L.text;
            rally.meter = clampf(.5f + rally.sum / rally.round * .6f, 0, 1);
            pbubble = L.text;
            pbubbleT = 3;
            ppose = PO_TALK;
            pposeT = 3;
            std::string fn = firstName(C.name);
            for (auto& m : rally.crowd) {
                if (rv > .45f && chance(.35f + rv * .5f)) {
                    m.pose = PO_CHEER;
                    m.poseT = frand(1.5f, 3);
                    if (chance(.08f)) m.bubble = pick(std::vector<std::string>{"Zindabad!", "Sahi baat!", "Wah!", fn + " zindabad!"}), m.bubbleT = 2.4f;
                } else if (rv > .1f && chance(.5f)) {
                    m.pose = PO_CLAP;
                    m.poseT = 2;
                    if (chance(.03f)) m.bubble = "Theek hai.", m.bubbleT = 2;
                } else if (rv <= .1f && chance(.3f)) {
                    m.pose = PO_CROSSED;
                    m.poseT = 2.5f;
                    if (chance(.07f)) m.bubble = pick(std::vector<std::string>{"Jhooth!", "Bas karo!", "Boooo!"}), m.bubbleT = 2.4f;
                }
            }
            if (rv > .45f) {
                sfx::play(sfx::CHEER, .7f);
                petals({rally.stageX, 450}, 40);
            } else if (rv > .1f) sfx::play(sfx::CLAP, .6f);
            else sfx::play(sfx::BOO, .6f);
            if (rally.round < rally.rounds) rally.opts = C.speechOptions(region, rally.usedMask);
            return;
        }
        y += 50;
    }
}

// ---------------------------------------------------------------- roadshow
void Game::startRoadshow() {
    road = RoadshowState{};
    road.before = snap();
    road.x = std::max(camX - 150, 0.f);
    smode = SM_ROADSHOW;
    nowDoing = std::string("Roadshow through ") + REGIONS[region].name;
    sfx::play(sfx::HORN, .4f);
}

void Game::updateRoadshow() {
    road.x += 330 * dt;
    road.timer += dt;
    road.petalT -= dt;
    px = road.x;
    float target = clampf(road.x - 520, 0, STREET_W - 1280);
    camX += (target - camX) * std::min(1.f, dt * 5);
    if (road.petalT <= 0) {
        petals({road.x + frand(-200, 200), 420}, 8);
        road.petalT = .25f;
    }
    for (auto& z : people[region]) {
        float d = std::fabs(z.x - road.x);
        if (d < 220 && z.poseT <= 0) {
            z.facing = road.x > z.x ? 1 : -1;
            if (z.support > .1f) {
                z.react(EM_HAPPY, z.support > .4f ? PO_CHEER : PO_WAVE, 2.5f);
                if (chance(.2f)) z.say(pick(std::vector<std::string>{"Zindabad!", "Over here!", "Best of luck!"}), 2);
            } else if (z.support < -.4f) z.react(EM_ANGRY, PO_CROSSED, 2.5f);
            else z.react(EM_THINK, PO_STAND, 2.5f);
        }
    }
    if (road.x > STREET_W + 100 || road.timer > 11) {
        std::string msg = C.roadshow(region);
        smode = SM_WALK;
        px = clampf(road.x, 100, STREET_W - 100);
        py = 625;
        nowDoing = std::string("Walking through ") + REGIONS[region].name;
        dlgs.push_front(resultDialog(msg, road.before));
        afterAction();
    }
}

// ---------------------------------------------------------------- street actions bar
void Game::streetActions() {
    float x = 16, y = 128, w = 230, h = 46;
    bool sil = C.silence();
    const char* nm = REGIONS[region].name;
    if (ui::button({x, y, w, h}, "Rally  [R]", !sil && C.funds >= 6 && !C.lateFor(180), false, "3 hrs " DOT " " RUPEE "6L", KEY_R)) {
        Dialog d;
        d.kicker = std::string("Rally " DOT " ") + nm;
        d.title = "Hold a jansabha here?";
        d.body = strf("Your team estimates a crowd of about %s. Hiring buses brings in people from the villages around, for a bigger crowd.", fmtIN(C.crowdSize(region, false)).c_str());
        if (C.rain) d.body += "\n\nIt's raining today. Expect a thin crowd.";
        d.opts.push_back({"Standard rally", RUPEE "6L " DOT " 3 hours", true, [this] {
                              startRally(false);
                              return std::string();
                          },
                          true});
        d.opts.push_back({"Rally with hired buses", RUPEE "8L " DOT " 3 hours " DOT " bigger crowd", C.funds >= 8, [this] {
                              startRally(true);
                              return std::string();
                          }});
        d.opts.push_back({"Not now", "", true, nullptr});
        push(d);
    }
    y += h + 6;
    if (ui::button({x, y, w, h}, "Roadshow  [T]", !sil && C.funds >= 4 && !C.lateFor(120), false, "2 hrs " DOT " " RUPEE "4L", KEY_T)) startRoadshow();
    y += h + 6;
    if (ui::button({x, y, w, h}, "Booth agents  [B]", C.booth[region] < 3 && C.funds >= 2 && !C.lateFor(60), false, strf("1 hr " DOT " " RUPEE "2L " DOT " %d/3 here", C.booth[region]), KEY_B)) {
        nowDoing = std::string("Training booth agents in ") + nm;
        runAction([this] { return C.trainBooth(region); });
    }
    y += h + 6;
    if (ui::button({x, y, w, h}, "\"Gifts\"  [G]", C.funds >= 7 && !C.lateFor(60), false, "1 hr " DOT " " RUPEE "7L " DOT " illegal", KEY_G)) {
        Dialog d;
        d.kicker = "Off the books";
        d.title = "Distribute cash and sarees?";
        d.body = std::string("Your local fixer can quietly hand out envelopes in ") + nm + ". It buys votes, but election flying squads patrol at night. If caught, you get an Election Commission notice. Three notices and you are disqualified.";
        d.opts.push_back({"Do it", RUPEE "7L, not declared", true, [this] {
                              nowDoing = "\"Distribution\" after dark";
                              return C.gifts(region);
                          }});
        d.opts.push_back({"No. Win it clean", "", true, nullptr, true});
        push(d);
    }
    y += h + 6;
    if (ui::button({x, y, w, h}, "Back to office  [M]", true, false, "1 hr drive", KEY_M)) leaveRegion(true);
}

// ---------------------------------------------------------------- drawing
void Game::drawStreet() {
    gfx::drawSky(C.minute, t);
    gfx::drawFar(region, camX, C.minute, t);
    Camera2D cam{};
    cam.target = {camX, 0};
    cam.zoom = 1;
    BeginMode2D(cam);
    gfx::drawNear(region, camX, camX + 1280, C.minute, t, C.el.exact(region), C.symbol, C.name);
    for (auto& v : vehicles)
        if (smode != SM_RALLY || std::fabs(v.x - rally.stageX) > 330) gfx::drawVehicle(v.type, {v.x, v.y}, v.speed > 0 ? 1 : -1, t);
    if (smode == SM_RALLY) gfx::drawStage({rally.stageX, 540}, flagColor(YOU), C.symbol, C.name, C.party, t);

    struct D {
        float y;
        int kind, idx;
    };
    std::vector<D> ds;
    auto& P = people[region];
    for (int i = 0; i < (int)P.size(); ++i) {
        if (P[i].x < camX - 80 || P[i].x > camX + 1360) continue;
        if (smode == SM_RALLY && std::fabs(P[i].x - rally.stageX) < 470) continue;
        ds.push_back({P[i].y, 0, i});
    }
    for (int i = 0; i < 2; ++i) ds.push_back({workers[i].y, 1, i});
    if (smode == SM_ROADSHOW) ds.push_back({650, 4, 0});
    else ds.push_back({py, 2, 0});
    if (smode == SM_RALLY)
        for (int i = 0; i < (int)rally.crowd.size(); ++i) ds.push_back({rally.crowd[i].p.y, 3, i});
    std::sort(ds.begin(), ds.end(), [](const D& a, const D& b) { return a.y < b.y; });
    for (auto& d : ds) {
        switch (d.kind) {
            case 0: {
                Citizen& z = P[d.idx];
                float s = depthScale(z.y);
                if (d.idx == nearIdx && smode == SM_WALK && dlgs.empty()) DrawEllipse((int)z.x, (int)z.y + 1, 22 * s, 7 * s, Fade(Pal::accent, .6f));
                gfx::drawPerson(z.look, {z.x, z.y}, s, z.phase, z.phase > 0, z.facing, z.pose, t, z.mood());
            } break;
            case 1: {
                Worker& w = workers[d.idx];
                float s = depthScale(w.y);
                gfx::drawPerson(w.look, {w.x, w.y}, s, w.phase, w.phase > 0, w.facing, PO_STAND, t, 1);
                gfx::drawFlag({w.x + w.facing * 9 * s, w.y - 36 * s}, 78 * s, flagColor(YOU), C.symbol, t + d.idx);
            } break;
            case 2: {
                float s = smode == SM_RALLY ? 1.15f : depthScale(py) * 1.04f;
                drawCandidateAt({px, py}, s, ppose, pphase > 0, pphase, pfacing);
            } break;
            case 3: {
                CrowdMember& m = rally.crowd[d.idx];
                gfx::drawPerson(m.look, m.p, m.s, 0, false, 1, m.pose, t + m.phase, 1, true);
            } break;
            case 4: {
                gfx::drawJeep({road.x, 660}, t);
                drawCandidateAt({road.x - 10, 618}, 1.1f, PO_WAVE, false, 0, 1);
            } break;
        }
    }
    for (auto& p : parts) DrawCircleV(p.p, p.size, Fade(p.c, clampf(p.life, 0, 1)));
    EndMode2D();
    gfx::drawNightOverlay(C.minute);
    BeginMode2D(cam);
    gfx::drawLights(region, camX, camX + 1280, C.minute);
    EndMode2D();

    // speech bubbles, emotes, prompts (screen space)
    auto sx = [&](float x) { return x - camX; };
    for (auto& z : P) {
        if (z.x < camX - 80 || z.x > camX + 1360) continue;
        if (smode == SM_RALLY && std::fabs(z.x - rally.stageX) < 470) continue;
        float s = depthScale(z.y);
        Vector2 head{sx(z.x), z.y - 92 * s * z.look.height};
        if (z.bubbleT > 0) ui::bubble(z.bubble, head, std::min(1.f, z.bubbleT * 2), z.bubbleCol, Pal::ink, 15, 230);
        else if (z.emoteT > 0) gfx::drawEmote(z.emote, {head.x, head.y - 8}, std::min(1.f, z.emoteT * 2));
        if (z.heckler && !z.hecklerDone && smode == SM_WALK) ui::textCentered("!", head.x, head.y - 30, 22, Pal::bad, true);
    }
    if (smode == SM_RALLY)
        for (auto& m : rally.crowd)
            if (m.bubbleT > 0) ui::bubble(m.bubble, {sx(m.p.x), m.p.y - 88 * m.s}, std::min(1.f, m.bubbleT * 2), WHITE, Pal::ink, 14, 160);
    for (auto& w : workers)
        if (w.bubbleT > 0) ui::bubble(w.bubble, {sx(w.x), w.y - 95}, std::min(1.f, w.bubbleT * 2), Color{235, 225, 255, 255}, Pal::ink, 14, 230);
    if (pbubbleT > 0) {
        Vector2 a = smode == SM_ROADSHOW ? Vector2{sx(road.x - 10), 520} : Vector2{sx(px), py - 100 * (smode == SM_RALLY ? 1.15f : 1.f)};
        ui::bubble(pbubble, a, std::min(1.f, pbubbleT * 2), Color{255, 245, 210, 255}, Pal::ink, smode == SM_RALLY ? 18 : 16, smode == SM_RALLY ? 420 : 260);
    }
    for (auto& f : floats) ui::textCentered(f.s, sx(f.p.x), f.p.y, 16, Fade(f.c, clampf(f.life, 0, 1)), true);
    if (nearIdx >= 0 && smode == SM_WALK && dlgs.empty() && nearIdx < (int)P.size()) {
        Citizen& z = P[nearIdx];
        float s = depthScale(z.y);
        Rectangle card{sx(z.x) - 110, z.y - 160 * s - 26, 220, 58};
        if (z.bubbleT > 0) card.y -= 46;
        DrawRectangleRounded(card, .2f, 8, Fade(Color{22, 18, 32, 255}, .92f));
        ui::text(z.name + strf(", %d", z.age), card.x + 10, card.y + 6, 15, Pal::text, true);
        ui::textRight(z.job, card.x + card.width - 10, card.y + 8, 12, Pal::muted);
        ui::text("[E] Talk     [F] Namaste", card.x + 10, card.y + 32, 14, Pal::accent, true);
    }
}

void Game::street() {
    updateWorld();
    if (sc != SC_STREET) return;
    if (smode == SM_WALK) {
        updateStreet();
        float target = clampf(px - 640, 0, STREET_W - 1280);
        camX += (target - camX) * std::min(1.f, dt * 5);
    } else if (smode == SM_RALLY) {
        if (dlgs.empty()) updateRally();
    } else if (dlgs.empty())
        updateRoadshow();
    drawStreet();
    drawHud();
    // what the candidate is doing right now
    Shares sh = C.el.shown(region, false);
    int lead = (int)(std::max_element(sh.begin(), sh.end()) - sh.begin());
    Rectangle now{16, 64, 600, 54};
    DrawRectangleRounded(now, .2f, 8, Fade(Color{22, 18, 32, 255}, .9f));
    ui::text("NOW: " + nowDoing, now.x + 12, now.y + 7, 16, Pal::text, true);
    ui::text(strf("%s " DOT " you %s%.0f%% " DOT " %s leads here " DOT " rain: %s", REGIONS[region].name, C.surveyed() ? "" : "~", sh[YOU],
                  lead == YOU ? "you" : RIVALS[lead].shortName, C.rain ? "yes" : "no"),
             now.x + 12, now.y + 30, 14, Pal::muted);
    if (smode == SM_WALK) streetActions();
    if (smode == SM_RALLY) drawRallyUi();
    if (showFeed && smode != SM_RALLY) drawFeed({960, 64, 306, 360});
    if (smode == SM_WALK)
        ui::text("A/D walk " DOT " W/S step " DOT " Shift hurry " DOT " E talk " DOT " F namaste " DOT " H help", 16, 672, 14, Fade(WHITE, .85f), true);
    drawTicker();
    if (dlgs.empty() && smode == SM_WALK) afterAction();
}

}  // namespace seat
