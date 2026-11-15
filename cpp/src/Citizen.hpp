// Individual voters you meet on the street: who they are, what they want, what they remember,
// and how they respond to what you say.
#pragma once
#include "Campaign.hpp"

namespace seat {

enum Pose { PO_STAND, PO_CHEER, PO_WAVE, PO_NAMASTE, PO_CROSSED, PO_SELFIE, PO_TALK, PO_CLAP };
enum Emote { EM_NONE, EM_HAPPY, EM_ANGRY, EM_HEART, EM_THINK, EM_CLAP };

struct Look {
    Color skin, top, bottom, hair, accent;
    int hairStyle = 0;  // 0 short, 1 long, 2 balding, 3 bun
    bool female = false, saree = false, turban = false, glasses = false, beard = false, kurta = false, cap = false, scarf = false;
    float height = 1.f, girth = 1.f;
};
Look randomLook(bool female, int age, Bloc bloc);
Look candidateLook(int cand);  // the politician: white kurta and party scarf
Look workerLook(int cand);     // party volunteer in a party-colour T-shirt

struct Citizen {
    std::string name, job;
    int age = 30;
    bool female = false;
    Bloc bloc = YOUTH;
    Issue issue = JOBS;
    int region = 0;
    float lean[NCAND]{};  // private preference
    float support = 0;     // -1 hostile .. +1 committed
    bool influencer = false, heckler = false, hecklerDone = false;
    float influence = 1;
    int talks = 0, greets = 0;
    bool privatePromise = false, convinced = false, selfie = false;
    Look look;
    // movement and animation
    float x = 0, y = 600, targetX = 0, speed = 40, phase = 0, idle = 0;
    int facing = 1;
    int pose = PO_STAND;
    float poseT = 0;
    std::string bubble;
    float bubbleT = 0;
    Color bubbleCol = WHITE;
    int emote = EM_NONE;
    float emoteT = 0;

    int favourite() const;  // candidate they lean to most
    int mood() const { return support > .25f ? 1 : (support < -.25f ? -1 : 0); }
    void say(const std::string& s, float secs = 3.5f, Color c = WHITE) {
        bubble = s;
        bubbleT = secs;
        bubbleCol = c;
    }
    void react(int e, int p, float secs = 2.5f) {
        emote = e;
        emoteT = secs;
        pose = p;
        poseT = secs;
    }
};

std::vector<Citizen> generateCitizens(int region, const Campaign& c, int count, float streetW);

enum TalkId { TK_LISTEN, TK_PROMISE, TK_ATTACK, TK_VOTE, TK_SELFIE, TK_RECRUIT };
enum HeckleId { HK_CALM, HK_SHOUT, HK_CHAI, HK_IGNORE };
struct TalkOption {
    std::string label, sub;
    int id;
};
struct Outcome {
    float base = 0;  // effect on the area's support, before your multiplier
    std::string reply;
    int emote = EM_NONE, pose = PO_STAND;
    std::string feedCtx;
};

std::string opener(Citizen& z, Campaign& c);  // first thing they say (may react to memory)
std::vector<TalkOption> talkOptions(const Citizen& z, const Campaign& c);
Outcome resolveTalk(Citizen& z, Campaign& c, int id);
Outcome greet(Citizen& z, Campaign& c);
std::vector<TalkOption> heckleOptions();
Outcome resolveHeckle(Citizen& z, Campaign& c, int id);
void applyOutcome(Citizen& z, Campaign& c, const Outcome& o);  // region effect + reaction

}  // namespace seat
