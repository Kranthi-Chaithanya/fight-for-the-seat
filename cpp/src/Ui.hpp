// Immediate-mode UI helpers on top of raylib: fonts, wrapped text, buttons, panels, bubbles.
#pragma once
#include "Common.hpp"

namespace seat::ui {

void load();
void unload();
Font& font(bool bold, float size);

Vector2 measure(const std::string& s, float size, bool bold = false);
void text(const std::string& s, float x, float y, float size, Color c, bool bold = false);
void textCentered(const std::string& s, float cx, float y, float size, Color c, bool bold = false);
void textRight(const std::string& s, float rx, float y, float size, Color c, bool bold = false);
std::vector<std::string> wrap(const std::string& s, float w, float size, bool bold = false);
float textWrapped(const std::string& s, Rectangle box, float size, Color c, bool bold = false, float gap = 1.3f);
float wrappedHeight(const std::string& s, float w, float size, bool bold = false, float gap = 1.3f);

extern bool blocked;  // true while a dialog covers the scene
extern bool pendingClick;  // web: a press that was released within the same frame
bool mousePressed();
extern bool webMouse;         // web: position comes from browser events, in 1280x720 canvas units
extern Vector2 webMousePos;
Vector2 mouse();
bool hovered(Rectangle r);
bool clickedIn(Rectangle r);
bool button(Rectangle r, const std::string& label, bool enabled = true, bool primary = false, const std::string& sub = "", int hotkey = 0);
void panel(Rectangle r, Color c = Pal::panel, float round = .06f);
void bar(Rectangle r, float frac, Color col, Color bg = Pal::panel2);
void chip(float x, float y, const std::string& s, Color bg, Color fg, float size = 15);
float chipWidth(const std::string& s, float size = 15);
void bubble(const std::string& s, Vector2 anchor, float alpha, Color bg, Color fg, float size = 16, float maxW = 240);

}  // namespace seat::ui
