#include "Render.hpp"
#include "Ui.hpp"

namespace seat::gfx {

static Vector2 V(float x, float y) { return {x, y}; }
static Color mix(Color a, Color b, float t) {
    t = clampf(t, 0, 1);
    return {(unsigned char)lerpf(a.r, b.r, t), (unsigned char)lerpf(a.g, b.g, t), (unsigned char)lerpf(a.b, b.b, t), (unsigned char)lerpf(a.a, b.a, t)};
}

void tri(Vector2 a, Vector2 b, Vector2 c, Color col) {
    float cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (cross > 0) std::swap(b, c);
    DrawTriangle(a, b, c, col);
}
static void quad(Vector2 a, Vector2 b, Vector2 c, Vector2 d, Color col) {
    tri(a, b, c, col);
    tri(a, c, d, col);
}
static void limb(Vector2 a, Vector2 b, float w, Color c) {
    DrawLineEx(a, b, w, c);
    DrawCircleV(b, w * .5f, c);
}

// ---------------------------------------------------------------- people
void drawPerson(const Look& L, Vector2 f, float s, float ph, bool walking, int facing, int pose, float t, int mood, bool back) {
    float h = L.height, g = L.girth;
    float x = f.x, y = f.y;
    float hipY = y - 40 * s * h, shY = y - 68 * s * h, headY = y - 79 * s * h, hr = 9 * s;
    float sw = walking ? std::sin(ph) * .5f : 0.f;
    DrawEllipse((int)x, (int)(y + 1), 15 * s, 4 * s, Fade(BLACK, .22f));

    // hair behind head (long hair)
    if (L.hairStyle == 1 && !L.turban) DrawRectangleRounded({x - hr, headY - 2 * s, hr * 2, shY - headY + 12 * s}, .5f, 6, L.hair);

    // legs or skirt
    if (L.saree || (L.female && L.kurta)) {
        Color sk = L.saree ? L.top : L.bottom;
        float spread = 2 * s + std::fabs(sw) * 6 * s;
        quad(V(x - 10 * s * g, hipY - 2 * s), V(x + 10 * s * g, hipY - 2 * s), V(x + 13 * s + spread, y - 3 * s), V(x - 13 * s - spread, y - 3 * s), sk);
        DrawCircleV(V(x - 5 * s + sw * 6 * s, y - 1 * s), 3 * s, Color{40, 30, 30, 255});
        DrawCircleV(V(x + 5 * s - sw * 6 * s, y - 1 * s), 3 * s, Color{40, 30, 30, 255});
    } else {
        float ll = 40 * s * h;
        Vector2 hl = V(x - 4 * s, hipY), hrp = V(x + 4 * s, hipY);
        Vector2 fl = V(x - 4 * s + std::sin(sw) * ll * .7f, y - 2 * s), fr = V(x + 4 * s - std::sin(sw) * ll * .7f, y - 2 * s);
        limb(hl, fl, 7 * s, L.bottom);
        limb(hrp, fr, 7 * s, L.bottom);
        DrawEllipse((int)(fl.x + facing * 2 * s), (int)fl.y + 1, 5 * s, 2.5f * s, Color{40, 30, 30, 255});
        DrawEllipse((int)(fr.x + facing * 2 * s), (int)fr.y + 1, 5 * s, 2.5f * s, Color{40, 30, 30, 255});
    }

    // torso
    float torsoBottom = hipY + (L.kurta && !L.female ? 14 * s : 3 * s);
    DrawRectangleRounded({x - 11 * s * g, shY, 22 * s * g, torsoBottom - shY}, .45f, 6, L.top);
    if (L.saree) DrawLineEx(V(x - 10 * s * g, hipY), V(x + 8 * s * g, shY + 1 * s), 6 * s, L.accent);
    if (L.scarf) {
        DrawLineEx(V(x - 6 * s, shY + 1 * s), V(x - 7 * s, hipY - 4 * s), 4.5f * s, L.accent);
        DrawLineEx(V(x + 6 * s, shY + 1 * s), V(x + 7 * s, hipY - 4 * s), 4.5f * s, L.accent);
        DrawLineEx(V(x - 7 * s, shY + 1 * s), V(x + 7 * s, shY + 1 * s), 4.5f * s, L.accent);
    }

    // arms
    Vector2 sl = V(x - 10 * s * g, shY + 3 * s), sr = V(x + 10 * s * g, shY + 3 * s);
    Vector2 front = facing > 0 ? sr : sl, rear = facing > 0 ? sl : sr;
    float aw = 5 * s;
    Color sleeve = L.saree ? L.skin : L.top;
    auto hand = [&](Vector2 p) { DrawCircleV(p, 2.8f * s, L.skin); };
    float wig = std::sin(t * 10) * 3 * s;
    switch (pose) {
        case PO_CHEER:
            limb(sl, V(sl.x - 8 * s, shY - 24 * s + wig), aw, sleeve);
            limb(sr, V(sr.x + 8 * s, shY - 24 * s - wig), aw, sleeve);
            hand(V(sl.x - 8 * s, shY - 24 * s + wig));
            hand(V(sr.x + 8 * s, shY - 24 * s - wig));
            break;
        case PO_WAVE: {
            Vector2 hp = V(front.x + facing * 6 * s + std::sin(t * 9) * 6 * s, shY - 24 * s);
            limb(front, hp, aw, sleeve);
            hand(hp);
            limb(rear, V(rear.x, hipY + 2 * s), aw, sleeve);
            hand(V(rear.x, hipY + 2 * s));
        } break;
        case PO_NAMASTE: {
            Vector2 hp = V(x + facing * 3 * s, shY + 6 * s);
            limb(sl, V(sl.x + 1 * s, shY + 16 * s), aw, sleeve);
            limb(sr, V(sr.x - 1 * s, shY + 16 * s), aw, sleeve);
            limb(V(sl.x + 1 * s, shY + 16 * s), hp, aw, sleeve);
            limb(V(sr.x - 1 * s, shY + 16 * s), hp, aw, sleeve);
            tri(V(hp.x - 2.5f * s, hp.y + 2 * s), V(hp.x + 2.5f * s, hp.y + 2 * s), V(hp.x, hp.y - 6 * s), L.skin);
        } break;
        case PO_CROSSED:
            limb(sl, V(sl.x, shY + 14 * s), aw, sleeve);
            limb(sr, V(sr.x, shY + 14 * s), aw, sleeve);
            DrawLineEx(V(sl.x - 1 * s, shY + 13 * s), V(sr.x + 1 * s, shY + 15 * s), aw * 1.1f, sleeve);
            break;
        case PO_SELFIE: {
            Vector2 hp = V(x + facing * 26 * s, shY - 14 * s);
            limb(front, hp, aw, sleeve);
            hand(hp);
            DrawRectangleRec({hp.x - 3 * s, hp.y - 8 * s, 6 * s, 10 * s}, Color{30, 30, 40, 255});
            limb(rear, V(rear.x, hipY + 2 * s), aw, sleeve);
        } break;
        case PO_TALK: {
            Vector2 hp = V(front.x + facing * 20 * s, shY + 6 * s + std::sin(t * 7) * 4 * s);
            limb(front, hp, aw, sleeve);
            hand(hp);
            limb(rear, V(rear.x, hipY + 2 * s), aw, sleeve);
            hand(V(rear.x, hipY + 2 * s));
        } break;
        case PO_CLAP: {
            float c = std::fabs(std::sin(t * 14)) * 5 * s;
            Vector2 a = V(x + facing * 10 * s - c, shY + 8 * s), b = V(x + facing * 10 * s + c, shY + 8 * s);
            limb(sl, a, aw, sleeve);
            limb(sr, b, aw, sleeve);
            hand(a);
            hand(b);
        } break;
        default: {
            Vector2 hl = V(sl.x - 2 * s + std::sin(-sw) * 7 * s, hipY + 3 * s), hr2 = V(sr.x + 2 * s + std::sin(sw) * 7 * s, hipY + 3 * s);
            limb(sl, hl, aw, sleeve);
            limb(sr, hr2, aw, sleeve);
            hand(hl);
            hand(hr2);
        }
    }

    // head
    float hx = x + facing * 1 * s;
    DrawRectangleRec({hx - 3 * s, headY + 6 * s, 6 * s, 6 * s}, L.skin);
    DrawCircleV(V(hx, headY), hr, L.skin);
    if (back && L.hairStyle != 2 && !L.turban) DrawCircleV(V(hx, headY), hr + .5f * s, L.hair);
    if (L.turban) {
        DrawEllipse((int)hx, (int)(headY - hr * .55f), hr * 1.25f, hr * .8f, L.accent);
        DrawLineEx(V(hx - hr, headY - hr * .6f), V(hx + hr, headY - hr * .9f), 1.5f * s, Fade(BLACK, .2f));
    } else if (L.cap) {
        DrawCircleSector(V(hx, headY - 1 * s), hr + 1 * s, 180, 360, 12, L.accent);
        DrawRectangleRec({facing > 0 ? hx : hx - hr - 6 * s, headY - 2 * s, hr + 6 * s, 2.5f * s}, L.accent);
    } else {
        switch (L.hairStyle) {
            case 2: DrawCircleSector(V(hx, headY), hr + .5f * s, 200, 340, 10, L.hair); break;
            case 3:
                DrawCircleSector(V(hx, headY), hr + 1 * s, 180, 360, 12, L.hair);
                DrawCircleV(V(hx - facing * hr, headY - 2 * s), 4 * s, L.hair);
                break;
            default: DrawCircleSector(V(hx, headY), hr + 1 * s, 170, 370, 12, L.hair); break;
        }
    }
    if (!back) {
        Color eye{30, 20, 20, 255};
        float ex = hx + facing * 2.5f * s;
        DrawCircleV(V(ex, headY - 1 * s), 1.2f * s, eye);
        DrawCircleV(V(ex + facing * 4 * s, headY - 1 * s), 1.2f * s, eye);
        Vector2 m = V(hx + facing * 4.5f * s, headY + 4 * s);
        Color lip = ColorBrightness(L.skin, -.45f);
        if (mood > 0) DrawRing(m, 1.7f * s, 2.4f * s, 20, 160, 8, lip);
        else if (mood < 0) DrawRing(V(m.x, m.y + 2.2f * s), 1.8f * s, 2.4f * s, 220, 320, 8, lip);
        else DrawLineEx(V(m.x - 1.6f * s, m.y), V(m.x + 1.6f * s, m.y), .9f * s, lip);
        if (L.glasses) {
            DrawCircleLinesV(V(ex, headY - 1 * s), 2.4f * s, Color{30, 30, 30, 255});
            DrawCircleLinesV(V(ex + facing * 4 * s, headY - 1 * s), 2.4f * s, Color{30, 30, 30, 255});
        }
        if (L.beard) DrawCircleSector(V(hx + facing * 1 * s, headY + 1 * s), hr * .95f, 20, 160, 10, Fade(L.hair, .9f));
    }
}

void drawEmote(int e, Vector2 p, float a) {
    if (e == EM_NONE || a <= 0) return;
    Color bg = Fade(WHITE, a * .95f);
    DrawCircleV(p, 11, bg);
    switch (e) {
        case EM_HAPPY:
            DrawCircleV(p, 8, Fade(Color{250, 200, 60, 255}, a));
            DrawRing(V(p.x, p.y + 1), 3, 4.5f, 20, 160, 8, Fade(Color{90, 50, 20, 255}, a));
            DrawCircleV(V(p.x - 3, p.y - 2), 1.2f, Fade(BLACK, a));
            DrawCircleV(V(p.x + 3, p.y - 2), 1.2f, Fade(BLACK, a));
            break;
        case EM_ANGRY:
            DrawCircleV(p, 8, Fade(Color{235, 80, 60, 255}, a));
            DrawLineEx(V(p.x - 5, p.y - 4), V(p.x - 1, p.y - 2), 1.6f, Fade(BLACK, a));
            DrawLineEx(V(p.x + 5, p.y - 4), V(p.x + 1, p.y - 2), 1.6f, Fade(BLACK, a));
            DrawRing(V(p.x, p.y + 5), 2.5f, 3.8f, 200, 340, 8, Fade(BLACK, a));
            break;
        case EM_HEART:
            DrawCircleV(V(p.x - 3.2f, p.y - 2), 4, Fade(Color{230, 50, 90, 255}, a));
            DrawCircleV(V(p.x + 3.2f, p.y - 2), 4, Fade(Color{230, 50, 90, 255}, a));
            tri(V(p.x - 7, p.y - 1), V(p.x + 7, p.y - 1), V(p.x, p.y + 7), Fade(Color{230, 50, 90, 255}, a));
            break;
        case EM_THINK: ui::textCentered("?", p.x, p.y - 11, 20, Fade(Color{80, 70, 120, 255}, a), true); break;
        case EM_CLAP: ui::textCentered("!!", p.x, p.y - 11, 18, Fade(Color{40, 140, 80, 255}, a), true); break;
    }
}

// ---------------------------------------------------------------- symbols and flags
int symbolOf(int cand, int ps) {
    switch (cand) {
        case YOU: return ps;
        case PD: return 6;
        case LKM: return 7;
        case IND: return 8;
        default: return 9;
    }
}

void drawSymbol(int id, Vector2 c, float s, bool mono, Color col) {
    auto C = [&](Color natural) { return mono ? col : natural; };
    switch (id) {
        case 0: {  // diya
            DrawCircleSector(V(c.x, c.y + .05f * s), .5f * s, 0, 180, 16, C({190, 100, 40, 255}));
            DrawRectangleRec({c.x - .5f * s, c.y, s, .08f * s}, C({150, 70, 30, 255}));
            DrawCircleV(V(c.x, c.y - .2f * s), .15f * s, C({255, 190, 40, 255}));
            tri(V(c.x, c.y - .72f * s), V(c.x - .15f * s, c.y - .22f * s), V(c.x + .15f * s, c.y - .22f * s), C({255, 140, 30, 255}));
        } break;
        case 1: {  // sunflower
            if (!mono) DrawLineEx(V(c.x, c.y), V(c.x, c.y + .7f * s), .08f * s, {70, 140, 60, 255});
            for (int i = 0; i < 12; ++i) {
                float a = i * 30 * DEG2RAD;
                DrawCircleV(V(c.x + std::cos(a) * .36f * s, c.y - .1f * s + std::sin(a) * .36f * s), .15f * s, C({250, 200, 40, 255}));
            }
            DrawCircleV(V(c.x, c.y - .1f * s), .24f * s, mono ? Fade(BLACK, .35f) : Color{110, 60, 20, 255});
        } break;
        case 2: {  // rocket
            tri(V(c.x - .17f * s, c.y + .25f * s), V(c.x - .38f * s, c.y + .5f * s), V(c.x - .17f * s, c.y), C({220, 60, 50, 255}));
            tri(V(c.x + .17f * s, c.y + .25f * s), V(c.x + .38f * s, c.y + .5f * s), V(c.x + .17f * s, c.y), C({220, 60, 50, 255}));
            DrawRectangleRounded({c.x - .17f * s, c.y - .42f * s, .34f * s, .82f * s}, .5f, 8, C({230, 230, 240, 255}));
            tri(V(c.x, c.y - .78f * s), V(c.x - .17f * s, c.y - .36f * s), V(c.x + .17f * s, c.y - .36f * s), C({220, 60, 50, 255}));
            DrawCircleV(V(c.x, c.y - .12f * s), .08f * s, mono ? Fade(BLACK, .35f) : Color{70, 140, 220, 255});
            if (!mono) tri(V(c.x - .1f * s, c.y + .4f * s), V(c.x + .1f * s, c.y + .4f * s), V(c.x, c.y + .72f * s), {255, 160, 30, 255});
        } break;
        case 3: {  // banyan
            DrawRectangleRec({c.x - .08f * s, c.y - .1f * s, .16f * s, .62f * s}, C({110, 70, 40, 255}));
            for (float dx : {-.38f, -.2f, .22f, .4f}) DrawLineEx(V(c.x + dx * s, c.y - .05f * s), V(c.x + dx * s, c.y + .45f * s), .03f * s, C({110, 70, 40, 255}));
            DrawCircleV(V(c.x - .3f * s, c.y - .2f * s), .3f * s, C({60, 140, 70, 255}));
            DrawCircleV(V(c.x + .3f * s, c.y - .2f * s), .3f * s, C({60, 140, 70, 255}));
            DrawCircleV(V(c.x, c.y - .42f * s), .34f * s, C({80, 160, 80, 255}));
        } break;
        case 4: {  // scales
            Color k = C({200, 160, 60, 255});
            DrawLineEx(V(c.x, c.y - .6f * s), V(c.x, c.y + .45f * s), .07f * s, k);
            DrawRectangleRec({c.x - .25f * s, c.y + .42f * s, .5f * s, .08f * s}, k);
            DrawLineEx(V(c.x - .55f * s, c.y - .45f * s), V(c.x + .55f * s, c.y - .45f * s), .06f * s, k);
            for (float sx : {-1.f, 1.f}) {
                Vector2 pc = V(c.x + sx * .5f * s, c.y + .02f * s);
                DrawLineEx(V(c.x + sx * .55f * s, c.y - .45f * s), V(pc.x - .2f * s, pc.y), .025f * s, k);
                DrawLineEx(V(c.x + sx * .55f * s, c.y - .45f * s), V(pc.x + .2f * s, pc.y), .025f * s, k);
                DrawCircleSector(pc, .22f * s, 0, 180, 12, k);
            }
        } break;
        case 5: {  // bell
            Color k = C({220, 170, 50, 255});
            DrawCircleSector(V(c.x, c.y - .12f * s), .34f * s, 180, 360, 16, k);
            quad(V(c.x - .34f * s, c.y - .13f * s), V(c.x + .34f * s, c.y - .13f * s), V(c.x + .5f * s, c.y + .3f * s), V(c.x - .5f * s, c.y + .3f * s), k);
            DrawCircleV(V(c.x, c.y + .4f * s), .1f * s, k);
            DrawCircleV(V(c.x, c.y - .5f * s), .07f * s, k);
        } break;
        case 6: {  // gear
            Color k = C({120, 130, 140, 255});
            for (int i = 0; i < 8; ++i) {
                float a = i * 45.f;
                DrawPoly(V(c.x + std::cos(a * DEG2RAD) * .4f * s, c.y + std::sin(a * DEG2RAD) * .4f * s), 4, .13f * s, a + 45, k);
            }
            DrawRing(c, .14f * s, .38f * s, 0, 360, 32, k);
        } break;
        case 7: {  // candle
            DrawRectangleRec({c.x - .15f * s, c.y - .25f * s, .3f * s, .75f * s}, C({245, 235, 215, 255}));
            DrawLineEx(V(c.x, c.y - .25f * s), V(c.x, c.y - .36f * s), .03f * s, C({40, 40, 40, 255}));
            DrawCircleV(V(c.x, c.y - .44f * s), .1f * s, C({255, 190, 40, 255}));
            tri(V(c.x, c.y - .75f * s), V(c.x - .1f * s, c.y - .44f * s), V(c.x + .1f * s, c.y - .44f * s), C({255, 140, 30, 255}));
        } break;
        case 8: {  // kettle
            Color k = C({200, 80, 70, 255});
            DrawLineEx(V(c.x + .28f * s, c.y + .08f * s), V(c.x + .62f * s, c.y - .25f * s), .11f * s, k);
            DrawRing(V(c.x - .02f * s, c.y - .14f * s), .26f * s, .34f * s, 180, 360, 16, k);
            DrawCircleV(V(c.x, c.y + .12f * s), .38f * s, k);
            DrawRectangleRec({c.x - .3f * s, c.y + .38f * s, .6f * s, .08f * s}, k);
            DrawCircleV(V(c.x, c.y - .28f * s), .08f * s, k);
        } break;
        default: {
            Color k = C({120, 120, 130, 255});
            DrawLineEx(V(c.x - .4f * s, c.y - .4f * s), V(c.x + .4f * s, c.y + .4f * s), .14f * s, k);
            DrawLineEx(V(c.x + .4f * s, c.y - .4f * s), V(c.x - .4f * s, c.y + .4f * s), .14f * s, k);
        }
    }
}

void drawFlag(Vector2 b, float h, Color col, int sym, float t) {
    DrawLineEx(b, V(b.x, b.y - h), 3, Color{120, 90, 60, 255});
    float fw = 46, fh = 30, top = b.y - h;
    int strips = 8;
    for (int i = 0; i < strips; ++i) {
        float x0 = b.x + i * fw / strips;
        float off = std::sin(t * 6 + i * .7f) * 3 * (i / (float)strips);
        DrawRectangleRec({x0, top + off, fw / strips + 1, fh}, col);
    }
    drawSymbol(sym, V(b.x + fw / 2, top + fh / 2 + std::sin(t * 6 + 3) * 1.5f), 20, true, WHITE);
}

void drawPoster(Rectangle r, int cand, int ps, const std::string& label) {
    DrawRectangleRec(r, flagColor(cand));
    DrawRectangleLinesEx(r, 2, Fade(WHITE, .7f));
    drawSymbol(symbolOf(cand, ps), V(r.x + r.width / 2, r.y + r.height * .42f), r.width * .5f, true, WHITE);
    ui::textCentered(label, r.x + r.width / 2, r.y + r.height - 16, 12, WHITE, true);
}

// ---------------------------------------------------------------- sky and scenery
struct Key {
    float m;
    Color top, bot;
};
static const Key SKY[] = {
    {0, {12, 14, 34, 255}, {32, 30, 60, 255}},          {330, {20, 24, 60, 255}, {70, 60, 100, 255}},
    {420, {110, 140, 200, 255}, {250, 190, 150, 255}},  {600, {80, 145, 225, 255}, {190, 220, 245, 255}},
    {960, {95, 150, 220, 255}, {220, 222, 232, 255}},   {1110, {70, 60, 140, 255}, {250, 140, 90, 255}},
    {1230, {15, 18, 45, 255}, {45, 40, 80, 255}},       {1440, {12, 14, 34, 255}, {32, 30, 60, 255}}};
static void skyAt(float m, Color& top, Color& bot) {
    m = std::fmod(m, 1440.f);
    for (int i = 0; i < 7; ++i)
        if (m >= SKY[i].m && m <= SKY[i + 1].m) {
            float u = (m - SKY[i].m) / (SKY[i + 1].m - SKY[i].m);
            top = mix(SKY[i].top, SKY[i + 1].top, u);
            bot = mix(SKY[i].bot, SKY[i + 1].bot, u);
            return;
        }
    top = SKY[0].top;
    bot = SKY[0].bot;
}
Color skyTop(float m) { Color a, b; skyAt(m, a, b); return a; }
Color skyBottom(float m) { Color a, b; skyAt(m, a, b); return b; }
float night(float m) {
    if (m < 390) return clampf((390 - m) / 60.f, 0, 1);
    return clampf((m - 1110) / 100.f, 0, 1);
}

void drawSky(float m, float t) {
    Color a, b;
    skyAt(m, a, b);
    DrawRectangleGradientV(0, 0, GetScreenWidth(), (int)BASE_Y + 40, a, b);
    float n = night(m);
    // sun or moon arc
    float u = clampf((m - 360) / (1140 - 360), 0, 1);
    if (n < .9f) {
        Vector2 sp = V(150 + u * 980, 380 - std::sin(u * PI) * 300);
        DrawCircleV(sp, 46, Fade(Color{255, 220, 150, 255}, .25f * (1 - n)));
        DrawCircleV(sp, 26, Fade(Color{255, 236, 190, 255}, 1 - n));
    }
    if (n > .2f) {
        for (int i = 0; i < 70; ++i) {
            float tw = .5f + .5f * std::sin(t * 2 + i);
            DrawCircleV(V(hash01(i, 7) * GetScreenWidth(), hash01(i, 9) * 330), 1.2f, Fade(WHITE, n * tw * .8f));
        }
        DrawCircleV(V(1050, 110), 20, Fade(Color{240, 240, 225, 255}, n));
        DrawCircleV(V(1058, 104), 18, Fade(skyTop(m), n));
    }
}

void drawFar(int region, float camX, float m, float t) {
    float n = night(m);
    Color base = mix(Color{150, 160, 185, 255}, Color{40, 40, 70, 255}, n);
    Color base2 = mix(Color{120, 128, 155, 255}, Color{30, 30, 55, 255}, n);
    float par = .3f, period = 110;
    float off = camX * par;
    int i0 = (int)std::floor(off / period) - 1;
    for (int i = i0; i < i0 + 15; ++i) {
        float x = i * period - off;
        float r1 = hash01(i, region * 31 + 1), r2 = hash01(i, region * 31 + 2);
        switch (region) {
            case GAON: {
                DrawCircleV(V(x + 55, BASE_Y + 60), 120 + r1 * 60, mix(Color{120, 160, 120, 255}, Color{25, 40, 40, 255}, n));
                if (r2 > .5f) {
                    DrawRectangleRec({x + 40, BASE_Y - 70, 8, 70}, base2);
                    DrawCircleV(V(x + 44, BASE_Y - 85), 30, mix(Color{90, 140, 90, 255}, Color{20, 35, 30, 255}, n));
                }
                if (i % 6 == 0) {  // transmission tower
                    tri(V(x + 20, BASE_Y), V(x + 60, BASE_Y), V(x + 40, BASE_Y - 170), Fade(base2, .7f));
                    DrawLineEx(V(x + 10, BASE_Y - 140), V(x + 70, BASE_Y - 140), 3, base2);
                }
            } break;
            case TOWN: {
                float hgt = 160 + r1 * 170;
                DrawRectangleRec({x, BASE_Y - hgt, 80, hgt}, base);
                for (int wy = 0; wy < (int)(hgt / 22) - 1; ++wy)
                    for (int wx = 0; wx < 4; ++wx) {
                        bool lit = hash01(i * 100 + wy * 7 + wx, 3) < .45f;
                        Color wc = n > .3f ? (lit ? Fade(Color{255, 220, 140, 255}, n) : Fade(BLACK, .2f)) : Fade(Color{200, 220, 240, 255}, .5f);
                        DrawRectangleRec({x + 8 + wx * 18, BASE_Y - hgt + 10 + wy * 22, 10, 12}, wc);
                    }
                if (i % 5 == 0) DrawRectangleRec({x + 30, BASE_Y - hgt - 30, 4, 30}, base);
            } break;
            case UNI: {
                float hgt = 90 + r1 * 80;
                DrawRectangleRec({x, BASE_Y - hgt, 100, hgt}, base);
                if (i % 7 == 0) {
                    DrawRectangleRec({x + 20, BASE_Y - hgt - 40, 60, 40}, base2);
                    DrawCircleSector(V(x + 50, BASE_Y - hgt - 40), 30, 180, 360, 16, base2);
                }
                if (i % 9 == 4) {  // clock tower
                    DrawRectangleRec({x + 40, BASE_Y - 260, 26, 260}, base2);
                    DrawCircleV(V(x + 53, BASE_Y - 230), 9, mix(WHITE, Color{255, 230, 160, 255}, n));
                }
            } break;
            case OLD: {
                float hgt = 100 + r1 * 90;
                DrawRectangleRec({x, BASE_Y - hgt, 105, hgt}, base);
                if (r2 > .6f) DrawCircleSector(V(x + 50, BASE_Y - hgt), 22, 180, 360, 12, base2);
                if (i % 8 == 3) {
                    DrawRectangleRec({x + 30, BASE_Y - 240, 30, 240}, base2);
                    tri(V(x + 26, BASE_Y - 240), V(x + 64, BASE_Y - 240), V(x + 45, BASE_Y - 275), base2);
                }
                if (i % 6 == 1) {  // water tank
                    DrawRectangleRec({x + 60, BASE_Y - hgt - 50, 6, 50}, base2);
                    DrawRectangleRec({x + 80, BASE_Y - hgt - 50, 6, 50}, base2);
                    DrawEllipse((int)x + 73, (int)(BASE_Y - hgt - 60), 26, 18, base2);
                }
            } break;
            case MILL: {
                float hgt = 70 + r1 * 50;
                // saw-tooth sheds
                DrawRectangleRec({x, BASE_Y - hgt, 110, hgt}, base);
                for (int k = 0; k < 3; ++k) tri(V(x + k * 36, BASE_Y - hgt), V(x + k * 36 + 36, BASE_Y - hgt), V(x + k * 36 + 36, BASE_Y - hgt - 22), base2);
                if (i % 5 == 0) {
                    DrawRectangleRec({x + 50, BASE_Y - 300, 22, 300}, base2);
                    for (int k = 0; k < 4; ++k) {
                        float u = std::fmod(t * .15f + k * .25f, 1.f);
                        DrawCircleV(V(x + 61 + u * 60, BASE_Y - 310 - u * 90), 10 + u * 20, Fade(Color{200, 200, 200, 255}, .35f * (1 - u) * (1 - n * .5f)));
                    }
                }
            } break;
        }
    }
}

static void drawShopFront(int region, int i, float x, float w, float m, const std::array<float, NCAND>& sh, int ps, const std::string& yourName) {
    float n = night(m);
    float hgt = 150 + hash01(i, region * 17 + 5) * 130;
    static const Color WALLS[NREGION][3] = {
        {{214, 196, 170, 255}, {190, 170, 150, 255}, {220, 210, 190, 255}},
        {{225, 225, 230, 255}, {200, 205, 215, 255}, {235, 230, 220, 255}},
        {{224, 170, 120, 255}, {200, 120, 110, 255}, {120, 170, 175, 255}},
        {{170, 160, 150, 255}, {150, 140, 130, 255}, {190, 175, 150, 255}},
        {{200, 160, 110, 255}, {190, 150, 100, 255}, {210, 175, 125, 255}}};
    Color wall = mix(WALLS[region][(int)(hash01(i, 11) * 2.99f)], Color{40, 38, 60, 255}, n * .7f);
    DrawRectangleRec({x, BASE_Y - hgt, w - 6, hgt}, wall);
    DrawRectangleRec({x, BASE_Y - hgt, w - 6, 6}, Fade(BLACK, .15f));
    // upper windows / balcony
    int floors = (int)((hgt - 90) / 55);
    for (int f = 0; f < floors; ++f) {
        float wy = BASE_Y - 100 - f * 55;
        for (int k = 0; k < 2; ++k) {
            Rectangle win{x + 18 + k * (w - 60) / 1.f * .55f, wy - 30, 30, 34};
            bool lit = hash01(i * 13 + f * 3 + k, 21) < .5f;
            DrawRectangleRec(win, n > .3f && lit ? Fade(Color{255, 210, 130, 255}, .5f + n * .5f) : Color{60, 70, 90, 255});
            DrawRectangleLinesEx(win, 2, Fade(BLACK, .3f));
        }
        if (region == MILL) {  // laundry lines on chawl balconies
            DrawLineEx(V(x + 6, wy - 38), V(x + w - 14, wy - 34), 1, Fade(BLACK, .5f));
            for (int k = 0; k < 4; ++k) DrawRectangleRec({x + 14 + k * 26, wy - 37, 12, 16}, Color{(unsigned char)(120 + 120 * hash01(i + k, f)), (unsigned char)(80 + 150 * hash01(k, i)), 160, 255});
        }
    }
    // shop
    float sy = BASE_Y - 70;
    DrawRectangleRec({x + 8, sy, w - 22, 70}, Color{50, 45, 50, 255});
    for (int k = 0; k < 7; ++k) DrawLineEx(V(x + 8, sy + 8 + k * 9), V(x + w - 14, sy + 8 + k * 9), 1, Fade(GRAY, .35f));
    if (hash01(i, 33) < .7f) DrawRectangleRec({x + 14, sy + 12, w - 34, 58}, mix(Color{240, 200, 120, 255}, Color{255, 200, 110, 255}, n));
    // awning
    Color aw1 = Color{(unsigned char)(180 + 70 * hash01(i, 40)), (unsigned char)(60 + 120 * hash01(i, 41)), 60, 255};
    for (int k = 0; k < 6; ++k) {
        float ax = x + 4 + k * (w - 14) / 6;
        quad(V(ax, sy - 6), V(ax + (w - 14) / 6, sy - 6), V(ax + (w - 14) / 6 + 4, sy + 12), V(ax + 4, sy + 12), k % 2 ? aw1 : Color{240, 235, 225, 255});
    }
    // signboard
    const std::string& sign = SHOP_SIGNS[region][(int)(hash01(i, 50) * SHOP_SIGNS[region].size()) % SHOP_SIGNS[region].size()];
    Rectangle sb{x + 8, sy - 34, w - 22, 26};
    DrawRectangleRec(sb, Color{(unsigned char)(30 + 60 * hash01(i, 52)), (unsigned char)(60 + 80 * hash01(i, 53)), 120, 255});
    float fs = 13;
    while (fs > 8 && ui::measure(sign, fs, true).x > sb.width - 8) fs -= 1;
    ui::textCentered(sign, sb.x + sb.width / 2, sb.y + 5, fs, Color{255, 240, 200, 255}, true);
    // campaign posters, weighted by who is strong here
    int nposters = (int)(hash01(i, 60) * 3);
    for (int k = 0; k < nposters; ++k) {
        float roll = hash01(i * 7 + k, 61) * 100, acc = 0;
        int cand = 0;
        for (int c = 0; c < NCAND; ++c) {
            acc += sh[c];
            if (roll <= acc) { cand = c; break; }
        }
        Rectangle pr{x + 14 + k * 46, BASE_Y - hgt + 30 + hash01(i, 62 + k) * 20, 40, 52};
        if (pr.y + pr.height > sy - 40) continue;
        drawPoster(pr, cand, ps, cand == YOU ? yourName.substr(0, std::min<size_t>(8, yourName.size())) : RIVALS[cand].shortName);
    }
}

static void drawHut(float x, float m) {
    float n = night(m);
    Color wall = mix(Color{170, 120, 80, 255}, Color{50, 40, 50, 255}, n * .7f);
    DrawRectangleRec({x, BASE_Y - 80, 120, 80}, wall);
    tri(V(x - 15, BASE_Y - 78), V(x + 135, BASE_Y - 78), V(x + 60, BASE_Y - 140), mix(Color{190, 160, 90, 255}, Color{60, 50, 40, 255}, n * .7f));
    DrawRectangleRec({x + 45, BASE_Y - 50, 28, 50}, Color{70, 45, 30, 255});
}

void drawNear(int region, float x0, float x1, float m, float t, const std::array<float, NCAND>& sh, int ps, const std::string& yourName) {
    float n = night(m);
    // ground
    Color foot = mix(region == GAON ? Color{170, 140, 100, 255} : Color{185, 180, 175, 255}, Color{45, 42, 55, 255}, n * .75f);
    Color road = mix(region == GAON ? Color{150, 115, 80, 255} : Color{85, 85, 92, 255}, Color{30, 30, 40, 255}, n * .75f);
    Color near = mix(region == GAON ? Color{165, 135, 95, 255} : Color{160, 155, 150, 255}, Color{40, 38, 50, 255}, n * .75f);
    DrawRectangleRec({x0 - 10, BASE_Y, x1 - x0 + 20, 35}, foot);
    DrawRectangleRec({x0 - 10, BASE_Y + 35, x1 - x0 + 20, 90}, road);
    DrawRectangleRec({x0 - 10, BASE_Y + 125, x1 - x0 + 20, 140}, near);
    if (region != GAON)
        for (float x = std::floor(x0 / 80) * 80; x < x1; x += 80) DrawRectangleRec({x, BASE_Y + 78, 40, 4}, Fade(WHITE, .6f - n * .3f));
    if (region == TOWN || region == OLD)  // potholes
        for (int i = 0; i < 14; ++i) {
            float px = hash01(i, 90) * STREET_W;
            if (px > x0 - 50 && px < x1 + 50) DrawEllipse((int)px, (int)(BASE_Y + 50 + hash01(i, 91) * 60), 18, 6, Fade(BLACK, .45f));
        }

    if (region == GAON) {
        for (int i = 0; i < 16; ++i) {
            float x = i * 210.f + 20;
            if (x + 200 < x0 || x > x1) continue;
            float r = hash01(i, 120);
            if (r < .4f) drawHut(x, m);
            else if (r < .6f) {  // banyan tree
                DrawRectangleRec({x + 60, BASE_Y - 140, 26, 140}, Color{100, 70, 45, 255});
                for (int k = 0; k < 5; ++k) DrawLineEx(V(x + 20 + k * 25, BASE_Y - 110), V(x + 22 + k * 25, BASE_Y), 2, Color{110, 80, 50, 255});
                Color leaf = mix(Color{70, 140, 70, 255}, Color{20, 40, 30, 255}, n);
                DrawCircleV(V(x + 40, BASE_Y - 150), 60, leaf);
                DrawCircleV(V(x + 110, BASE_Y - 150), 60, leaf);
                DrawCircleV(V(x + 75, BASE_Y - 195), 65, ColorBrightness(leaf, .1f));
            } else if (r < .8f) {  // field with crop rows
                for (int k = 0; k < 8; ++k) DrawLineEx(V(x + k * 24, BASE_Y - 2), V(x + k * 24 + 10, BASE_Y - 30), 4, mix(Color{220, 190, 60, 255}, Color{60, 60, 40, 255}, n));
                DrawCircleSector(V(x + 170, BASE_Y), 30, 180, 360, 12, mix(Color{220, 190, 100, 255}, Color{60, 55, 40, 255}, n));
            } else {  // panchayat building
                drawShopFront(region, i, x, 190, m, sh, ps, yourName);
            }
            if (i % 4 == 1) {  // handpump
                DrawRectangleRec({x + 150, BASE_Y - 36, 8, 36}, Color{60, 90, 120, 255});
                DrawLineEx(V(x + 154, BASE_Y - 34), V(x + 180, BASE_Y - 44), 4, Color{60, 90, 120, 255});
            }
            if (i % 3 == 0) {
                Rectangle pr{x + 130, BASE_Y - 70, 34, 44};
                int cand = hash01(i, 130) * 100 < sh[YOU] ? YOU : (hash01(i, 131) < .5f ? PD : LKM);
                drawPoster(pr, cand, ps, cand == YOU ? "" : RIVALS[cand].shortName);
            }
        }
    } else {
        float w = 200;
        for (int i = 0; i < (int)(STREET_W / w); ++i) {
            float x = i * w;
            if (x + w < x0 || x > x1) continue;
            if (region == MILL && i == 8) {  // the shut mill gate
                DrawRectangleRec({x, BASE_Y - 120, w - 6, 120}, Color{110, 100, 95, 255});
                DrawRectangleRec({x + 30, BASE_Y - 95, w - 66, 95}, Color{70, 60, 55, 255});
                for (int k = 0; k < 10; ++k) DrawLineEx(V(x + 34 + k * 13, BASE_Y - 92), V(x + 34 + k * 13, BASE_Y), 3, Color{120, 110, 100, 255});
                ui::textCentered("CHANDANPUR TEXTILE MILLS", x + w / 2, BASE_Y - 116, 13, Color{240, 230, 210, 255}, true);
                ui::textCentered("EST. 1952  " DASH "  CLOSED", x + w / 2, BASE_Y - 100, 11, Color{240, 200, 180, 255}, true);
                continue;
            }
            drawShopFront(region, i, x, w, m, sh, ps, yourName);
        }
    }
    // electric poles with sagging wires
    for (float px = std::floor(x0 / 420) * 420 + 120; px < x1 + 420; px += 420) {
        DrawLineEx(V(px, BASE_Y + 30), V(px, BASE_Y - 230), 6, Color{90, 90, 95, 255});
        DrawLineEx(V(px - 30, BASE_Y - 220), V(px + 30, BASE_Y - 220), 4, Color{90, 90, 95, 255});
        for (int w = -1; w <= 1; ++w) {
            Vector2 prev = V(px + w * 25, BASE_Y - 220);
            for (int k = 1; k <= 10; ++k) {
                float u = k / 10.f;
                Vector2 p = V(px + w * 25 + u * 420, BASE_Y - 220 + std::sin(u * PI) * (24 + w * 4));
                DrawLineEx(prev, p, 1.2f, Fade(BLACK, .7f));
                prev = p;
            }
        }
        if (n > .1f) DrawCircleV(V(px + 30, BASE_Y - 222), 5, Color{255, 230, 160, 255});
    }
    (void)t;
}

void drawNightOverlay(float m) {
    float n = night(m);
    if (n > 0) DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(Color{10, 10, 40, 255}, n * .35f));
}

