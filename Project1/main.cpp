#include "simple.h"

/* keymap
* 'j': bawah
* 'k': atas
* 'h', arrow_left: kiri
* 'l', arrow_right: kanan
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
const std::initializer_list<std::string> tanggal{
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
	"11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
	"21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31"
};
const std::initializer_list<std::string> bulan{
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"
};
const std::initializer_list<std::string> tahun{
	"1970",	"1971",	"1972",	"1973",	"1974",	"1975",	"1976",	"1977",	"1978",	"1979",
	"1980",	"1981",	"1982",	"1983",	"1984",	"1985",	"1986",	"1987",	"1988",	"1989",
	"1990",	"1991",	"1992",	"1993",	"1994",	"1995",	"1996",	"1997",	"1998",	"1999",
	"2000",	"2001",	"2002",	"2003",	"2004",	"2005",	"2006",	"2007",	"2008",	"2009",
	"2010",	"2011",	"2012",	"2013",	"2014",	"2015",	"2016",	"2017",	"2018",	"2019",
	"2020",	"2021",	"2022",	"2023",	"2024",	"2025",	"2026",	"2027",	"2028",	"2029",
	"2030",	"2031",	"2032",	"2033",	"2034",	"2035",	"2036",	"2037",	"2038",	"2039",
	"2040",	"2041",	"2042",	"2043",	"2044",	"2045",	"2046",	"2047",	"2048",	"2049",
	"2050",	"2051",	"2052",	"2053",	"2054",	"2055",	"2056",	"2057",	"2058",	"2059",
	"2060",	"2061",	"2062",	"2063",	"2064",	"2065",	"2066",	"2067",	"2068",	"2069",
	"2070",	"2071",	"2072",	"2073",	"2074",	"2075",	"2076",	"2077",	"2078",	"2079"
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
		auto input_nama = std::make_shared<input>("Nama Siswa            : ");
		auto input_tempat_lahir = std::make_shared<input>("Tempat lahir          : ");
		auto hlayout_tanggal_lahir = std::make_shared<hlayout>();
		auto dropdown_tanggal = std::make_shared<dropdown>("Tanggal : ", tanggal, 10, 5);
		auto dropdown_bulan = std::make_shared<dropdown>("Bulan : ", bulan, 10, 0);
		auto dropdown_tahun = std::make_shared<dropdown>("Tahun : ", tahun, 14, 5);
		auto choice_jk = std::make_shared<choice>("Jenis kelamin         : ", jenis_kelamin);
		auto input_alamat = std::make_shared<input>("Alamat                : ");
		auto choice_agama = std::make_shared<choice>("Agama                 : ", agama);
		auto input_notel = std::make_shared<input>("Nomor Telepon/HP      : ");
		auto dropdown_jurusan = std::make_shared<dropdown>("Jurusan               : ", jurusan, 10, 30);
		auto function_daftar = [&]() {
			if (input_nama->empty() || input_tempat_lahir->empty() || input_alamat->empty() || input_notel->empty()) {
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
				window_konfirmasi.add(std::make_shared<text>("Tempat, Tanggal lahir :" + input_tempat_lahir->value()));
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
		window_dashboard.add(input_tempat_lahir);
		hlayout_tanggal_lahir->add(std::make_shared<text>("Tanggal lahir         :"));
		hlayout_tanggal_lahir->add(dropdown_tanggal);
		hlayout_tanggal_lahir->add(dropdown_bulan);
		hlayout_tanggal_lahir->add(dropdown_tahun);
		window_dashboard.add(hlayout_tanggal_lahir);
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