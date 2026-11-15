#include "Campaign.hpp"

namespace seat {

static const char* REGION_HANDLES[NREGION][2] = {
    {"@uniroad_memes", "@topper_ki_awaaz"},     {"@township_rwa", "@techie_in_chandanpur"},
    {"@bazaar_khabar", "@purana_shahar_diaries"}, {"@mill_mazdoor_voice", "@chawl_no_7"},
    {"@kisan_awaaz", "@gaon_ki_beti"}};
static const std::vector<std::string> GENERAL_HANDLES = {"@chandanpur_buzz", "@angry_uncle_77", "@cp_times_live",
                                                         "@neutral_voter_99", "@chai_pe_charcha"};
static const char* RIVAL_HANDLES[NCAND] = {"", "@prakash_fan_club", "@rao_for_change", "@bhola_bhai_army"};

void Campaign::start(const std::string& n, const std::string& p, int sym, int background, int difficulty) {
    *this = Campaign();
    name = n;
    party = p;
    symbol = sym;
    bg = background;
    diff = difficulty;
    funds = BGS[bg].funds;
    rep = BGS[bg].rep;
    hype = BGS[bg].hype;
    el.init(DIFFS[diff].D);
    dayStartShare = yourShare();
    addLog(name + " files nomination papers for Chandanpur on the " + party + " ticket.", T_INFO);
    post("@cp_times_live", "New face in the Chandanpur race: " + name + " (" + party + ") files nomination. Can a newcomer take on a three-term MP?", T_INFO, 412);
}

float Campaign::mult() const {
    float tired = 0.6f + 0.4f * clampf(stamina, 0, 100) / 100.f;
    return 0.62f * (0.55f + rep / 100.f * 0.9f) * (1.f + hype / 120.f) * tired;
}

float Campaign::gain(float base, int r, bool fatigue) {
    float g = base * mult() * frand(.85f, 1.15f);
    if (fatigue && r >= 0) g /= (1.f + visits[r] * 0.07f);
    return g;
}

void Campaign::addLog(const std::string& t, Tone tone) {
    log.insert(log.begin(), {day, t, tone});
    if (log.size() > 120) log.pop_back();
}

void Campaign::post(const std::string& handle, const std::string& text, Tone tone, int likes) {
    if (likes < 0) likes = irand(20, 400) + (int)(hype * irand(5, 40));
    feed.push_front({handle, text, likes, tone, day});
    if (feed.size() > 60) feed.pop_back();
}

void Campaign::doing(const std::string& what) { diary.push_back({clockStr(minute), what}); }

void Campaign::spendTime(float mins, float perHour) {
    minute += mins;
    stamina -= mins / 60.f * perHour;
    normalize();
}

void Campaign::normalize() {
    rep = clampf(rep, 0, 100);
    hype = clampf(hype, 0, 60);
    stamina = clampf(stamina, 0, 100);
    if (funds < 0) funds = 0;
    if (strikes >= 3) ended = DQ;
}

void Campaign::everywhere(float base) {
    for (int r = 0; r < NREGION; ++r) el.addYou(r, gain(base, -1, false));
}

// ---------------------------------------------------------------- public reaction
void Campaign::react(const std::string& ctx, int r, float o) {
    const std::string R = r >= 0 ? REGIONS[r].name : "Chandanpur";
    const std::string h = r >= 0 && chance(.7f) ? REGION_HANDLES[r][irand(0, 1)] : pick(GENERAL_HANDLES);
    auto P = [&](const std::string& t, Tone tone) { post(h, t, tone); };
    bool pos = o > 0.15f, neg = o < -0.1f;
    if (ctx == "rally") {
        if (pos) P(pick(std::vector<std::string>{"Massive crowd at " + name + "'s rally in " + R + ". The ground was overflowing.",
                                                 "Went for the free samosas, stayed for " + name + "'s speech. Not bad.",
                                                 name + " just said what " + R + " has been saying for years."}), T_GOOD);
        else if (neg) P(pick(std::vector<std::string>{"Half-empty chairs at " + name + "'s rally in " + R + ". Even the DJ left early.",
                                                      "Paid crowd at " + name + "'s rally? My cousin got 300 rupees and a lunch box."}), T_BAD);
        else P(name + " held a rally in " + R + ". Speech was okay, traffic was not.", T_INFO);
    } else if (ctx == "roadshow") {
        if (pos) P("Rose petals all over " + R + ". " + name + "'s roadshow just passed our gali.", T_GOOD);
        else P("Stuck in traffic for two hours because of " + name + "'s roadshow. Not impressed.", T_BAD);
    } else if (ctx == "talk") {
        if (pos) P(pick(std::vector<std::string>{name + " sat on a charpai in " + R + " and actually listened for twenty minutes. Rare.",
                                                 "Met " + name + " outside the ration shop today. Seems genuine.",
                                                 name + " remembered my mother's name. Okay, I'm impressed."}), T_GOOD);
        else if (neg) P(name + " came to " + R + ", made a promise, clicked a photo, left. Same old.", T_BAD);
    } else if (ctx == "heckler_calm") {
        P("Watch " + name + " handle a heckler in " + R + " without losing their temper. Respect.", T_GOOD);
    } else if (ctx == "heckler_angry") {
        P("VIDEO: " + name + " loses cool and shouts back at a voter in " + R + ".", T_BAD);
    } else if (ctx == "press") {
        if (pos) P(name + "'s press conference today was sharp. Rivals have no answer.", T_GOOD);
        else P("Defamation notice served on " + name + ". Should have stuck to facts.", T_BAD);
    } else if (ctx == "gifts_caught") {
        post("@cp_times_live", "Flying squad seizes cash from " + name + "'s worker in " + R + ". So much for clean politics.", T_BAD, 2400);
    } else if (ctx == "social") {
        if (pos) P(pick(std::vector<std::string>{"Can't stop humming " + name + "'s campaign jingle. Send help.",
                                                 name + "'s reel has 4 lakh views. My dadi forwarded it to me twice."}), T_GOOD);
        else P(name + "'s campaign reel has a spelling mistake in the caption. We are dying.", T_BAD);
    } else if (ctx == "selfie") {
        P("Got a selfie with " + name + " in " + R + "! Voting decided? Maybe.", T_GOOD);
    } else if (ctx == "debate") {
        P(pos ? name + " won the debate tonight. Not even close." : "Debate tonight: lots of noise, little substance.", pos ? T_GOOD : T_INFO);
    }
}

// ---------------------------------------------------------------- actions
std::string Campaign::travel(int r) {
    spendTime(60, 3);
    location = r;
    doing("Drove to " + std::string(REGIONS[r].name));
    return "";
}

int Campaign::crowdSize(int r, bool bussed) const {
    float s = el.exact(r)[YOU];
    float n = 1500 + s * 160 + hype * 120;
    if (bussed) n *= 1.45f;
    if (rain) n *= .5f;
    if (bg == BG_STAR) n *= 1.3f;
    return (int)n;
}

std::string Campaign::rallyFinish(int r, float quality, bool bussed, int crowd) {
    float base = 7.f * quality * (bussed ? 1.12f : 1.f) * (bg == BG_STAR ? 1.25f : 1.f);
    if (rain) base *= .55f;
    el.addYou(r, gain(base, r));
    visits[r]++;
    hype += 4 + quality * 5;
    stats.rallies++;
    std::string msg;
    int claimed = crowd * irand(4, 7);
    if (quality >= 1.0f)
        msg = strf("A roaring success. Police estimate the crowd at %s; your team claims %s.", fmtIN(crowd).c_str(), fmtIN(claimed).c_str());
    else if (quality >= .7f)
        msg = strf("A solid rally. Police estimate %s people; your team says %s.", fmtIN(crowd).c_str(), fmtIN(claimed).c_str());
    else {
        msg = strf("The crowd drifted away mid-speech. Police estimate %s; even your team won't put a number on it.", fmtIN(crowd).c_str());
        hype -= 6;
    }
    addLog(std::string("Rally in ") + REGIONS[r].name + ". " + msg, quality >= .7f ? T_GOOD : T_BAD);
    react("rally", r, quality - .75f);
    headline = std::string("Big crowds as ") + name + " rallies in " + REGIONS[r].name;
    normalize();
    return msg;
}

std::string Campaign::roadshow(int r) {
    spend(4);
    spendTime(120, 4);
    float g = gain(rain ? 1.8f : 3.6f, r) * (bg == BG_STAR ? 1.25f : 1.f);
    el.addYou(r, g);
    visits[r]++;
    hype += 4;
    stats.roadshows++;
    doing(std::string("Roadshow through ") + REGIONS[r].name);
    bool jam = chance(.2f);
    react("roadshow", r, jam ? -.5f : .5f);
    if (jam) rep -= 1;
    normalize();
    return jam ? "The roadshow went well, but it jammed the main road for two hours. Some commuters are furious."
               : pick(std::vector<std::string>{"Rose petals, slogans and kids running beside the jeep the whole way.",
                                                "Shopkeepers came out to wave. A grandmother tied a rakhi-style thread on your wrist."});
}

std::string Campaign::trainBooth(int r) {
    spend(2);
    spendTime(60, 2);
    booth[r]++;
    doing(std::string("Trained booth agents in ") + REGIONS[r].name);
    return strf("Booth team %d of 3 trained in %s. On polling morning they will knock on every supporter's door.", booth[r], REGIONS[r].name);
}

std::string Campaign::gifts(int r) {
    spend(7, true);
    spendTime(60, 2);
    el.addYou(r, gain(8.f, r, false));
    rep -= 2;
    stats.gifts++;
    doing(std::string("\"Distribution\" in ") + REGIONS[r].name);
    float p = silence() ? .45f : .3f;
    if (chance(p)) {
        strikes++;
        rep -= 12;
        react("gifts_caught", r, -1);
        addLog("Flying squad seizes cash and sarees from your worker's car.", T_BAD);
        headline = std::string("Cash seized from ") + name + "'s worker";
        normalize();
        return strf("The election flying squad stops your worker's car in %s and seizes cash and 400 sarees. The EC issues you a notice (%d of 3).",
                    REGIONS[r].name, strikes);
    }
    normalize();
    return std::string("Envelopes went out quietly in ") + REGIONS[r].name + ". Nobody saw anything. Probably.";
}

std::string Campaign::social() {
    spend(3);
    spendTime(120, 1);
    float f = 1.f / (1.f + socialUses * .15f);
    socialUses++;
    for (int r = 0; r < NREGION; ++r) {
        const auto& b = REGIONS[r].blocs;
        el.addYou(r, gain(7.f * (b[YOUTH] + b[MIDDLE] * .8f) * f, -1, false));
    }
    hype += 4;
    doing("Shot campaign reels with the social media team");
    if (chance(.12f)) {
        rep -= 4;
        react("social", -1, -1);
        normalize();
        return "Your campaign reel went viral, for a spelling mistake in the caption. The trolls are having a field day.";
    }
    react("social", -1, 1);
    normalize();
    return "Your jingle is stuck in every college student's head, and the WhatsApp uncles are forwarding your manifesto card.";
}

std::string Campaign::press(int t) {
    spend(.5f);
    spendTime(90, 2);
    stats.press++;
    hype += 5;
    doing(std::string("Press conference on ") + RIVALS[t].name);
    if (chance(.22f)) {
        rep -= 5;
        react("press", -1, -1);
        normalize();
        return std::string(RIVALS[t].shortName) + "'s lawyers send you a defamation notice. The channels replay your worst line all evening.";
    }
    el.hit(t, -1, 2.4f * frand(.8f, 1.2f));
    react("press", -1, 1);
    headline = name + " targets " + RIVALS[t].name;
    normalize();
    return std::string("Your attack on ") + RIVALS[t].shortName + " leads the 9 o'clock news.";
}

std::string Campaign::promise(int k) {
    spendTime(60, 1);
    promised[k] = true;
    nPromises++;
    for (int r = 0; r < NREGION; ++r) {
        int w = REGIONS[r].issues[k];
        if (w) el.addYou(r, gain(1.8f * w, -1, false));
    }
    std::string extra;
    if (nPromises > freePromises()) {
        rep -= 3;
        extra = " Some voters roll their eyes: how will you pay for all this?";
    }
    doing(std::string("Added to manifesto: ") + ISSUES[k].label);
    post(pick(GENERAL_HANDLES), name + " promises: \"" + ISSUES[k].promise + ".\" Saving this post for next election.", T_INFO);
    normalize();
    return std::string("New manifesto promise: \"") + ISSUES[k].promise + ".\"" + extra;
}

std::string Campaign::endorse(int g) {
    const GroupDef& G = GROUPS[g];
    bool ok = promised[G.wants];
    spendTime(120, 2);
    if (!ok) {
        spend(5);
        rep -= 3;
    }
    endorsed[g] = true;
    nEndorse++;
    for (int r = 0; r < NREGION; ++r) {
        float s = REGIONS[r].blocs[G.bloc];
        if (s > 0) el.addYou(r, gain(16.f * s, -1, false));
    }
    doing(std::string("Meeting with ") + G.name);
    post(pick(GENERAL_HANDLES), std::string(G.name) + " backs " + name + ". " + BLOC_NAMES[G.bloc] + " across Chandanpur take note.", T_GOOD);
    normalize();
    return std::string(G.name) + " endorses you" + (ok ? "." : " after a generous sponsorship of their annual function.");
}

std::string Campaign::fundSmall() {
    spendTime(120, 1);
    float d = smallDonation();
    funds += d;
    doing("Fundraising with small donors");
    return "A thousand small donations add up to " + fmtL(d) + ".";
}

std::string Campaign::fundBuilder() {
    spendTime(120, 1);
    funds += 12;
    rep -= 5;
    builder = true;
    doing("Dinner with a builder");
    normalize();
    return "A builder drops off " RUPEE "12L in a box of kaju katli. He smiles like a man who will call again.";
}

std::string Campaign::survey() {
    spend(2);
    spendTime(60, 1);
    surveyUntil = day + 2;
    doing("Commissioned an internal survey");
    return strf("Survey commissioned. Your map shows exact numbers until day %d.", std::min(surveyUntil, maxDay));
}

std::string Campaign::rest() {
    spendTime(120, 0);
    stamina += 30;
    doing("Rested at the campaign office");
    normalize();
    return "Two hours of sleep on the office sofa and a glass of nimbu-paani. You feel human again.";
}

// ---------------------------------------------------------------- rally speech
std::vector<SpeechLine> Campaign::speechOptions(int r, unsigned usedMask) {
    std::vector<SpeechLine> all;
    for (int k = 0; k < NISSUE; ++k)
        if (!(usedMask & (1u << k))) all.push_back({ISSUES[k].rallyLine, SP_ISSUE, k});
    for (int c = PD; c <= IND; ++c)
        if (!(usedMask & (1u << (8 + c)))) all.push_back({RIVALS[c].attackLine, SP_ATTACK, c});
    static const std::vector<std::string> emo = {"My mother sold vegetables in a bazaar just like this one. I know your struggle.",
                                                 "I am not here as your leader. I am here as your servant.",
                                                 "Every vote you cast is a prayer for your children's future."};
    if (!(usedMask & (1u << 12))) all.push_back({pick(emo), SP_EMOTION, 0});
    if (!(usedMask & (1u << 13))) all.push_back({"Free electricity, up to 200 units, for every home in Chandanpur!", SP_FREEBIE, 0});
    if (!(usedMask & (1u << 14))) all.push_back({"Prakash-ji's flyover is so slow, my grandson will cut its ribbon!", SP_JOKE, PD});
    // offer one line on a strong local issue when possible, plus two random others
    std::vector<SpeechLine> out;
    std::vector<int> idx(all.size());
    for (size_t i = 0; i < idx.size(); ++i) idx[i] = (int)i;
    std::shuffle(idx.begin(), idx.end(), rng().g);
    for (int i : idx)
        if (all[i].kind == SP_ISSUE && REGIONS[r].issues[all[i].arg] >= 2 && chance(.7f)) {
            out.push_back(all[i]);
            break;
        }
    for (int i : idx) {
        if (out.size() >= 3) break;
        bool dup = false;
        for (auto& o : out) dup |= (o.text == all[i].text);
        if (!dup) out.push_back(all[i]);
    }
    std::shuffle(out.begin(), out.end(), rng().g);
    return out;
}

float Campaign::speechReaction(int r, const SpeechLine& s, unsigned usedKinds) {
    float v = 0;
    const RegionDef& R = REGIONS[r];
    switch (s.kind) {
        case SP_ISSUE: v = -0.2f + R.issues[s.arg] * 0.33f + (promised[s.arg] ? .15f : 0.f); break;
        case SP_ATTACK: {
            float sh = el.exact(r)[s.arg];
            v = sh > 35 ? -0.25f : 0.55f - sh / 100.f;
            if (s.arg == PD && (r == UNI || r == TOWN)) v += .15f;
        } break;
        case SP_EMOTION: v = 0.3f + hype / 150.f; break;
        case SP_FREEBIE: v = 0.85f - R.blocs[MIDDLE] * 1.1f; break;
        case SP_JOKE: v = 0.45f + frand(-.35f, .3f); break;
    }
    if (usedKinds & (1u << s.kind)) v -= .2f;  // repeating the same trick
    v += hype / 300.f + frand(-.15f, .15f);
    return clampf(v, -1.f, 1.f);
}

void Campaign::speechSideEffects(int r, const SpeechLine& s) {
    if (s.kind == SP_ATTACK) el.hit(s.arg, r, 1.2f);
    if (s.kind == SP_FREEBIE) rep -= 2;
    if (s.kind == SP_JOKE) el.hit(PD, r, .6f);
    normalize();
}

// ---------------------------------------------------------------- rivals and day cycle
void Campaign::rivalsAct() {
    float D = el.D;
    std::string bits;
    for (int k = PD; k <= IND; ++k) {
        const RivalDef& rv = RIVALS[k];
        for (int r = 0; r < NREGION; ++r) el.score[r][k] += rv.str * rv.aff[r] * frand(.5f, 1.3f) * D;
        float tot = 0;
        for (float a : rv.aff) tot += a;
        float x = frand(0, tot);
        int f = 0;
        for (; f < NREGION - 1; ++f) {
            x -= rv.aff[f];
            if (x <= 0) break;
        }
        el.score[f][k] += 4.5f * D * frand(.7f, 1.3f);
        if (k == IND && bribedInd) continue;
        std::string verb = pick(rv.verbs);
        if (!bits.empty()) bits += ". ";
        bits += std::string(rv.shortName) + " " + verb + " in " + REGIONS[f].name;
        if (chance(.45f)) post(RIVAL_HANDLES[k], std::string(rv.name) + " " + verb + " in " + REGIONS[f].name + " today. Unstoppable!", T_RIVAL);
    }
    addLog("Rival trail: " + bits + ".", T_RIVAL);
    Shares ov = el.overall(true);
    if (day >= 5 && ov[YOU] >= std::max({ov[PD], ov[LKM], ov[IND]})) {
        for (int r = 0; r < NREGION; ++r) el.score[r][YOU] = std::max(1.f, el.score[r][YOU] - 1.3f * D);
        addLog("All three rivals are now attacking you. Being the front-runner has a price.", T_BAD);
        post("@prakash_fan_club", "Who is funding " + name + "? Questions must be asked.", T_RIVAL);
    }
}

void Campaign::endDay() {
    rivalsAct();
    // night recovery depends on how late you worked
    float lateBy = std::max(0.f, minute - DAY_END);
    stamina = clampf(stamina + 75 - lateBy / 4.f, 0, 100);
    hype = clampf(hype * .85f, 0, 60);
    day++;
    minute = DAY_START;
    location = -1;
    rain = false;
    diary.clear();
    el.rerollNoise();
    dayStartShare = yourShare();
    normalize();
}

void Campaign::computePolling() {
    for (int r = 0; r < NREGION; ++r) {
        float w[NCAND];
        float tot = 0;
        for (int k = 0; k < NCAND; ++k) {
            float v = std::max(.5f, el.score[r][k] * (1 + gauss() * .06f));
            float t = k == YOU ? 1 + booth[r] * .06f + hype / 400.f : RIVALS[k].turn;
            w[k] = (k == IND && bribedInd) ? v * .6f : v * t;
            tot += w[k];
        }
        float turnout = clampf(REGIONS[r].turnout + hype / 700.f + frand(-.03f, .03f) + booth[r] * .008f, .4f, .86f);
        long long polled = std::llround(REGIONS[r].voters * turnout);
        long long nota = std::llround(polled * frand(.006f, .014f));
        res[r].turnout = turnout;
        res[r].polled = polled;
        for (int k = 0; k < NCAND; ++k) res[r].votes[k] = std::llround((polled - nota) * w[k] / tot);
        res[r].votes[NOTA] = nota;
    }
}

void Campaign::finishResult() {
    total = Tally{};
    for (int r = 0; r < NREGION; ++r)
        for (int k = 0; k <= NCAND; ++k) total[k] += res[r].votes[k];
    for (int k = 0; k < NCAND; ++k) order[k] = k;
    std::sort(order, order + NCAND, [&](int a, int b) { return total[a] > total[b]; });
    valid = 0;
    for (int k = 0; k < NCAND; ++k) valid += total[k];
    margin = total[order[0]] - total[order[1]];
    share = total[YOU] * 100.f / valid;
    pos = (int)(std::find(order, order + NCAND, (int)YOU) - order) + 1;
    if (order[0] == YOU) ended = spent > EC_LIMIT ? PETITION : WIN;
    else ended = LOSE;
}

}  // namespace seat