void drawLights(int region, float x0, float x1, float m) {
    float n = night(m);
    if (n <= 0) return;
    (void)region;
    for (float px = std::floor(x0 / 420) * 420 + 120; px < x1 + 420; px += 420)
        DrawCircleGradient((int)px + 30, (int)BASE_Y - 210, 150, Fade(Color{255, 220, 150, 255}, .22f * n), Fade(Color{255, 220, 150, 255}, 0));
}

// ---------------------------------------------------------------- vehicles, stage, jeep
static void wheel(Vector2 p, float r) {
    DrawCircleV(p, r, Color{30, 30, 30, 255});
    DrawCircleV(p, r * .45f, Color{150, 150, 150, 255});
}

void drawVehicle(int type, Vector2 p, int dir, float t) {
    float d = (float)dir;
    switch (type) {
        case 0: {  // auto-rickshaw
            DrawRectangleRounded({p.x - 34, p.y - 44, 68, 34}, .3f, 6, Color{40, 40, 40, 255});
            DrawRectangleRounded({p.x - 36, p.y - 30, 72, 22}, .3f, 6, Color{80, 160, 60, 255});
            DrawRectangleRec({p.x - 36, p.y - 30, 72, 8}, Color{240, 200, 30, 255});
            DrawRectangleRec({p.x + d * 18 - 8, p.y - 42, 16, 14}, Fade(Color{150, 200, 230, 255}, .7f));
            wheel(V(p.x - 22, p.y - 6), 7);
            wheel(V(p.x + 22, p.y - 6), 7);
            wheel(V(p.x + d * 30, p.y - 6), 7);
        } break;
        case 1: {  // scooter with rider
            DrawRectangleRounded({p.x - 26, p.y - 24, 52, 14}, .5f, 6, Color{200, 40, 40, 255});
            wheel(V(p.x - 18, p.y - 6), 7);
            wheel(V(p.x + 18, p.y - 6), 7);
            DrawLineEx(V(p.x + d * 18, p.y - 24), V(p.x + d * 22, p.y - 44), 3, Color{60, 60, 60, 255});
            DrawRectangleRounded({p.x - 8, p.y - 52, 14, 28}, .5f, 6, Color{60, 100, 160, 255});
            DrawCircleV(V(p.x, p.y - 58), 8, Color{40, 40, 40, 255});
        } break;
        case 2: {  // car
            DrawRectangleRounded({p.x - 50, p.y - 32, 100, 24}, .4f, 6, Color{230, 230, 235, 255});
            DrawRectangleRounded({p.x - 30, p.y - 50, 60, 22}, .4f, 6, Color{230, 230, 235, 255});
            DrawRectangleRec({p.x - 24, p.y - 46, 22, 14}, Fade(Color{120, 160, 200, 255}, .8f));
            DrawRectangleRec({p.x + 2, p.y - 46, 22, 14}, Fade(Color{120, 160, 200, 255}, .8f));
            wheel(V(p.x - 30, p.y - 8), 9);
            wheel(V(p.x + 30, p.y - 8), 9);
        } break;
        case 3: {  // bus
            DrawRectangleRounded({p.x - 90, p.y - 76, 180, 66}, .15f, 6, Color{200, 90, 40, 255});
            DrawRectangleRec({p.x - 90, p.y - 40, 180, 8}, Color{240, 220, 200, 255});
            for (int k = 0; k < 6; ++k) DrawRectangleRec({p.x - 80 + k * 27, p.y - 70, 22, 22}, Fade(Color{150, 190, 220, 255}, .8f));
            wheel(V(p.x - 55, p.y - 8), 11);
            wheel(V(p.x + 55, p.y - 8), 11);
        } break;
        case 4: {  // tractor
            DrawRectangleRec({p.x - 10, p.y - 40, 40, 22}, Color{200, 50, 40, 255});
            DrawRectangleRec({p.x - 30, p.y - 56, 22, 30}, Color{200, 50, 40, 255});
            DrawLineEx(V(p.x + 20, p.y - 40), V(p.x + 20, p.y - 60), 3, Color{50, 50, 50, 255});
            DrawCircleV(V(p.x - 20, p.y - 18), 18, Color{30, 30, 30, 255});
            DrawCircleV(V(p.x - 20, p.y - 18), 8, Color{220, 180, 40, 255});
            wheel(V(p.x + 26, p.y - 9), 9);
        } break;
        default: {  // cow
            Color c{235, 230, 220, 255};
            DrawEllipse((int)p.x, (int)p.y - 32, 30, 14, c);
            for (float lx : {-18.f, -10.f, 12.f, 20.f}) DrawLineEx(V(p.x + lx, p.y - 24), V(p.x + lx + std::sin(t * 3 + lx) * 2, p.y - 2), 4, c);
            DrawEllipse((int)(p.x + d * 32), (int)p.y - 40, 9, 7, c);
            DrawLineEx(V(p.x + d * 32 - 4, p.y - 46), V(p.x + d * 30 - 8, p.y - 54), 2, Color{120, 100, 80, 255});
            DrawLineEx(V(p.x + d * 32 + 4, p.y - 46), V(p.x + d * 34 + 6, p.y - 54), 2, Color{120, 100, 80, 255});
            DrawLineEx(V(p.x - d * 30, p.y - 34), V(p.x - d * 36, p.y - 18 + std::sin(t * 4) * 3), 2, c);
        } break;
    }
}

