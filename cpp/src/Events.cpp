#include "Events.hpp"

namespace seat {

using S = std::string;
static float G(Campaign& c, float b) { return c.gain(b, -1, false); }

static std::vector<EventDef> build() {
    std::vector<EventDef> E;

    E.push_back({"pipe", "Pipeline burst", nullptr, [](Campaign&) { return pick(std::vector<int>{OLD, MILL, GAON}); },
                 [](Campaign&, int r) { return S("A main water line has burst in ") + REGIONS[r].name + ". Taps are dry, buckets are out and tempers are short."; },
                 {{"Arrive with water tankers", "", 3, 1, false, nullptr,
                   [](Campaign& c, int r) { c.el.addYou(r, G(c, 9)); c.rep += 3; c.react("talk", r, .8f);
                       return S("Photos of you handing out water in ") + REGIONS[r].name + " are on every front page."; }},
                  {"Blame the sitting MP", "", 0, 0, false, nullptr,
                   [](Campaign& c, int r) { c.el.hit(PD, r, 5); c.hype += 4; return S("\"Ten years and not one new pipeline,\" you thunder. Prakash's camp is rattled."); }},
                  {"Stick to your schedule", "", 0, 0, false, nullptr,
                   [](Campaign& c, int r) { c.el.addYou(r, -3); return S("People in ") + REGIONS[r].name + " noticed who did not turn up."; }}},
                 nullptr});

    E.push_back({"deepfake", "Deepfake video", nullptr, nullptr,
                 [](Campaign&, int) { return S("A doctored video shows you insulting farmers. It is fake, but it is on every phone in Chandanpur."); },
                 {{"File a cyber-crime complaint and clarify", "", 1, 1, false, nullptr,
                   [](Campaign& c, int) { c.rep += 3; c.hype -= 2; return S("Police trace the video to a rival IT cell. Most voters believe you."); }},
                  {"Fight memes with memes", "", 2, 0, false, nullptr,
                   [](Campaign& c, int) { c.hype += 7; if (chance(.35f)) { c.rep -= 6; return S("It turned into a mud-slinging war. Hype is up, dignity is down."); }
                       return S("Your counter-meme is funnier than the fake. The internet moves on."); }},
                  {"Ignore it", "", 0, 0, false, nullptr,
                   [](Campaign& c, int) { c.el.addYou(UNI, -3); c.el.addYou(TOWN, -3); c.el.addYou(GAON, -2); return S("Silence looked like guilt to some voters."); }}},
                 nullptr});

    E.push_back({"farmers", "Farmers block the highway", nullptr, nullptr,
                 [](Campaign&, int) { return S("Farmers from the Gaon Belt are on a dharna over crop prices. Tractors line the highway for two kilometres."); },
                 {{"Join the sit-in", "Takes 3 hours", 0, 3, false, nullptr,
                   [](Campaign& c, int) { c.el.addYou(GAON, G(c, 11)); c.hype += 4; c.doing("Sat on dharna with farmers on the highway");
                       c.react("talk", GAON, 1); return S("You sat on the tarmac with the farmers for hours. The Kisan Union notices."); }},
                  {"Issue a statement of support", "", 0, 0, false, nullptr,
                   [](Campaign& c, int) { c.el.addYou(GAON, G(c, 3)); return S("A polite statement. Polite applause."); }},
                  {"Stay away", "", 0, 0, false, nullptr, [](Campaign& c, int) { c.el.addYou(GAON, -4); return S("Farmers note your absence."); }}},
                 nullptr});

    E.push_back({"celeb", "A TV star offers to campaign", nullptr, nullptr,
                 [](Campaign&, int) { return S("A popular TV serial actor will campaign with you for two days. The fee is " RUPEE "10L."); },
                 {{"Pay the fee", "", 10, 0, false, nullptr,
                   [](Campaign& c, int) { c.hype += 14; c.el.addYou(UNI, G(c, 4)); c.el.addYou(TOWN, G(c, 4)); return S("Crowds came for the star and stayed for your speech."); }},
                  {"Decline politely", "", 0, 0, false, nullptr, [](Campaign& c, int) { c.rep += 1; return S("You keep the money for diesel and posters."); }}},
                 nullptr});

    E.push_back({"nephew", "Help from party headquarters", nullptr, nullptr,
                 [](Campaign& c, int) { return c.party + " headquarters is sending " RUPEE "8L. The general secretary's nephew comes with it, as your \"campaign manager\"."; },
                 {{"Take the money and the nephew", "", 0, 0, false, nullptr,
                   [](Campaign& c, int) { c.funds += 8; c.rep -= 3; return S("The nephew mostly takes selfies, but the money is real."); }},
                  {"Refuse both", "", 0, 0, false, nullptr, [](Campaign& c, int) { c.rep += 4; return S("Local papers praise your independence. Headquarters sulks."); }}},
                 nullptr});

    E.push_back({"defector", "A defector knocks", [](Campaign& c) { return c.day >= 3; }, [](Campaign&) { return irand(0, NREGION - 1); },
                 [](Campaign&, int r) { return S("A Pragati Dal ward councillor from ") + REGIONS[r].name + " wants to join you, with his supporters."; },
                 {{"Welcome him with a garland", "", 0, 0, false, nullptr,
                   [](Campaign& c, int r) { c.el.hit(PD, r, 6); c.el.addYou(r, 6); c.rep -= 2; return S("The councillor switches sides. Some call it opportunism."); }},
                  {"Turn him away", "", 0, 0, false, nullptr, [](Campaign& c, int) { c.rep += 2; return S("\"No turncoats,\" you say. Editorials approve."); }}},
                 nullptr});

    E.push_back({"muscle", "Trouble in Mill Colony", [](Campaign& c) { return !c.bribedInd; }, nullptr,
                 [](Campaign&, int) { return S("Bhola Pehelwan's supporters broke up your corner meeting in Mill Colony last night and tore your posters."); },
                 {{"File a police complaint", "", 0, 1, false, nullptr,
                   [](Campaign& c, int) { c.rep += 3; c.el.hit(IND, MILL, 3); return S("The FIR makes the news. Bhola looks like a bully."); }},
                  {"Send your own muscle", "", 0, 0, false, nullptr,
                   [](Campaign& c, int) { if (chance(.4f)) { c.strikes++; c.rep -= 10; return strf("A street fight breaks out and the EC sends you a notice (%d of 3).", c.strikes); }
                       c.el.hit(IND, MILL, 7); c.hype += 3; return S("Bhola's men backed off. Nobody was hurt, this time."); }},
                  {"Avoid Mill Colony for now", "", 0, 0, false, nullptr, [](Campaign& c, int) { c.el.addYou(MILL, -3); return S("Mill workers read it as fear."); }}},
                 nullptr});

    E.push_back({"onion", "Onions hit " RUPEE "90 a kilo", nullptr, nullptr,
                 [](Campaign&, int) { return S("Onion prices have doubled in a week. Every kitchen in Chandanpur is talking about it."); },
                 {{"Promise price relief", "Adds it to your manifesto", 0, 0, false, [](Campaign& c) { return !c.promised[PRICES]; },
                   [](Campaign& c, int) { c.minute -= 60; return c.promise(PRICES); }},
                  {"Run cheap-onion stalls", "", 2.5f, 1, false, nullptr,
                   [](Campaign& c, int) { c.el.addYou(OLD, G(c, 5)); c.el.addYou(MILL, G(c, 5)); return S("Queues at your onion stalls stretch around the block."); }},
                  {"Blame global markets", "", 0, 0, false, nullptr, [](Campaign& c, int) { c.hype -= 2; return S("Nobody wants an economics lecture when the dal is plain."); }}},
                 nullptr});

    E.push_back({"cricket", "Cricket final at University Road", nullptr, nullptr,
                 [](Campaign&, int) { return S("The inter-college cricket final is on Sunday and the organisers want a chief guest."); },
                 {{"Sponsor the trophy", "", 2, 0, false, nullptr,
                   [](Campaign& c, int) { c.el.addYou(UNI, G(c, 6)); return S("Your symbol is on the trophy and in every team photo."); }},
                  {"Be chief guest and face one ball", "Takes 2 hours", 0, 2, false, nullptr,
                   [](Campaign& c, int) { c.doing("Chief guest at the cricket final");
                       if (chance(.5f)) { c.hype += 8; c.el.addYou(UNI, G(c, 4)); c.post("@uniroad_memes", c.name + " just hit a SIX at the college final. Vote decided.", T_GOOD, 3100); return S("You hit it for six. The clip is everywhere."); }
                       c.hype += 3; c.post("@uniroad_memes", c.name + " clean bowled first ball lol. Respect for trying though.", T_INFO, 2200);
                       return S("Clean bowled first ball. Even the bowler laughed, and the clip still went viral."); }}},
                 nullptr});

    E.push_back({"rain", "Unseasonal rain", nullptr, nullptr,
                 [](Campaign&, int) { return S("Heavy rain lashes Chandanpur. Rallies and roadshows will draw thin crowds today."); },
                 {{"Hand out umbrellas with your symbol", "", 2, 0, false, nullptr,
                   [](Campaign& c, int) { c.hype += 4; c.everywhere(1); return S("A sea of umbrellas with your symbol, on every street."); }},
                  {"Plan indoor work today", "", 0, 0, false, nullptr, [](Campaign&, int) { return S("Meetings indoors today. Rallies another day."); }}},
                 [](Campaign& c, int) { c.rain = true; }});

    E.push_back({"speech", "Model Code of Conduct notice", [](Campaign& c) { return c.day >= 4; }, nullptr,
                 [](Campaign&, int) { return S("One of your local workers gave an inflammatory speech. The Election Commission has asked you to explain."); },
                 {{"Expel him and apologise", "", 0, 0, false, nullptr, [](Campaign& c, int) { c.rep += 2; c.hype -= 3; return S("A clean apology. The story dies in a day."); }},
                  {"Defend him", "", 0, 0, false, nullptr,
                   [](Campaign& c, int) { c.hype += 5; if (chance(.5f)) { c.strikes++; c.rep -= 6; return strf("The EC is not amused. Formal notice issued (%d of 3).", c.strikes); }
                       return S("The storm passes. Your base loved the defiance."); }}},
                 nullptr});

    E.push_back({"assets", "A reporter asks about your assets", nullptr, nullptr,
                 [](Campaign&, int) { return S("A reporter wants to know why your election affidavit lists only one scooter."); },
                 {{"Publish a full asset statement", "", 0, 0, false, nullptr, [](Campaign& c, int) { c.rep += 5; return S("Your transparency becomes a talking point."); }},
                  {"Refuse to comment", "", 0, 0, false, nullptr, [](Campaign& c, int) { c.rep -= 4; return S("\"What is the candidate hiding?\" asks tomorrow's headline."); }}},
                 nullptr});

    E.push_back({"pothole", "A school bus stuck in a pothole", nullptr, nullptr,
                 [](Campaign&, int) { return S("A school bus sank axle-deep into a pothole in New Township. No one was hurt, and parents are furious."); },
                 {{"Fix the road with campaign funds", "", 3, 0, false, nullptr,
                   [](Campaign& c, int) { c.el.addYou(TOWN, G(c, 8)); c.rep += 2; return S("Fresh tarmac by morning. The RWA WhatsApp group is full of thank-yous."); }},
                  {"Hold a selfie protest in the pothole", "Takes 1 hour", 0, 1, false, nullptr,
                   [](Campaign& c, int) { c.hype += 6; c.el.addYou(TOWN, G(c, 3)); c.react("selfie", TOWN, 1); return S("You, ankle-deep in a pothole, holding a placard. It works."); }}},
                 nullptr});

    E.push_back({"builder", "The builder calls in his favour", [](Campaign& c) { return c.builder; }, nullptr,
                 [](Campaign&, int) { return S("Your builder donor wants you to promise that the wetland by the river will be rezoned for his towers."); },
                 {{"Agree, quietly", "", 0, 0, false, nullptr, [](Campaign& c, int) { c.funds += 6; c.rep -= 8; return S("Another " RUPEE "6L arrives. A local journalist starts asking questions."); }},
                  {"Refuse", "", 0, 0, false, nullptr, [](Campaign& c, int) { c.rep += 4; c.el.boost(PD, -1, 3); return S("He takes his money to the Pragati Dal. You keep your conscience."); }}},
                 nullptr});

    E.push_back({"song", "Your campaign song goes viral", nullptr, nullptr,
                 [](Campaign&, int) { return S("An auto-rickshaw driver wrote a song about you. It is catchy, slightly off-key and everywhere."); },
                 {{"Play it at every stop", "", 0, 0, false, nullptr, [](Campaign& c, int) { c.hype += 10; c.everywhere(1); c.react("social", -1, 1); return S("The whole constituency is humming it."); }}},
                 nullptr});

    E.push_back({"weddings", "Seven wedding invitations", nullptr, nullptr,
                 [](Campaign&, int) { return S("Seven wedding invitations tonight across the constituency. No politician skips the wedding season."); },
                 {{"Attend all seven", "Takes 3 hours", 0, 3, false, nullptr, [](Campaign& c, int) { c.everywhere(2.2f); c.doing("Attended seven weddings"); return S("Seven weddings, seven plates of food and a lot of blessings."); }},
                  {"Send shagun envelopes", "", 1.5f, 0, false, nullptr, [](Campaign& c, int) { c.everywhere(1); return S("Envelopes sent with your best wishes."); }},
                  {"Skip them", "", 0, 0, false, nullptr, [](Campaign&, int) { return S("You catch up on sleep. Some aunties will remember this."); }}},
                 nullptr});

    E.push_back({"sting", "Sting operation on the Pragati Dal", [](Campaign& c) { return c.day >= 6; }, nullptr,
                 [](Campaign&, int) { return S("A sting video shows a Pragati Dal aide promising contracts in exchange for votes."); },
                 {{"Amplify it everywhere", "", 0, 0, false, nullptr,
                   [](Campaign& c, int) { c.hype += 5; c.el.hit(PD, -1, 4); if (chance(.25f)) { c.rep -= 3; return S("You hammered it so hard some voters felt sorry for Prakash."); }
                       return S("The video plays on loop at every one of your stops."); }},
                  {"Stay above it", "", 0, 0, false, nullptr, [](Campaign& c, int) { c.rep += 3; c.el.hit(PD, -1, 2); return S("\"Let the voters decide,\" you say. It looks statesmanlike."); }}},
                 nullptr});

    E.push_back({"bhola", "An offer from Bhola Pehelwan", [](Campaign& c) { return c.day >= 7 && !c.bribedInd; }, nullptr,
                 [](Campaign&, int) { return S("Bhola Pehelwan sends word: for " RUPEE "15L he will \"step back\" and tell his supporters to vote for you. It is illegal and off the books."); },
                 {{"Pay him", "Illegal, off the books", 15, 0, true, nullptr,
                   [](Campaign& c, int) { c.bribedInd = true;
                       for (int r = 0; r < NREGION; ++r) { float m = c.el.score[r][IND] * .5f; c.el.score[r][IND] -= m; c.el.score[r][YOU] += m * .7f; }
                       if (chance(.3f)) { c.strikes++; c.rep -= 15; c.post("@cp_times_live", "AUDIO LEAK: " + c.name + " and Bhola Pehelwan discuss a 'settlement'.", T_BAD, 5200);
                           return strf("A recording of the deal leaks. The EC issues a notice (%d of 3) and voters are disgusted, though Bhola still goes quiet.", c.strikes); }
                       return S("Bhola stops campaigning and his men start wearing your scarf."); }},
                  {"Refuse", "", 0, 0, false, nullptr, [](Campaign& c, int) { c.rep += 3; return S("You send the messenger back. Bhola vows to make your life hard."); }}},
                 nullptr});

    E.push_back({"hospital", "Ambulance stuck at the railway crossing", nullptr, nullptr,
                 [](Campaign&, int) { return S("An ambulance waited forty minutes at the shut railway crossing. The patient survived, and the city is angry."); },
                 {{"Visit the family at the hospital", "Takes 2 hours", 0, 2, false, nullptr,
                   [](Campaign& c, int) { c.rep += 3; c.el.addYou(OLD, G(c, 4)); c.el.addYou(TOWN, G(c, 3)); c.doing("Visited a patient's family at the district hospital"); return S("You sat with the family, quietly, without cameras. Word spreads anyway."); }},
                  {"Promise the overbridge", "Adds roads to your manifesto", 0, 0, false, [](Campaign& c) { return !c.promised[ROADS]; },
                   [](Campaign& c, int) { c.minute -= 60; return c.promise(ROADS); }},
                  {"Post a statement online", "", 0, 0, false, nullptr, [](Campaign& c, int) { c.hype += 1; return S("A statement that 32 people liked."); }}},
                 nullptr});
    return E;
}

const std::vector<EventDef>& allEvents() {
    static std::vector<EventDef> E = build();
    return E;
}

const EventDef* rollEvent(Campaign& c) {
    const auto& E = allEvents();
    auto unused = [&](const EventDef& e) { return std::find(c.used.begin(), c.used.end(), e.id) == c.used.end() && (!e.cond || e.cond(c)); };
    for (auto& e : E)
        if (e.id == "builder" && unused(e) && chance(.5f)) return &e;
    if (!chance(.72f)) return nullptr;
    std::vector<const EventDef*> pool;
    for (auto& e : E)
        if (unused(e)) pool.push_back(&e);
    if (pool.empty()) return nullptr;
    return pool[irand(0, (int)pool.size() - 1)];
}

const std::vector<DebateQuestion>& debate() {
    static std::vector<DebateQuestion> D = {
        {"How will you create jobs in Chandanpur?", "The moderator turns to you first. Three hundred students lean forward.",
         {{"Reopen the textile mill as a workers' cooperative", [](Campaign& c) { c.el.addYou(MILL, G(c, 7)); c.el.addYou(GAON, G(c, 2)); return S("Mill Colony cheers"); }},
          {"A start-up hub and skills centres near the university", [](Campaign& c) { c.el.addYou(UNI, G(c, 7)); c.el.addYou(TOWN, G(c, 3)); return S("students love it"); }},
          {"Tax holidays to bring in a big factory", [](Campaign& c) { c.el.addYou(TOWN, G(c, 4)); c.el.addYou(OLD, G(c, 3)); return S("traders nod along"); }}}},
        {"Your opponents call you an outsider. Your answer?", "Prakash smirks. Rao takes notes.",
         {{"\"I grew up in a Purana Shahar mohalla.\"", [](Campaign& c) { c.el.addYou(OLD, G(c, 5)); c.hype += 3; return S("the old city claims you"); }},
          {"\"I am an outsider only to corruption.\"", [](Campaign& c) { c.hype += 6; c.el.addYou(TOWN, G(c, 3)); return S("your line becomes a headline"); }},
          {"List the sitting MP's ten years of failures", [](Campaign& c) { c.el.hit(PD, -1, 3); c.hype += 4; return S("Prakash stumbles on his reply"); }}}},
        {"Water or roads: what comes first?", "A final question from a student in the back row.",
         {{"Water. Always water.", [](Campaign& c) { c.el.addYou(GAON, G(c, 5)); c.el.addYou(OLD, G(c, 4)); return S("villages and the old city agree"); }},
          {"Roads, because they bring everything else", [](Campaign& c) { c.el.addYou(TOWN, G(c, 5)); c.el.addYou(GAON, G(c, 2)); return S("the township applauds"); }},
          {"Both, in my first hundred days", [](Campaign& c) { c.everywhere(2); c.rep -= 3; return S("big applause, raised eyebrows"); }}}},
    };
    return D;
}

}  // namespace seat
