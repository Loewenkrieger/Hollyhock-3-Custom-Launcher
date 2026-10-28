// SPDX-License-Identifier: GPL-3.0-or-later
// Compile the real gui.cpp against the original SDK declarations. Only the
// three display functions and GetInput are replaced with host fakes.
#include "gui.hpp"
#include <sdk/os/input.h>
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
std::vector<Input_Event> events;
std::size_t position;
int refreshes;
std::uint16_t pixels[320 * 528];
void size(unsigned *w, unsigned *h) { *w = 320; *h = 528; }
std::uint16_t *framebuffer() { return pixels; }
void refresh() { ++refreshes; }
int input(Input_Event *out, std::uint32_t wait, std::uint32_t mode) {
  assert(wait == 0xffffffffu && mode == 0x10);
  assert(out->zero == 0 && static_cast<unsigned>(out->type) == 0);
  if (position == events.size()) throw std::runtime_error("UI consumed all events without returning");
  *out = events[position++];
  return 0;
}
Input_Event touch(Input_TouchEventType type, int x, int y) {
  Input_Event e{}; e.type = EVENT_TOUCH; e.data.touch_single.direction = type;
  e.data.touch_single.p1_x = x; e.data.touch_single.p1_y = y; return e;
}
Input_Event key(Input_Keycode code, Input_KeyEventType direction = KEY_PRESSED) {
  Input_Event e{}; e.type = EVENT_KEY; e.data.key.keyCode = code; e.data.key.direction = direction; return e;
}
std::unique_ptr<char[]> copy(const char *s) {
  auto out = std::make_unique<char[]>(std::strlen(s) + 1); std::strcpy(out.get(), s); return out;
}
struct FakeApp final : Executable {
  int id;
  explicit FakeApp(int number) : id(number) {}
  void load() override { assert(false); }
  void unload() override { assert(false); }
  int execute() override { assert(false); return 0; }
  std::unique_ptr<char[]> getPath() override { return copy(id == 0 ? "\\fls0\\one.hh3" : "\\fls0\\two.hhk"); }
  std::unique_ptr<char[]> getName() override { return copy(id == 0 ? "One" : "Two"); }
};
std::forward_list<std::unique_ptr<Executable>> apps() {
  std::forward_list<std::unique_ptr<Executable>> out;
  auto tail = out.emplace_after(out.before_begin(), std::make_unique<FakeApp>(0));
  out.emplace_after(tail, std::make_unique<FakeApp>(1)); return out;
}
void reset(std::vector<Input_Event> queue) { events = std::move(queue); position = 0; refreshes = 0; }
}

extern "C" {
void (*LCD_GetSize)(unsigned *, unsigned *) = size;
std::uint16_t *(*_FP_LCD_GetVRAMAddress)() = framebuffer;
void (*LCD_Refresh)() = refresh;
int (*GetInput)(Input_Event *, std::uint32_t, std::uint32_t) = input;
}

int main() {
  { auto list = apps(); reset({touch(TOUCH_DOWN, 45, 55), touch(TOUCH_UP, 0, 0),
      touch(TOUCH_DOWN, 50, 74 + 28 + 10), touch(TOUCH_UP, -500, -500),
      touch(TOUCH_DOWN, 50, 95), touch(TOUCH_UP, 0, 0)});
    auto chosen = do_gui(list); assert(chosen && static_cast<FakeApp *>(chosen.get())->id == 1 && position == 6);
    std::cout << "PASS real adapter: short dropdown/Run taps with invalid lift coordinates choose HHK2 entry\n"; }
  { auto list = apps(); reset({key(KEYCODE_EXE, KEY_RELEASED), key(KEYCODE_DOWN), key(KEYCODE_EXE)});
    auto chosen = do_gui(list); assert(chosen && static_cast<FakeApp *>(chosen.get())->id == 1 && position == 3);
    std::cout << "PASS real adapter: released EXE ignored; Down + EXE selects intended app\n"; }
  { auto list = apps(); Input_Event idle{}; idle.type = EVENT_TIMER;
    reset({idle, idle, touch(TOUCH_DOWN, 50, 95), touch(TOUCH_UP, 0, 0)});
    auto chosen = do_gui(list); assert(chosen && static_cast<FakeApp *>(chosen.get())->id == 0 && refreshes == 2);
    std::cout << "PASS real adapter: idle events do not redraw or swallow the following tap\n"; }
  { auto list = apps(); reset({touch(TOUCH_DOWN, 50, 95), touch(TOUCH_HOLD_DRAG, 180, 200),
      touch(TOUCH_UP, 50, 95), key(KEYCODE_POWER_CLEAR)});
    assert(!do_gui(list)); assert(position == 4);
    std::cout << "PASS real adapter: drag cancellation followed by physical Clear exits\n"; }
  std::cout << "4/4 adapter checks passed\n";
}
