#include "Data.hpp"

namespace seat {

// blocs: FARMERS, TRADERS, YOUTH, WORKERS, WOMEN, MIDDLE
// issues: JOBS, PRICES, WATER, ROADS, FARM, EDUCATION, SAFETY, CORRUPTION
// init: YOU, PD, LKM, IND
const RegionDef REGIONS[NREGION] = {
    {"University Road", "Colleges, coaching centres and PG hostels", 24000, .54f,
     {0, .08f, .60f, 0, .12f, .20f}, {3, 0, 0, 0, 0, 3, 2, 2}, {10, 26, 46, 18}},
    {"New Township", "Gated societies around the IT park", 28000, .50f,
     {0, .10f, .25f, 0, .15f, .50f}, {0, 0, 1, 3, 0, 1, 2, 3}, {10, 34, 44, 12}},
    {"Purana Shahar", "Bazaars and old mohallas", 38000, .61f,
     {0, .35f, .18f, .25f, .22f, 0}, {0, 3, 3, 2, 0, 0, 1, 0}, {8, 38, 22, 32}},
    {"Mill Colony", "Chawls around the shut textile mill", 30000, .63f,
     {0, .10f, .15f, .55f, .20f, 0}, {3, 2, 2, 0, 0, 0, 1, 0}, {8, 30, 24, 38}},
    {"Gaon Belt", "22 villages along the Chandan river", 52000, .69f,
     {.50f, 0, .15f, .10f, .25f, 0}, {1, 0, 3, 2, 3, 0, 0, 0}, {8, 46, 24, 22}},
};

const IssueDef ISSUES[NISSUE] = {
    {"Jobs", "Reopen the textile mill and open a skills centre",
     {"My son has an engineering degree and he delivers parcels.", "The mill shut twelve years ago. Half this colony has no fixed work.",
      "Every boy from my lane has gone to Surat or Pune for work."},
     "The mill chimney will smoke again, and your children will not have to leave for Surat!"},
    {"Price rise", "Cheaper cooking gas and full ration on every card",
     {"Gas cylinder, onions, school fees. Everything costs double.", "My ration card gets half the grain it should.",
      "I run a small shop. Customers buy one packet where they used to buy three."},
     "No family in Chandanpur should have to choose between a gas cylinder and school fees!"},
    {"Drinking water", "Piped water to every home within two years",
     {"Water comes once in three days, and that also at 4 AM.", "The handpump has been dry since Holi.",
      "We buy water from tankers. Twenty litres for thirty rupees."},
     "Water in every tap, every day. Not at 4 AM. At breakfast time!"},
    {"Roads", "Pothole-free roads and a railway overbridge",
     {"My scooter has broken down twice in these potholes.", "The railway crossing stays shut for forty minutes. Ambulances wait.",
      "They re-lay this road every election and it washes away every monsoon."},
     "Potholes are not our destiny. Roads that last, and an overbridge that actually opens!"},
    {"Crop prices", "Guaranteed MSP procurement for wheat and mustard",
     {"Nobody buys my mustard at MSP. The trader pays whatever he likes.", "Canal water never reaches the tail-end fields.",
      "Diesel, fertiliser, seeds: all up. Only my crop price stays the same."},
     "Every grain of your wheat bought at MSP, right at your village gate!"},
    {"Education", "A government degree college with a girls' hostel",
     {"There is no government college here. Girls travel 40 kilometres.", "Coaching fees are eighty thousand a year. Who can pay that?",
      "The paper leaked again. Two years of my preparation, wasted."},
     "A government college right here, so our daughters can study at home!"},
    {"Women's safety", "Streetlights and a women's police helpdesk in every ward",
     {"The streetlights on our lane have been dead for months.", "My daughter will not take the late bus anymore.",
      "Last month a chain snatching happened right outside the temple."},
     "Every lane lit, every daughter safe, and a women's helpdesk in every ward!"},
    {"Corruption", "Publish every rupee of the MP fund (MPLADS) online",
     {"I paid a bribe just to get my own land record.", "Where did the MP fund go? Nobody knows.",
      "The new road was paid for twice. It exists once, barely."},
     "Every rupee of the MP fund online. If I steal, you will see it on your phone!"},
};

const char* BLOC_NAMES[NBLOC] = {"Farmers", "Traders", "Youth", "Workers", "Women", "Middle class"};

const RivalDef RIVALS[NCAND] = {
    {"", "", "", 0, 1, {0, 0, 0, 0, 0}, {}, {}, {}, "", ""},
    {"Gajendra Prakash", "Pragati Dal", "Prakash", 1.25f, 1.06f, {.6f, .8f, 1.1f, .9f, 1.3f},
     {"held a massive rally", "inaugurated a half-built flyover", "handed out ration kits", "led a bike rally", "met the sarpanches"},
     {"What has your party ever done for Chandanpur?", "Prakash-ji built the bus stand. What have you built?", "Outsider! Go back where you came from!"},
     {"Prakash-ji got us the bus stand and the hospital!", "At least he picks up the phone when we call."},
     "Ten years as your MP, and the flyover is still half-built!", "Three-term MP, chasing a fourth"},
    {"Adv. Sunita Rao", "Lok Kalyan Morcha", "Rao", 1.1f, .97f, {1.4f, 1.3f, .8f, .9f, .7f},
     {"led a candle march", "held a town hall", "released a policy paper", "went door to door", "spoke at a college fest"},
     {"Another career politician with no plan!", "Where is your policy paper?", "Rao madam fights our cases for free. What do you do?"},
     {"At least Rao madam reads before she speaks.", "She got the hostel fees reduced. You did nothing."},
     "She came from the city with a briefcase and a slogan. We were born here!", "Lawyer and reformist"},
    {"Bhola Pehelwan", "Independent", "Bhola", .85f, 1.02f, {.5f, .4f, 1.2f, 1.4f, .9f},
     {"staged a wrestling dangal", "threw a community feast", "toured on a motorbike convoy", "held court at the akhada"},
     {"Bhola bhai is the only one who comes when there's trouble!", "Mill Colony belongs to Bhola Pehelwan!", "Try walking here at night without Bhola bhai."},
     {"Bhola bhai paid for my sister's wedding.", "When the police beat us, Bhola bhai came. Not you."},
     "Chandanpur will be ruled by votes, not by lathis!", "Local strongman, wrestler"},
};

const GroupDef GROUPS[NGROUP] = {
    {"Chandanpur Kisan Union", FARMERS, FARM},       {"Vyapari Mandal (traders)", TRADERS, CORRUPTION},
    {"Chhatra Sangh (students)", YOUTH, EDUCATION}, {"Mill Mazdoor Union", WORKERS, JOBS},
    {"Mahila Mandal", WOMEN, SAFETY},                {"RWA Federation", MIDDLE, ROADS},
};

const BgDef BGS[4] = {
    {"Social activist", "Rep 75, " RUPEE "15L. Street work hits 30% harder.", 75, 15, 5},
    {"Businessperson", "Rep 62, " RUPEE "50L war chest.", 62, 50, 5},
    {"Film star", "Rep 55, " RUPEE "28L. Famous; rallies hit harder.", 55, 28, 30},
    {"Retired IAS officer", "Rep 70, " RUPEE "20L. First 5 promises believed.", 70, 20, 0},
};

const DiffDef DIFFS[3] = {
    {"Councillor", "Rivals campaign half-heartedly", 1.0f},
    {"MLA", "A fair, tough fight", 1.6f},
    {"Chanakya", "Rivals are relentless", 2.0f},
};

const char* SYMBOL_NAMES[NSYMBOL] = {"Diya", "Sunflower", "Rocket", "Banyan tree", "Scales", "Bell"};

const std::vector<std::string> MALE_NAMES = {
    "Ramesh", "Suresh", "Imran", "Arjun", "Vikram", "Manoj", "Harish", "Salim", "Rakesh", "Deepak", "Anil", "Joseph",
    "Gurpreet", "Rohit", "Sanjay", "Mukesh", "Ajay", "Farhan", "Kishore", "Babulal", "Naveen", "Iqbal", "Mohan", "Tarun",
    "Dinesh", "Yusuf", "Kartik", "Prem", "Bhupendra", "Aamir"};
const std::vector<std::string> FEMALE_NAMES = {
    "Lakshmi", "Fatima", "Kavita", "Pooja", "Meena", "Anjali", "Rekha", "Shabana", "Priya", "Geeta", "Savitri", "Nisha",
    "Mary", "Harpreet", "Asha", "Neha", "Rubina", "Kamla", "Divya", "Sneha", "Zainab", "Usha", "Radha", "Tanvi"};

const std::vector<std::string> JOBS_BY_BLOC[NBLOC] = {
    {"wheat farmer", "dairy farmer", "farm labourer", "mustard grower", "tractor mechanic"},
    {"kirana shop owner", "cloth merchant", "sweet shop owner", "hardware dealer", "fruit seller"},
    {"engineering student", "coaching aspirant", "delivery rider", "BA student", "gym trainer"},
    {"ex-mill weaver", "construction worker", "auto driver", "power-loom operator", "security guard"},
    {"anganwadi worker", "homemaker", "tailor", "nurse", "beautician"},
    {"software engineer", "bank clerk", "school teacher", "retired colonel", "pharmacist"},
};
const char* INFLUENCER_JOB[NBLOC] = {"village sarpanch", "chai stall owner", "local YouTuber", "union leader",
                                     "self-help group leader", "RWA president"};

const std::vector<std::string> SHOP_SIGNS[NREGION] = {
    {"STAR COACHING CENTRE", "SUNRISE PG FOR BOYS", "XEROX & PRINTOUT", "CAMPUS CANTEEN", "BOOK DEPOT", "CYBER CAFE"},
    {"GREEN VALLEY HEIGHTS", "FRESH MART", "ZUMBA STUDIO", "CAFE 24x7", "PET CLINIC", "SALON & SPA"},
    {"LAXMI KIRANA", "NEW BHARAT TAILORS", "OM MEDICAL", "JAI HIND HARDWARE", "RAJU CHAI STALL", "CHANDAN SWEETS"},
    {"MAZDOOR UNION OFFICE", "RATION SHOP", "CYCLE REPAIR", "PAN SHOP", "MOBILE RECHARGE", "DHABA"},
    {"PANCHAYAT BHAWAN", "SEED & FERTILISER", "MILK COLLECTION", "PRIMARY SCHOOL", "FLOUR MILL", "TEA POINT"},
};

}  // namespace seat