void drawStage(Vector2 c, Color pc, int sym, const std::string& name, const std::string& party, float t) {
    // backdrop banner
    Rectangle bn{c.x - 230, c.y - 250, 460, 120};
    DrawRectangleRec(bn, pc);
    DrawRectangleLinesEx(bn, 4, Color{250, 200, 60, 255});
    drawSymbol(sym, V(bn.x + 60, bn.y + 60), 70, true, WHITE);
    ui::text(name, bn.x + 115, bn.y + 18, 34, WHITE, true);
    ui::text(party + "  " DOT "  Vote for change", bn.x + 117, bn.y + 66, 18, Color{255, 235, 200, 255}, true);
    for (float px : {bn.x + 10, bn.x + bn.width - 14}) DrawLineEx(V(px, bn.y + bn.height), V(px, c.y), 5, Color{110, 80, 50, 255});
    // marigold garlands
    for (int g = 0; g < 2; ++g) {
        for (int k = 0; k <= 30; ++k) {
            float u = k / 30.f;
            float gx = bn.x + u * bn.width, gy = bn.y + bn.height + 6 + std::sin(u * PI) * (18 + g * 10) + g * 4;
            DrawCircleV(V(gx, gy), 5, k % 2 ? Color{250, 150, 20, 255} : Color{250, 200, 40, 255});
        }
    }
    // platform
    DrawRectangleRec({c.x - 250, c.y - 40, 500, 40}, Color{120, 85, 55, 255});
    for (int k = 0; k < 10; ++k) DrawRectangleRec({c.x - 250 + k * 50, c.y - 40, 25, 40}, Fade(pc, .85f));
    DrawRectangleRec({c.x - 250, c.y - 44, 500, 6}, Color{250, 200, 60, 255});
    // loudspeaker horns on poles
    for (float sx : {-300.f, 300.f}) {
        DrawLineEx(V(c.x + sx, c.y), V(c.x + sx, c.y - 230), 4, Color{80, 80, 80, 255});
        float dir = sx > 0 ? 1 : -1;
        tri(V(c.x + sx, c.y - 222), V(c.x + sx + dir * 34, c.y - 240 + std::sin(t * 30) * .8f), V(c.x + sx + dir * 34, c.y - 200), Color{150, 150, 155, 255});
    }
    // microphone stand
    DrawLineEx(V(c.x + 40, c.y - 40), V(c.x + 40, c.y - 95), 3, Color{60, 60, 60, 255});
    DrawCircleV(V(c.x + 40, c.y - 98), 5, Color{40, 40, 40, 255});
}

