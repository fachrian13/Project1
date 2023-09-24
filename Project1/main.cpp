#include "simple.h"

class application final : public console {
public:
	application() : console(150, 37) {
		self.disable_maximize_button();
		self.disable_minimize_button();
		self.disable_resize_window();
		self.cursor_visible(false);
	}
	void main() override {
		bool loop = true;

		auto window_main = window(self.width, self.height);
		auto input_nama_lengkap = std::make_shared<input>("Nama Lengkap : ", 30);
		auto input_alamat = std::make_shared<input>("Alamat : ", 30);

		while (loop) self.render(window_main);
	}
} app;

int main() {
	app.run();

	return 0;
}