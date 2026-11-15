#include "Ui.hpp"
#include "Audio.hpp"

namespace seat::ui {

bool blocked = false;
bool pendingClick = false;
bool webMouse = false;
Vector2 webMousePos{};
Vector2 mouse() { return webMouse ? webMousePos : GetMousePosition(); }
bool mousePressed() { return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || pendingClick; }

static Font fReg20, fReg30, fBold22, fBold34, fBold72;
static bool loaded = false;

static Font loadOne(const char* path, int size) {
    std::vector<int> cps;
    for (int c = 32; c < 127; ++c) cps.push_back(c);
    for (int c = 0xA0; c <= 0xFF; ++c) cps.push_back(c);
    for (int c : {0x2013, 0x2014, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2026, 0x20B9, 0x2190, 0x2191, 0x2192, 0x2193, 0x25B2, 0x25BC})
        cps.push_back(c);
    if (!FileExists(path)) return GetFontDefault();
    Font f = LoadFontEx(path, size, cps.data(), (int)cps.size());
    if (f.texture.id == 0) return GetFontDefault();
    GenTextureMipmaps(&f.texture);
    SetTextureFilter(f.texture, TEXTURE_FILTER_TRILINEAR);
    return f;
}

void load() {
    // Bundled open-licence font (Hind, SIL OFL), looked up next to the game; Segoe UI as a native fallback.
    std::string reg = "C:/Windows/Fonts/segoeui.ttf", bold = "C:/Windows/Fonts/segoeuib.ttf";
    std::string app = GetApplicationDirectory();
    for (const std::string& dir : {std::string("assets/fonts/"), std::string("/assets/fonts/"), app + "../assets/fonts/", app + "assets/fonts/"}) {
        if (FileExists((dir + "Hind-Regular.ttf").c_str())) {
            reg = dir + "Hind-Regular.ttf";
            bold = dir + "Hind-Bold.ttf";
            break;
        }
    }
    fReg20 = loadOne(reg.c_str(), 40);
    fReg30 = loadOne(reg.c_str(), 60);
    fBold22 = loadOne(bold.c_str(), 44);
    fBold34 = loadOne(bold.c_str(), 68);
    fBold72 = loadOne(bold.c_str(), 112);
    loaded = true;
}

void unload() {
    if (!loaded) return;
    for (Font* f : {&fReg20, &fReg30, &fBold22, &fBold34, &fBold72})
        if (f->texture.id != GetFontDefault().texture.id) UnloadFont(*f);
}

Font& font(bool bold, float size) {
    if (size <= 22) return bold ? fBold22 : fReg20;
    if (size <= 38) return bold ? fBold34 : fReg30;
    return fBold72;
}

Vector2 measure(const std::string& s, float size, bool bold) { return MeasureTextEx(font(bold, size), s.c_str(), size, 0); }

void text(const std::string& s, float x, float y, float size, Color c, bool bold) {
    DrawTextEx(font(bold, size), s.c_str(), {std::round(x), std::round(y)}, size, 0, c);
}
void textCentered(const std::string& s, float cx, float y, float size, Color c, bool bold) {
    text(s, cx - measure(s, size, bold).x / 2, y, size, c, bold);
}
void textRight(const std::string& s, float rx, float y, float size, Color c, bool bold) {
    text(s, rx - measure(s, size, bold).x, y, size, c, bold);
}

std::vector<std::string> wrap(const std::string& s, float w, float size, bool bold) {
    std::vector<std::string> lines;
    size_t start = 0;
    while (start <= s.size()) {
        size_t nl = s.find('\n', start);
        std::string para = s.substr(start, nl == std::string::npos ? std::string::npos : nl - start);
        std::string cur;
        size_t i = 0;
        while (i <= para.size()) {
            size_t sp = para.find(' ', i);
            std::string word = para.substr(i, sp == std::string::npos ? std::string::npos : sp - i);
            std::string trial = cur.empty() ? word : cur + " " + word;
            if (!cur.empty() && measure(trial, size, bold).x > w) {
                lines.push_back(cur);
                cur = word;
            } else
                cur = trial;
            if (sp == std::string::npos) break;
            i = sp + 1;
        }
        lines.push_back(cur);
        if (nl == std::string::npos) break;
        start = nl + 1;
    }
    return lines;
}

float textWrapped(const std::string& s, Rectangle box, float size, Color c, bool bold, float gap) {
    auto lines = wrap(s, box.width, size, bold);
    float y = box.y;
    for (auto& l : lines) {
        text(l, box.x, y, size, c, bold);
        y += size * gap;
    }
    return y - box.y;
}

float wrappedHeight(const std::string& s, float w, float size, bool bold, float gap) {
    return wrap(s, w, size, bold).size() * size * gap;
}

bool hovered(Rectangle r) { return !blocked && CheckCollisionPointRec(mouse(), r); }
bool clickedIn(Rectangle r) { return hovered(r) && mousePressed(); }

bool button(Rectangle r, const std::string& label, bool enabled, bool primary, const std::string& sub, int hotkey) {
    bool hov = enabled && hovered(r);
    Color bg = primary ? Pal::accentDeep : Pal::panel2;
    if (!enabled) bg = Fade(Pal::panel2, .5f);
    else if (hov) bg = primary ? ColorBrightness(Pal::accentDeep, .15f) : ColorBrightness(Pal::panel2, .25f);
    DrawRectangleRounded(r, .25f, 8, bg);
    DrawRectangleRoundedLinesEx(r, .25f, 8, 1.5f, hov ? Pal::accent : Fade(Pal::line, enabled ? 1.f : .5f));
    Color fg = enabled ? Pal::text : Fade(Pal::muted, .7f);
    float ls = 17;
    if (sub.empty()) {
        Vector2 m = measure(label, ls, true);
        text(label, r.x + (r.width - m.x) / 2, r.y + (r.height - m.y) / 2, ls, fg, true);
    } else {
        text(label, r.x + 12, r.y + 7, ls, fg, true);
        text(sub, r.x + 12, r.y + 29, 14, enabled ? Pal::muted : Fade(Pal::muted, .6f));
    }
    bool fire = enabled && !blocked && ((hov && mousePressed()) || (hotkey && IsKeyPressed(hotkey)));
    if (fire) sfx::play(sfx::CLICK, .5f);
    return fire;
}

void panel(Rectangle r, Color c, float round) {
    DrawRectangleRounded(r, round, 8, c);
    DrawRectangleRoundedLinesEx(r, round, 8, 1.f, Fade(Pal::line, .8f));
}

void bar(Rectangle r, float frac, Color col, Color bg) {
    DrawRectangleRounded(r, 1.f, 8, bg);
    frac = clampf(frac, 0, 1);
    if (frac > 0.005f) DrawRectangleRounded({r.x, r.y, std::max(r.height, r.width * frac), r.height}, 1.f, 8, col);
}

float chipWidth(const std::string& s, float size) { return measure(s, size, true).x + 16; }
void chip(float x, float y, const std::string& s, Color bg, Color fg, float size) {
    float w = chipWidth(s, size);
    DrawRectangleRounded({x, y, w, size + 8}, 1.f, 8, bg);
    text(s, x + 8, y + 3, size, fg, true);
}

void bubble(const std::string& s, Vector2 a, float alpha, Color bg, Color fg, float size, float maxW) {
    if (alpha <= 0.01f || s.empty()) return;
    auto lines = wrap(s, maxW, size, true);
    float w = 0;
    for (auto& l : lines) w = std::max(w, measure(l, size, true).x);
    float h = lines.size() * size * 1.2f;
    Rectangle r{a.x - w / 2 - 10, a.y - h - 22, w + 20, h + 12};
    if (r.x < 4) r.x = 4;
    if (r.x + r.width > GetScreenWidth() - 4 && r.width < GetScreenWidth()) r.x = GetScreenWidth() - 4 - r.width;
    DrawRectangleRounded(r, .35f, 8, Fade(bg, alpha));
    Vector2 t1{a.x - 7, r.y + r.height - 1}, t2{a.x + 7, r.y + r.height - 1}, t3{a.x, r.y + r.height + 9};
    DrawTriangle(t1, t3, t2, Fade(bg, alpha));
    float y = r.y + 6;
    for (auto& l : lines) {
        text(l, r.x + 10, y, size, Fade(fg, alpha), true);
        y += size * 1.2f;
    }
}

}  // namespace seat::ui