void drawJeep(Vector2 p, float t) {
    (void)t;
    DrawRectangleRounded({p.x - 80, p.y - 50, 160, 36}, .25f, 6, Color{60, 90, 60, 255});
    DrawRectangleRec({p.x + 40, p.y - 78, 6, 30}, Color{50, 50, 50, 255});
    DrawRectangleRec({p.x + 44, p.y - 78, 4, 30}, Fade(Color{150, 200, 230, 255}, .6f));
    for (int k = 0; k < 12; ++k) DrawCircleV(V(p.x - 76 + k * 14, p.y - 50), 5, k % 2 ? Color{250, 150, 20, 255} : Color{250, 210, 50, 255});
    wheel(V(p.x - 50, p.y - 12), 14);
    wheel(V(p.x + 50, p.y - 12), 14);
}

// ---------------------------------------------------------------- constituency map
static const std::vector<Vector2> POLY[NREGION] = {
    {{120, 10}, {250, 10}, {240, 90}, {150, 100}, {110, 70}},
    {{250, 10}, {390, 10}, {390, 130}, {300, 140}, {240, 90}},
    {{150, 100}, {240, 90}, {300, 140}, {270, 200}, {170, 195}, {140, 150}},
    {{300, 140}, {390, 130}, {390, 290}, {260, 290}, {270, 200}},
    {{10, 10}, {120, 10}, {110, 70}, {150, 100}, {140, 150}, {170, 195}, {270, 200}, {260, 290}, {10, 290}}};
