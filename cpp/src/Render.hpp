// Procedural drawing: people, party symbols, flags, vehicles, and the five street scenes.
#pragma once
#include "Citizen.hpp"

namespace seat::gfx {

constexpr float STREET_W = 3200.f;
constexpr float BASE_Y = 470.f;  // where buildings meet the footpath

void tri(Vector2 a, Vector2 b, Vector2 c, Color col);  // any winding
void drawPerson(const Look& L, Vector2 feet, float s, float walkPhase, bool walking, int facing, int pose, float t, int mood, bool back = false);
void drawEmote(int emote, Vector2 p, float alpha);

int symbolOf(int cand, int playerSymbol);  // 0..5 player choices, 6 gear, 7 candle, 8 kettle, 9 NOTA
void drawSymbol(int id, Vector2 c, float s, bool mono, Color col);
void drawFlag(Vector2 base, float h, Color col, int sym, float t);
void drawPoster(Rectangle r, int cand, int playerSymbol, const std::string& label);

Color skyTop(float minute);
Color skyBottom(float minute);
float night(float minute);  // 0 day .. 1 night
void drawSky(float minute, float t);
void drawFar(int region, float camX, float minute, float t);
// near layer in world coordinates; shares decide whose posters are on the walls
void drawNear(int region, float x0, float x1, float minute, float t, const std::array<float, NCAND>& shares, int playerSymbol, const std::string& yourName);
void drawNightOverlay(float minute);
void drawLights(int region, float x0, float x1, float minute);

void drawVehicle(int type, Vector2 p, int dir, float t);
void drawStage(Vector2 c, Color partyCol, int sym, const std::string& name, const std::string& party, float t);
void drawJeep(Vector2 p, float t);
void drawMap(Rectangle area, const std::array<int, NREGION>& leader, const std::array<float, NREGION>& yours, int hover, int selected, bool exact, float t);
int mapHit(Rectangle area, Vector2 m);

}  // namespace seat::gfx
