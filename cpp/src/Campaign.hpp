// Campaign logic: the player's resources, actions, rivals, public reactions and results.
// No drawing code lives here, so the same logic drives both the game and the --selftest simulator.
#pragma once
#include "Election.hpp"

namespace seat {

struct LogLine {
    int day;
    std::string text;
    Tone tone;
};
struct Post {
    std::string handle, text;
    int likes;
    Tone tone;
    int day;
};
struct DiaryLine {
    std::string time, text;
};
struct Stats {
    int rallies = 0, roadshows = 0, talks = 0, greets = 0, gifts = 0, press = 0, hecklers = 0, converted = 0, selfies = 0;
};

// A line you can deliver from the rally stage.
enum SpeechKind { SP_ISSUE, SP_ATTACK, SP_EMOTION, SP_FREEBIE, SP_JOKE };
struct SpeechLine {
    std::string text;
    SpeechKind kind;
    int arg;  // issue or rival
};

constexpr float DAY_START = 8 * 60, DAY_END = 22 * 60;

class Campaign {
   public:
    // identity
    std::string name = "Ravi Kumar", party = "Jan Awaaz Party";
    int symbol = 0, bg = 0, diff = 1;

    Election el;
    int day = 1;
    static constexpr int maxDay = 14;
    float minute = DAY_START;
    float funds = 0, spent = 0, rep = 60, hype = 0, stamina = 100;
    int strikes = 0;
    bool promised[NISSUE]{};
    int nPromises = 0;
    bool endorsed[NGROUP]{};
    int nEndorse = 0;
    bool builder = false, bribedInd = false, rain = false;
    int surveyUntil = 0;
    int booth[NREGION]{};
    int visits[NREGION]{};
    int socialUses = 0, offPromises = 0;
    int location = -1;  // -1 = campaign office
    Stats stats;

    std::vector<LogLine> log;
    std::deque<Post> feed;
    std::vector<DiaryLine> diary;  // today's schedule
    std::vector<std::string> used;  // events already seen
    float dayStartShare = 0;
    std::string headline;  // tomorrow's front page

    // polling and results
    RegionResult res[NREGION];
    int votedFor = -1;
    Tally total{};
    int order[NCAND]{};
    long long margin = 0, valid = 0;
    float share = 0;
    int pos = 0;
    enum End { NONE, WIN, LOSE, PETITION, DQ } ended = NONE;

    void start(const std::string& n, const std::string& p, int sym, int background, int difficulty);

    float mult() const;
    float gain(float base, int r, bool fatigue = true);
    bool surveyed() const { return day <= surveyUntil; }
    bool silence() const { return day == maxDay; }
    bool lateFor(float mins) const { return minute + mins > DAY_END + 1; }
    int freePromises() const { return bg == BG_IAS ? 5 : 3; }
    float streetBonus() const { return bg == BG_ACTIVIST ? 1.3f : 1.f; }

    void addLog(const std::string& t, Tone tone);
    void post(const std::string& handle, const std::string& text, Tone tone, int likes = -1);
    void doing(const std::string& what);
    void spendTime(float mins, float staminaPerHour = 3.f);
    void spend(float lakh, bool offbook = false) {
        funds -= lakh;
        if (!offbook) spent += lakh;
    }
    void normalize();
    float yourShare() const { return el.overall(true)[YOU]; }
    void everywhere(float base);

    // public reaction on social media
    void react(const std::string& ctx, int r, float outcome);

    // actions: each returns a sentence describing what happened
    std::string travel(int r);
    std::string rallyFinish(int r, float quality, bool bussed, int crowd);
    std::string roadshow(int r);
    std::string trainBooth(int r);
    std::string gifts(int r);
    std::string social();
    std::string press(int target);
    std::string promise(int issue);
    std::string endorse(int group);
    std::string fundSmall();
    std::string fundBuilder();
    std::string survey();
    std::string rest();
    float smallDonation() const { return std::round((3 + rep / 25.f) * 10.f) / 10.f; }

    // rally speech
    std::vector<SpeechLine> speechOptions(int r, unsigned usedMask);
    float speechReaction(int r, const SpeechLine& s, unsigned usedKinds);
    void speechSideEffects(int r, const SpeechLine& s);
    int crowdSize(int r, bool bussed) const;

    void rivalsAct();
    void endDay();  // rivals act, night passes, next morning
    void computePolling();
    void finishResult();
};

}  // namespace seat
