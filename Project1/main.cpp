#include "simple.h"

/* keymap
* 'j': bawah
* 'k': atas
* 'h': kiri
* 'l': kanan
* tab, arrow_down: component selanjutnya
* shift+tab, arrow_up: component sebelumnya
* return: select
*/

const std::initializer_list<std::string> jenis_kelamin{ "Pria", "Wanita", "Non-Biner" };
const std::initializer_list<std::string> agama{ "Islam", "Katolik", "Protestan", "Hindu", "Buddha", "Khonghuchu", "Lainnya" };
const std::initializer_list<std::string> jurusan{
	"Teknik Mesin",
	"Teknik Otomotif",
	"Teknik Listrik",
	"Teknik Elektronika",
	"Teknik Komputer dan Jaringan",
	"Tata Boga",
	"Tata Busana",
	"Tata Rias",
	"Multimedia",
	"Desain Grafis",
	"Akuntansi",
	"Perhotelan dan Pariwisata",
	"Kesehatan",
	"Pemasaran",
	"Teknik Bangunan",
	"Pariwisata",
	"Perikanan",
	"Peternakan",
	"Agribisnis",
	"Kimia Industri",
	"Teknologi Pangan",
	"Tata Air",
	"Konstruksi Kapal",
	"Keamanan Jaringan",
	"Logistik",
	"Teknik Audio dan Video",
	"Teknik Pesawat Udara",
	"Kebidanan",
	"Akomodasi Perhotelan",
	"Farmasi"
};

class penerimaan_peserta_didik_baru final : public console {
public:
	penerimaan_peserta_didik_baru() : console({ 120, 30 }) {
		self.disable_maximize_button();
		self.disable_minimize_button();
		self.disable_resize_window();
		self.set_cursor_visible(false);
	}
	auto main() -> void override {
		auto loop = true;
		auto window_dashboard = window(self.width, self.height);
		auto input_nama = std::make_shared<input>("Nama Siswa            :");
		auto input_ttl = std::make_shared<input>("Tempat, Tanggal lahir :");
		auto choice_jk = std::make_shared<choice>("Jenis kelamin         :", jenis_kelamin);
		auto input_alamat = std::make_shared<input>("Alamat                :");
		auto choice_agama = std::make_shared<choice>("Agama                 :", agama);
		auto input_notel = std::make_shared<input>("Nomor Telepon/HP      :");
		auto dropdown_jurusan = std::make_shared<dropdown>("Jurusan               :", jurusan, 30);
		auto function_daftar = [&]() {
			if (input_nama->empty() || input_ttl->empty() || input_alamat->empty() || input_notel->empty()) {
				auto loop = true;
				auto window_kosong = window(self.width, self.height);
				auto button_ok = std::make_shared<button>("Ok", [&loop]() { loop = false; });

				window_kosong.add(std::make_shared<text>("================================================="));
				window_kosong.add(std::make_shared<text>("  FORM PENERIMAAN PESERTA DIDIK BARU TAHUN 2023"));
				window_kosong.add(std::make_shared<text>("================================================="));
				window_kosong.add(std::make_shared<text>("Silakan lengkapi seluruh data."));
				window_kosong.add(button_ok);

				while (loop)
					self.render(window_kosong);
			}
			else {
				auto loop = true;
				auto window_konfirmasi = window(self.width, self.height);
				auto function_ok = [&]() {
					auto loop = true;
					auto window_berhasil = window(self.width, self.height);
					auto button_ok = std::make_shared<button>("Ok", [&loop]() { loop = false; });

					window_berhasil.add(std::make_shared<text>("================================================="));
					window_berhasil.add(std::make_shared<text>("  FORM PENERIMAAN PESERTA DIDIK BARU TAHUN 2023"));
					window_berhasil.add(std::make_shared<text>("================================================="));
					window_berhasil.add(std::make_shared<text>("Data siswa berhasil ditambahkan."));
					window_berhasil.add(button_ok);

					while (loop)
						self.render(window_berhasil);
					};
				auto button_ok = std::make_shared<button>("Ok", [&function_ok, &loop]() { function_ok(); loop = false; });
				auto button_batal = std::make_shared<button>("Batal", [&loop]() { loop = false; });

				window_konfirmasi.add(std::make_shared<text>("================================================="));
				window_konfirmasi.add(std::make_shared<text>("  FORM PENERIMAAN PESERTA DIDIK BARU TAHUN 2023"));
				window_konfirmasi.add(std::make_shared<text>("================================================="));
				window_konfirmasi.add(std::make_shared<text>("Konfirmasi data siswa."));
				window_konfirmasi.add(std::make_shared<space>());
				window_konfirmasi.add(std::make_shared<text>("Nama Siswa            :" + input_nama->value()));
				window_konfirmasi.add(std::make_shared<text>("Tempat, Tanggal lahir :" + input_ttl->value()));
				window_konfirmasi.add(std::make_shared<text>("Jenis kelamin         :" + choice_jk->value()));
				window_konfirmasi.add(std::make_shared<text>("Alamat                :" + input_alamat->value()));
				window_konfirmasi.add(std::make_shared<text>("Agama                 :" + choice_agama->value()));
				window_konfirmasi.add(std::make_shared<text>("Nomor Telepon/HP      :" + input_notel->value()));
				window_konfirmasi.add(std::make_shared<text>("Jurusan               :" + dropdown_jurusan->value()));
				window_konfirmasi.add(button_ok);
				window_konfirmasi.add(button_batal);

				while (loop)
					self.render(window_konfirmasi);
			}
			};
		auto button_daftar = std::make_shared<button>("Daftar", function_daftar);
		auto button_exit = std::make_shared<button>("Exit", [&loop]() { loop = false; });

		window_dashboard.add(std::make_shared<text>("================================================="));
		window_dashboard.add(std::make_shared<text>("  FORM PENERIMAAN PESERTA DIDIK BARU TAHUN 2023"));
		window_dashboard.add(std::make_shared<text>("================================================="));
		window_dashboard.add(std::make_shared<space>());
		window_dashboard.add(input_nama);
		window_dashboard.add(input_ttl);
		window_dashboard.add(choice_jk);
		window_dashboard.add(input_alamat);
		window_dashboard.add(choice_agama);
		window_dashboard.add(input_notel);
		window_dashboard.add(dropdown_jurusan);
		window_dashboard.add(button_daftar);
		window_dashboard.add(button_exit);

		while (loop)
			self.render(window_dashboard);
	}
} ppdb;

int main() { ppdb.run(); return 0; }