static const Vector2 LABEL[NREGION] = {{180, 48}, {318, 62}, {212, 146}, {330, 218}, {72, 170}};

static Vector2 mp(Rectangle a, Vector2 p) { return {a.x + p.x / 400.f * a.width, a.y + p.y / 300.f * a.height}; }

static bool pointIn(const std::vector<Vector2>& P, Vector2 q) {
    bool in = false;
    for (size_t i = 0, j = P.size() - 1; i < P.size(); j = i++)
        if (((P[i].y > q.y) != (P[j].y > q.y)) && (q.x < (P[j].x - P[i].x) * (q.y - P[i].y) / (P[j].y - P[i].y) + P[i].x)) in = !in;
    return in;
}

static void fillPoly(std::vector<Vector2> P, Color c) {  // ear clipping
    auto area = [&](Vector2 a, Vector2 b, Vector2 d) { return (b.x - a.x) * (d.y - a.y) - (b.y - a.y) * (d.x - a.x); };
    float s = 0;
    for (size_t i = 0; i < P.size(); ++i) s += P[i].x * P[(i + 1) % P.size()].y - P[(i + 1) % P.size()].x * P[i].y;
    if (s < 0) std::reverse(P.begin(), P.end());
    int guard = 0;
    while (P.size() > 3 && guard++ < 200) {
        bool cut = false;
        for (size_t i = 0; i < P.size(); ++i) {
            Vector2 a = P[(i + P.size() - 1) % P.size()], b = P[i], d = P[(i + 1) % P.size()];
            if (area(a, b, d) <= 0) continue;
            bool inside = false;
            for (auto& q : P)
                if (!(q.x == a.x && q.y == a.y) && !(q.x == b.x && q.y == b.y) && !(q.x == d.x && q.y == d.y) && pointIn({a, b, d}, q)) inside = true;
            if (inside) continue;
            tri(a, b, d, c);
            P.erase(P.begin() + i);
            cut = true;
            break;
        }
        if (!cut) break;
    }
    if (P.size() == 3) tri(P[0], P[1], P[2], c);
}

