// The game: screens, input, the street simulation and everything on screen.
#pragma once
#include "Audio.hpp"
#include "Events.hpp"
#include "Render.hpp"
#include "Ui.hpp"

namespace seat {

enum Screen { SC_TITLE, SC_SETUP, SC_MORNING, SC_OFFICE, SC_STREET, SC_NIGHT, SC_POLL, SC_COUNT, SC_RESULT };

struct DlgOpt {
    std::string label, sub;
    bool enabled = true;
    std::function<std::string()> fn;  // returns a result sentence, or "" for none
    bool primary = false;
};
struct Dialog {
    std::string kicker, title, body;
    std::vector<DlgOpt> opts;
    int citizen = -1;  // index into the current area's citizens: shows their portrait
    bool bars = false;
    Shares barValues{};
    Color accent = Pal::accent;
};
struct Snap {
    float rep, hype, funds, share, stamina;
};

struct Particle {
    Vector2 p, v;
    Color c;
    float life, size;
};
struct FloatText {
    std::string s;
    Vector2 p;
    float life;
    Color c;
};
struct Vehicle {
    float x, speed, y;
    int type;
    bool honked = false;
};
struct CrowdMember {
    Look look;
    Vector2 p;
    float s, phase;
    int pose = PO_STAND;
    float poseT = 0;
    std::string bubble;
    float bubbleT = 0;
};
struct Worker {
    float x = 0, y = 0, phase = 0;
    int facing = 1;
    Look look;
    std::string bubble;
    float bubbleT = 0;
};

class Game {
   public:
    void init();
    void frame();
    void shutdown();
    bool wantsQuit() const { return quit; }

   private:
    Campaign C;
    Screen sc = SC_TITLE;
    float t = 0, dt = 0;
    bool quit = false;
    std::deque<Dialog> dlgs;
    bool showFeed = true, showHelp = false;
    float tickerX = 1280;

    // setup form
    std::string inName = "Ravi Kumar", inParty = "Jan Awaaz Party";
    int inSym = 0, inBg = 0, inDiff = 1, focus = 0;

    // street
    std::vector<Citizen> people[NREGION];
    bool generated[NREGION]{};
    int region = 0;
    float px = 600, py = 620, pphase = 0, camX = 0;
    int pfacing = 1, ppose = PO_STAND;
    float pposeT = 0;
    std::string pbubble;
    float pbubbleT = 0;
    Worker workers[2];
    float chantT = 3, vehT = 1;
    std::vector<Vehicle> vehicles;
    std::vector<Particle> parts;
    std::vector<FloatText> floats;
    std::string nowDoing = "Planning the day at the campaign office";
    int nearIdx = -1;
    enum StreetMode { SM_WALK, SM_RALLY, SM_ROADSHOW } smode = SM_WALK;
    struct RallyState {
        int round = 0, rounds = 4;
        std::vector<SpeechLine> opts;
        unsigned usedMask = 0, usedKinds = 0;
        float sum = 0, reactT = 0, last = 0, meter = .5f, stageX = 0;
        bool bussed = false, finished = false;
        int crowdN = 0;
        std::string line;
        std::vector<CrowdMember> crowd;
        Snap before{};
    } rally;
    struct RoadshowState {
        float x = 0, timer = 0, petalT = 0;
        Snap before{};
    } road;
    int talksAtEnter = 0;
    void updateWorld();
    void leaveRegion(bool toOffice);

    // office
    int mapHover = -1;

    // polling and counting
    Count count;
    bool autoCount = false;
    float autoT = 0, tween = 1, evmT = 99;

    // flow
    Snap snap() const;
    void push(const Dialog& d) { dlgs.push_back(d); }
    void info(const std::string& kicker, const std::string& title, const std::string& body, std::function<std::string()> after = nullptr);
    Dialog resultDialog(const std::string& msg, const Snap& before);
    void drawDialog();
    void afterAction();

    // screens
    void title();
    void setup();
    void morning();
    void office();
    void street();
    void nightScreen();
    void polling();
    void counting();
    void results();

    // shared pieces
    void drawHud();
    void drawFeed(Rectangle r);
    void drawTicker();
    void drawHelp();
    void drawCandidateAt(Vector2 feet, float s, int pose, bool walking, float phase, int facing);

    // campaign flow
    void startCampaign();
    void endDayNow();
    void queueMorningEvents();
    void eventDialog(const EventDef& e);
    void pollDialog();
    void debateDialog(size_t q, std::string said);
    void runAction(const std::function<std::string()>& fn);

    // street
    void enterRegion(int r);
    void updateStreet();
    void drawStreet();
    void streetActions();
    void openTalk(int idx);
    void openHeckle(int idx);
    void contagion(int idx, float base);
    void floatText(const std::string& s, Vector2 p, Color c);
    void petals(Vector2 at, int n);
    void startRally(bool bussed);
    void updateRally();
    void drawRallyUi();
    void finishRally();
    void startRoadshow();
    void updateRoadshow();
};

int runSelfTest(int games);

}  // namespace seat
