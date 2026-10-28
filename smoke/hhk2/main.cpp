#include <appdef.hpp>
#include <sdk/calc/calc.hpp>
#include <sdk/os/debug.hpp>
#include <sdk/os/lcd.hpp>
APP_NAME("Dual test: HHK2")
APP_DESCRIPTION("Legacy HHK2 SDK app. Displays a message and returns after a key press.")
APP_AUTHOR("Dual Launcher test")
APP_VERSION("0.1")
extern "C" void main() {
  calcInit();
  LCD_ClearScreen();
  Debug_Printf(2, 3, false, 0, "HHK2 .hhk started");
  Debug_Printf(2, 5, false, 0, "Press any key to return");
  LCD_Refresh();
  Debug_WaitKey();
  calcEnd();
}
