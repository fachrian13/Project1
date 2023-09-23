#include <Windows.h>
#include <stdio.h>

int main() {
  HANDLE hConsoleInput;
  INPUT_RECORD inputRecords[128]; // Buffer to hold input records
  DWORD numEventsRead;
  DWORD i;

  hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);

  if (hConsoleInput == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Error: GetStdHandle\n");
    return 1;
  }

  while (1) {
    if (ReadConsoleInput(hConsoleInput, inputRecords, 128, &numEventsRead)) {
      for (i = 0; i < numEventsRead; i++) {
        if (inputRecords[i].EventType == KEY_EVENT) {
          // Handle key events
          if (inputRecords[i].Event.KeyEvent.bKeyDown) {
            printf("Key pressed: %c\n", inputRecords[i].Event.KeyEvent.wVirtualKeyCode);
          }
        }
        else if (inputRecords[i].EventType == MOUSE_EVENT) {
          // Handle mouse events
          MOUSE_EVENT_RECORD mouseEvent = inputRecords[i].Event.MouseEvent;
          if (mouseEvent.dwEventFlags == MOUSE_MOVED) {
            printf("Mouse moved to (%d, %d)\n", mouseEvent.dwMousePosition.X, mouseEvent.dwMousePosition.Y);
          }
          else if (mouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
            printf("Left mouse button pressed at (%d, %d)\n", mouseEvent.dwMousePosition.X, mouseEvent.dwMousePosition.Y);
          }
        }
      }
    }
  }

  return 0;
}
