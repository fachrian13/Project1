#include <iostream>
#include <windows.h>

int main() {
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut == INVALID_HANDLE_VALUE)
  {
    return GetLastError();
  }

  DWORD dwMode = 0;
  if (!GetConsoleMode(hOut, &dwMode))
  {
    return GetLastError();
  }

  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  if (!SetConsoleMode(hOut, dwMode))
  {
    return GetLastError();
  }

	std::cout << "\x1b[9m" << "Hello world";
	std::cin.get();
}