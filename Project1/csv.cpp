#include "simple.h"

class csv final : public console {
public:
	csv() : console() {
		self.disable_maximize_button();
		self.disable_minimize_button();
		self.disable_resize_window();
		self.set_cursor_visible(false);
	}

	auto main() -> void override {
		auto window_main = window(self.width, self.height);
		auto loop = true;
		auto menu_main = std::make_shared<menu>(std::initializer_list<std::string>{
				"1. Tambah data siswa",
				"2. Lihat seluruh data",
				"3. Cari data siswa",
				"4. Update data siswa",
				"5. Hapus data siswa"
			}
		);
		auto button_exit = std::make_shared<button>("Exit", [&loop]() { loop = false; });

		window_main.add(std::make_shared<text>("========================="));
		window_main.add(std::make_shared<text>("  MANAGEMENT DATA SISWA"));
		window_main.add(std::make_shared<text>("========================="));
		window_main.add(std::make_shared<space>());
		window_main.add(menu_main);
		window_main.add(button_exit);

		while (loop)
			self.render(window_main);
	}
} csv;

int main() { csv.run(); return 0; }