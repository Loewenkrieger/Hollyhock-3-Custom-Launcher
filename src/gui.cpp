// SPDX-License-Identifier: GPL-3.0-or-later
#include "gui.hpp"
#include "classic/ui.hpp"
#include <optional>
#include <sdk/os/input.h>
#include <sdk/os/lcd.h>
#include <string>
#include <vector>

namespace {
std::string text(std::unique_ptr<char[]> value) { return value ? value.get() : ""; }
std::optional<classic::Key> translate(Input_Keycode code) {
  switch (code) {
  case KEYCODE_UP: return classic::Key::Up;
  case KEYCODE_DOWN: return classic::Key::Down;
  case KEYCODE_LEFT: return classic::Key::Left;
  case KEYCODE_RIGHT: return classic::Key::Right;
  case KEYCODE_EXE: return classic::Key::Enter;
  case KEYCODE_POWER_CLEAR:
  case KEYCODE_BACKSPACE: return classic::Key::Escape;
  default: return std::nullopt;
  }
}
}

std::unique_ptr<Executable>
do_gui(std::forward_list<std::unique_ptr<Executable>> &executables) {
  std::vector<std::unique_ptr<Executable> *> entries;
  std::vector<classic::AppInfo> apps;
  for (auto &exe : executables) {
    classic::AppInfo app;
    app.path = text(exe->getPath());
    app.format = app.path.ends_with(".hhk") ? "HHK2" : "HH3";
    try {
      app.name = text(exe->getName());
      app.version = text(exe->getVersion());
      app.author = text(exe->getAuthor());
      app.description = text(exe->getDescription());
    } catch (const std::exception &error) {
      app.description = error.what();
    }
    if (app.name.empty()) app.name = app.path;
    entries.push_back(&exe);
    apps.push_back(std::move(app));
  }

  unsigned int screen_width = 0, screen_height = 0;
  LCD_GetSize(&screen_width, &screen_height);
  const classic::Canvas screen{LCD_GetVRAMAddress(), static_cast<int>(screen_width), static_cast<int>(screen_height)};
  classic::State state(apps.size());
  bool dirty = true;
  while (state.action == classic::Action::None) {
    if (dirty) { classic::render(screen, state, apps); LCD_Refresh(); dirty = false; }
    // Service one blocking OS event queue immediately. No timer or competing
    // input consumer, and DOWN/UP transitions are never collapsed together.
    Input_Event event{};
    GetInput(&event, 0xffffffffu, 0x10);
    switch (event.type) {
    case EVENT_TOUCH: {
      const auto &touch = event.data.touch_single;
      if (touch.direction & TOUCH_UP) dirty = state.touch_up();
      else if (touch.direction & TOUCH_DOWN) dirty = state.touch_down(touch.p1_x, touch.p1_y);
      else if (touch.direction & TOUCH_HOLD_DRAG) dirty = state.touch_move(touch.p1_x, touch.p1_y);
      break;
    }
    case EVENT_KEY:
      if (event.data.key.direction != KEY_RELEASED)
        if (auto key = translate(event.data.key.keyCode))
          dirty = state.key(*key, event.data.key.direction == KEY_HELD);
      break;
    case EVENT_ACTBAR_ESC: dirty = state.key(classic::Key::Escape); break;
    default: break;
    }
  }
  if (state.action == classic::Action::Run && state.selected >= 0 &&
      static_cast<std::size_t>(state.selected) < entries.size())
    return std::move(*entries[state.selected]);
  return nullptr;
}
