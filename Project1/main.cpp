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
		auto input_nama_lengkap = std::make_shared<input>("Nama Lengkap    : ", 50);
		auto input_nama_panggilan = std::make_shared<input>("Nama Panggilan  : ", 50);
		auto choice_jk = std::make_shared<choice>("Jenis Kelamin   : ", std::initializer_list<std::string>{ "Laki-Laki", "Perempuan", "Non-Binary" });
		auto input_nisn = std::make_shared<input>("NISN            : ", 50);
		auto input_nik = std::make_shared<input>("NIK             : ", 50);
		auto input_tempat_lahir = std::make_shared<input>("Tempat Lahir    : ", 50);
		auto dropdown_agama = std::make_shared<dropdown>("Agama           : ", std::initializer_list<std::string>{ "Islam", "Kristen", "Katolik", "Hindu", "Buddha", "Konghuchu", "Lainnya" });
		auto choice_goldar = std::make_shared<choice>("Golongan Darah  : ", std::initializer_list<std::string>{"A", "B", "AB", "O" });
		auto input_alamat = std::make_shared<input>("Alamat Rumah    : ", 50);
		auto input_notel = std::make_shared<input>("Nomor telepon   : ", 50);
		auto input_email = std::make_shared<input>("Email           : ", 50);
		auto button_daftar = std::make_shared<button>("Daftar");
		auto button_exit = std::make_shared<button>("Exit", [&loop]() { loop = false; });

		window_main.add(std::make_shared<text>("============================"));
		window_main.add(std::make_shared<text>("   FORM PENDAFTARAN SISWA"));
		window_main.add(std::make_shared<text>("============================"));
		window_main.add(std::make_shared<text>("Silakan lengkapi data dibawah ini."));
		window_main.add(std::make_shared<space>());
		window_main.add(input_nama_lengkap);
		window_main.add(input_nama_panggilan);
		window_main.add(choice_jk);
		window_main.add(input_nisn);
		window_main.add(input_nik);
		window_main.add(input_tempat_lahir);
		window_main.add(dropdown_agama);
		window_main.add(choice_goldar);
		window_main.add(input_alamat);
		window_main.add(input_notel);
		window_main.add(input_email);
		window_main.add(button_daftar);
		window_main.add(button_exit);

		while (loop) self.render(window_main);
	}
} app;

int main() {
	app.run();

	return 0;
}