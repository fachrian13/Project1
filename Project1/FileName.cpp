#include <windows.h>
#include <iostream>

int main() {
	COORD size = GetLargestConsoleWindowSize(GetStdHandle(STD_OUTPUT_HANDLE));

	std::cout << size.X << " " << size.Y;
	std::cin.get();
}