void drawMap(Rectangle A, const std::array<int, NREGION>& leader, const std::array<float, NREGION>& yours, int hover, int sel, bool exact, float t) {
    DrawRectangleRounded(A, .03f, 8, Color{36, 31, 50, 255});
    for (int r = 0; r < NREGION; ++r) {
        std::vector<Vector2> P;
        for (auto p : POLY[r]) P.push_back(mp(A, p));
        Color c = flagColor(leader[r]);
        fillPoly(P, Fade(c, hover == r ? .85f : .6f));
        for (size_t i = 0; i < P.size(); ++i) DrawLineEx(P[i], P[(i + 1) % P.size()], sel == r ? 4.f : 2.f, sel == r ? Pal::text : Color{30, 26, 40, 255});
    }
    // river and railway
    Vector2 prev = mp(A, {392, 168});
    for (int k = 1; k <= 24; ++k) {
        float u = k / 24.f;
        Vector2 q = mp(A, {392 - u * 242, 168 + u * 130 + std::sin(u * 6) * 12});
        DrawLineEx(prev, q, 6, Fade(Color{110, 160, 220, 255}, .8f));
        prev = q;
    }
    for (int k = 0; k < 40; k += 2) DrawLineEx(mp(A, {8 + k * 9.6f, 118 - k * .55f}), mp(A, {8 + (k + 1) * 9.6f, 118 - (k + 1) * .55f}), 2, Fade(WHITE, .5f));
    for (int r = 0; r < NREGION; ++r) {
        Vector2 l = mp(A, LABEL[r]);
        ui::textCentered(REGIONS[r].name, l.x, l.y - 12, 17, WHITE, true);
        ui::textCentered(strf("You %s%.0f%%", exact ? "" : "~", yours[r]), l.x, l.y + 8, 15, Fade(WHITE, .85f), true);
    }
    (void)t;
}

int mapHit(Rectangle A, Vector2 m) {
    Vector2 q{(m.x - A.x) / A.width * 400.f, (m.y - A.y) / A.height * 300.f};
    for (int r = 0; r < NREGION; ++r)
        if (pointIn(POLY[r], q)) return r;
    return -1;
}

}  // namespace seat::gfx
