// Static game data: the constituency, issues, rivals and voter groups. All fictional.
#pragma once
#include "Common.hpp"

namespace seat {

struct RegionDef {
    const char* name;
    const char* sub;
    int voters;
    float turnout;
    float blocs[NBLOC];
    int issues[NISSUE];
    float init[NCAND];
};
extern const RegionDef REGIONS[NREGION];

struct IssueDef {
    const char* label;
    const char* promise;
    std::vector<std::string> complaints;  // what citizens say
    std::string rallyLine;                // what you can say on stage
};
extern const IssueDef ISSUES[NISSUE];

extern const char* BLOC_NAMES[NBLOC];

struct RivalDef {
    const char* name;
    const char* party;
    const char* shortName;
    float str;
    float turn;
    float aff[NREGION];
    std::vector<std::string> verbs;
    std::vector<std::string> heckles;  // what their supporters shout at you
    std::vector<std::string> defend;   // what their supporters say if you attack them
    std::string attackLine;            // rally line attacking them
    std::string record;                // short description
};
extern const RivalDef RIVALS[NCAND];  // index 1..3 used

struct GroupDef {
    const char* name;
    Bloc bloc;
    Issue wants;
};
constexpr int NGROUP = 6;
extern const GroupDef GROUPS[NGROUP];

struct BgDef {
    const char* label;
    const char* sub;
    float rep, funds, hype;
};
extern const BgDef BGS[4];
enum Background { BG_ACTIVIST, BG_BUSINESS, BG_STAR, BG_IAS };

struct DiffDef {
    const char* label;
    const char* sub;
    float D;
};
extern const DiffDef DIFFS[3];

constexpr int NSYMBOL = 6;
extern const char* SYMBOL_NAMES[NSYMBOL];
constexpr float EC_LIMIT = 95.f;

extern const std::vector<std::string> MALE_NAMES, FEMALE_NAMES;
extern const std::vector<std::string> JOBS_BY_BLOC[NBLOC];
extern const char* INFLUENCER_JOB[NBLOC];
extern const std::vector<std::string> SHOP_SIGNS[NREGION];

}  // namespace seat
