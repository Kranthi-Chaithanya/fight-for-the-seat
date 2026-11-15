#include "Citizen.hpp"

namespace seat {

static const Color SKINS[] = {{141, 85, 36, 255}, {198, 134, 66, 255}, {224, 172, 105, 255}, {170, 110, 60, 255}, {120, 72, 40, 255}, {190, 125, 80, 255}, {160, 100, 55, 255}};
static const Color CLOTH[] = {{220, 70, 60, 255},  {240, 170, 40, 255}, {60, 140, 200, 255},  {90, 170, 110, 255}, {230, 120, 160, 255},
                              {150, 90, 180, 255}, {240, 230, 210, 255}, {70, 80, 110, 255},  {200, 200, 205, 255}, {120, 60, 40, 255},
                              {40, 120, 120, 255}, {250, 210, 120, 255}, {180, 40, 90, 255},   {100, 130, 60, 255}};
static Color cloth() { return CLOTH[irand(0, (int)(sizeof CLOTH / sizeof CLOTH[0]) - 1)]; }

Look randomLook(bool female, int age, Bloc bloc) {
    Look L;
    L.female = female;
    L.skin = SKINS[irand(0, 6)];
    bool old = age > 55;
    L.hair = old && chance(.6f) ? Color{200, 200, 200, 255} : (chance(.15f) ? Color{80, 50, 30, 255} : Color{25, 20, 20, 255});
    L.top = cloth();
    L.accent = cloth();
    L.bottom = Color{60, 60, 70, 255};
    L.height = frand(.92f, 1.06f) * (female ? .95f : 1.f);
    L.girth = frand(.9f, 1.2f);
    if (age < 16) L.height *= .7f;
    if (female) {
        L.hairStyle = chance(.6f) ? 1 : 3;
        L.saree = (age > 35 && chance(.6f)) || (bloc == FARMERS && chance(.5f));
        L.kurta = !L.saree;
        L.bottom = L.saree ? L.top : cloth();
        if (bloc == YOUTH && chance(.4f)) {
            L.saree = L.kurta = false;
            L.bottom = Color{50, 70, 120, 255};
        }
    } else {
        L.hairStyle = old && chance(.4f) ? 2 : 0;
        L.beard = chance(.25f);
        switch (bloc) {
            case FARMERS:
                L.top = Color{238, 234, 222, 255};
                L.bottom = Color{238, 234, 222, 255};
                L.kurta = true;
                L.turban = chance(.6f);
                break;
            case YOUTH: L.bottom = Color{50, 70, 120, 255}; break;
            case MIDDLE: L.glasses = chance(.5f); L.bottom = Color{90, 90, 100, 255}; break;
            case TRADERS: L.kurta = chance(.5f); L.bottom = Color{230, 225, 215, 255}; break;
            default: L.bottom = Color{70, 65, 60, 255}; break;
        }
    }
    if (old) L.glasses = L.glasses || chance(.4f);
    return L;
}

Look candidateLook(int cand) {
    Look L;
    L.skin = SKINS[2];
    L.hair = {25, 20, 20, 255};
    L.top = {246, 244, 238, 255};
    L.bottom = {246, 244, 238, 255};
    L.kurta = true;
    L.scarf = true;
    L.accent = flagColor(cand);
    L.height = 1.06f;
    return L;
}

Look workerLook(int cand) {
    Look L = randomLook(false, irand(20, 40), YOUTH);
    L.top = flagColor(cand);
    L.cap = true;
    L.accent = flagColor(cand);
    return L;
}

int Citizen::favourite() const { return (int)(std::max_element(lean, lean + NCAND) - lean); }

static Bloc pickBloc(int r) {
    float x = frand(0, 1), acc = 0;
    for (int b = 0; b < NBLOC; ++b) {
        acc += REGIONS[r].blocs[b];
        if (x <= acc) return (Bloc)b;
    }
    return YOUTH;
}

static Issue pickIssue(int r, Bloc b) {
    static const std::vector<Issue> pref[NBLOC] = {{FARM, WATER}, {CORRUPTION, PRICES, ROADS}, {JOBS, EDUCATION}, {JOBS, PRICES}, {SAFETY, PRICES, WATER}, {ROADS, CORRUPTION, SAFETY}};
    if (chance(.6f)) return pick(pref[b]);
    int tot = 0;
    for (int w : REGIONS[r].issues) tot += w;
    int x = irand(1, tot);
    for (int k = 0; k < NISSUE; ++k) {
        x -= REGIONS[r].issues[k];
        if (x <= 0) return (Issue)k;
    }
    return JOBS;
}

std::vector<Citizen> generateCitizens(int r, const Campaign& c, int count, float streetW) {
    std::vector<Citizen> out;
    Shares sh = c.el.exact(r);
    int infl1 = irand(0, count - 1), infl2 = (infl1 + count / 2) % count;
    for (int i = 0; i < count; ++i) {
        Citizen z;
        z.region = r;
        z.bloc = pickBloc(r);
        z.female = z.bloc == WOMEN ? true : chance(.35f);
        switch (z.bloc) {
            case YOUTH: z.age = irand(18, 28); break;
            case MIDDLE: z.age = irand(28, 64); break;
            case FARMERS: z.age = irand(24, 72); break;
            default: z.age = irand(22, 66); break;
        }
        z.name = pick(z.female ? FEMALE_NAMES : MALE_NAMES);
        z.influencer = (i == infl1 || i == infl2);
        z.influence = z.influencer ? 3.f : 1.f;
        z.job = z.influencer ? INFLUENCER_JOB[z.bloc] : pick(JOBS_BY_BLOC[z.bloc]);
        z.issue = pickIssue(r, z.bloc);
        float t = 0;
        for (int k = 0; k < NCAND; ++k) t += (z.lean[k] = sh[k] * frand(.4f, 1.6f));
        for (float& l : z.lean) l /= t;
        z.support = clampf((z.lean[YOU] - .22f) * 3.f + frand(-.25f, .25f), -1, 1);
        z.look = randomLook(z.female, z.age, z.bloc);
        z.x = frand(150, streetW - 150);
        z.targetX = frand(100, streetW - 100);
        z.y = frand(568, 662);
        z.speed = frand(22, 50) * (z.age > 60 ? .6f : 1.f);
        z.phase = frand(0, 6);
        z.idle = frand(0, 4);
        out.push_back(z);
    }
    return out;
}

std::string opener(Citizen& z, Campaign& c) {
    const IssueDef& I = ISSUES[z.issue];
    if (z.talks > 0) {
        if (z.privatePromise && !c.promised[z.issue]) {
            z.support -= .15f;
            return std::string("You promised me something on ") + I.label + " last time. It is not even in your manifesto!";
        }
        if (c.promised[z.issue]) return std::string("I saw your manifesto has ") + I.label + ". Good. Don't forget us after the election.";
        if (z.convinced) return "Arre, you again! Don't worry, my vote is with you. Go meet the others.";
        return std::string("You again! We are still waiting on ") + I.label + ".";
    }
    std::string pre;
    if (z.influencer) pre = "Half this area listens to me, so listen carefully. ";
    else if (z.support < -.4f) pre = "Oh, another neta. ";
    else if (z.support > .4f) pre = "Namaste! I've seen your posters. ";
    std::string me = z.age > 50 ? "I've lived here all my life. " : "";
    return pre + me + pick(I.complaints);
}

std::vector<TalkOption> talkOptions(const Citizen& z, const Campaign& c) {
    std::vector<TalkOption> o;
    o.push_back({"Sit down, listen and take notes", "Builds trust", TK_LISTEN});
    o.push_back({std::string("Promise to fix ") + ISSUES[z.issue].label,
                 c.promised[z.issue] ? "It's in your manifesto, so it's credible" : "Not in your manifesto: depends on your reputation", TK_PROMISE});
    o.push_back({"Blame the sitting MP", "Works unless they back Prakash", TK_ATTACK});
    o.push_back({"Ask for their vote", "Best once they already like you", TK_VOTE});
    if (z.age < 35 && !z.selfie) o.push_back({"Take a selfie together", "Young voters love it", TK_SELFIE});
    if (z.influencer && !z.convinced) o.push_back({"Ask them to campaign for you", "Needs their trust first", TK_RECRUIT});
    return o;
}

Outcome resolveTalk(Citizen& z, Campaign& c, int id) {
    Outcome o;
    float dim = 1.f / (1.f + z.talks * .8f);
    const IssueDef& I = ISSUES[z.issue];
    switch (id) {
        case TK_LISTEN:
            o.base = .9f;
            z.support += .15f;
            o.reply = pick(std::vector<std::string>{"At least someone listens.", "Write it down properly, ji.", "Okay. Let's see if you remember.",
                                                    "Nobody has sat with us like this before."});
            o.emote = EM_THINK;
            o.pose = PO_TALK;
            o.feedCtx = "talk";
            break;
        case TK_PROMISE:
            if (c.promised[z.issue]) {
                o.base = 1.6f;
                z.support += .3f;
                o.reply = "It's in your manifesto? Then I'll hold you to it.";
                o.emote = EM_HAPPY;
                o.pose = PO_CLAP;
            } else if (chance(c.rep / 110.f)) {
                o.base = 1.1f;
                z.support += .2f;
                z.privatePromise = true;
                c.offPromises++;
                if (c.offPromises > 6) c.rep -= .5f;
                o.reply = std::string("A promise on ") + I.label + "... okay, I believe you. For now.";
                o.emote = EM_HAPPY;
                o.pose = PO_STAND;
            } else {
                o.base = .2f;
                z.support -= .05f;
                o.reply = "Everyone promises before elections.";
                o.emote = EM_THINK;
                o.pose = PO_CROSSED;
            }
            o.feedCtx = "talk";
            break;
        case TK_ATTACK:
            if (z.favourite() == PD) {
                o.base = -.6f;
                z.support -= .25f;
                o.reply = pick(RIVALS[PD].defend);
                o.emote = EM_ANGRY;
                o.pose = PO_CROSSED;
            } else {
                o.base = 1.f;
                c.el.hit(PD, z.region, .4f);
                z.support += .15f;
                o.reply = "Exactly! Ten years and what did we get?";
                o.emote = EM_HAPPY;
                o.pose = PO_TALK;
            }
            break;
        case TK_VOTE:
            if (z.support > .25f) {
                o.base = 1.2f;
                z.convinced = true;
                c.stats.converted++;
                o.reply = "You have my vote. My whole family's too!";
                o.emote = EM_HEART;
                o.pose = PO_CHEER;
            } else if (z.support > -.1f) {
                o.base = .5f;
                o.reply = "I'll think about it. Let's see.";
                o.emote = EM_THINK;
            } else {
                o.base = -.3f;
                z.support -= .1f;
                o.reply = "Not a chance. My vote is decided.";
                o.emote = EM_ANGRY;
                o.pose = PO_CROSSED;
            }
            break;
        case TK_SELFIE:
            z.selfie = true;
            if (z.age < 32) {
                o.base = .9f;
                c.hype += .5f;
                c.stats.selfies++;
                z.support += .1f;
                o.reply = "Posting this right now!";
                o.emote = EM_HEART;
                o.pose = PO_SELFIE;
                o.feedCtx = "selfie";
            } else {
                o.base = .1f;
                o.reply = "Beta, I don't do selfies.";
                o.emote = EM_THINK;
            }
            break;
        case TK_RECRUIT:
            if (z.support > .35f) {
                o.base = 2.4f;
                z.convinced = true;
                c.stats.converted++;
                o.reply = "Fine. I'll speak to everyone. Consider this area covered.";
                o.emote = EM_HEART;
                o.pose = PO_CHEER;
                c.post("@chandanpur_buzz", "Local " + z.job + " " + z.name + " joins " + c.name + "'s campaign in " + REGIONS[z.region].name + ".", T_GOOD);
            } else {
                o.base = 0;
                z.support -= .05f;
                o.reply = "Not so fast. Earn my trust first.";
                o.emote = EM_THINK;
                o.pose = PO_CROSSED;
            }
            break;
    }
    o.base *= dim;
    z.support = clampf(z.support, -1, 1);
    z.talks++;
    c.stats.talks++;
    c.spendTime(20, 2);
    return o;
}

Outcome greet(Citizen& z, Campaign& c) {
    Outcome o;
    z.greets++;
    c.stats.greets++;
    c.spendTime(5, 1);
    if (z.greets > 2) {
        o.base = -.05f;
        z.support -= .05f;
        o.reply = "Again? I'm busy, ji.";
        o.emote = EM_ANGRY;
        return o;
    }
    if (z.support > -.4f) {
        o.base = .25f;
        z.support += .05f;
        o.reply = pick(std::vector<std::string>{"Namaste ji!", "Ram Ram!", "Salaam!", "Sat Sri Akal!", "Best of luck!"});
        o.emote = EM_HAPPY;
        o.pose = z.support > .3f ? PO_WAVE : PO_NAMASTE;
    } else {
        o.base = 0;
        o.reply = "Hmph.";
        o.emote = EM_ANGRY;
        o.pose = PO_CROSSED;
    }
    return o;
}

std::vector<TalkOption> heckleOptions() {
    return {{"Answer calmly with facts", "The crowd is watching", HK_CALM},
            {"Shout back", "Fires up your base, risky on camera", HK_SHOUT},
            {"Offer them chai and a seat", "Might win them over", HK_CHAI},
            {"Ignore them and walk on", "", HK_IGNORE}};
}

Outcome resolveHeckle(Citizen& z, Campaign& c, int id) {
    Outcome o;
    z.hecklerDone = true;
    c.stats.hecklers++;
    c.spendTime(10, 1);
    switch (id) {
        case HK_CALM:
            o.base = .8f;
            c.rep += 1;
            z.support += .2f;
            o.reply = "Hmm. Fair enough, I suppose.";
            o.emote = EM_THINK;
            o.feedCtx = "heckler_calm";
            break;
        case HK_SHOUT:
            o.base = .3f;
            c.hype += 3;
            c.rep -= 2;
            o.reply = "See? This is the real face of " + c.name + "!";
            o.emote = EM_ANGRY;
            o.pose = PO_TALK;
            if (chance(.25f)) {
                c.rep -= 4;
                o.feedCtx = "heckler_angry";
            }
            break;
        case HK_CHAI:
            if (chance(.4f + c.rep / 250.f)) {
                o.base = 1.5f;
                z.support = .4f;
                o.reply = "...Okay. One cup. Then we talk.";
                o.emote = EM_HAPPY;
                o.pose = PO_STAND;
                o.feedCtx = "heckler_calm";
            } else {
                o.base = .1f;
                o.reply = "I don't drink chai with liars.";
                o.emote = EM_ANGRY;
                o.pose = PO_CROSSED;
            }
            break;
        default:
            o.base = -.2f;
            o.reply = "Running away already?";
            o.emote = EM_ANGRY;
            break;
    }
    z.support = clampf(z.support, -1, 1);
    return o;
}

void applyOutcome(Citizen& z, Campaign& c, const Outcome& o) {
    float pts = o.base >= 0 ? c.gain(o.base * z.influence * c.streetBonus(), -1, false) : o.base * z.influence;
    c.el.addYou(z.region, pts);
    z.say(o.reply, 4.f, o.base >= .5f ? Color{225, 250, 230, 255} : (o.base < 0 ? Color{255, 225, 220, 255} : WHITE));
    z.react(o.emote, o.pose, 3.f);
    if (!o.feedCtx.empty()) {
        bool always = o.feedCtx == "selfie" || o.feedCtx.rfind("heckler", 0) == 0;
        if (always || chance(.3f)) c.react(o.feedCtx, z.region, o.base - .4f);
    }
    c.normalize();
}

}  // namespace seat
