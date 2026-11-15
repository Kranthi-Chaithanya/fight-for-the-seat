// Breaking-news events that land on the candidate's desk each morning.
#pragma once
#include "Campaign.hpp"

namespace seat {

struct EventChoice {
    std::string label, sub;
    float cost = 0;   // lakh
    float hours = 0;  // time spent today
    bool offbook = false;
    std::function<bool(Campaign&)> cond;
    std::function<std::string(Campaign&, int)> fn;
};

struct EventDef {
    std::string id, title;
    std::function<bool(Campaign&)> cond;
    std::function<int(Campaign&)> region;
    std::function<std::string(Campaign&, int)> text;
    std::vector<EventChoice> choices;
    std::function<void(Campaign&, int)> onShow;
};

const std::vector<EventDef>& allEvents();
const EventDef* rollEvent(Campaign& c);  // nullptr when the day is quiet

struct DebateAnswer {
    std::string text;
    std::function<std::string(Campaign&)> fx;
};
struct DebateQuestion {
    std::string q, ctx;
    std::vector<DebateAnswer> answers;
};
const std::vector<DebateQuestion>& debate();

}  // namespace seat
