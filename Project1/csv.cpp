#include "simple.h"

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
		auto menu_main = std::make_shared<menu>(std::initializer_list<std::string>{
				"1. Tambah data siswa",
				"2. Lihat seluruh data",
				"3. Cari data siswa",
				"4. Update data siswa",
				"5. Hapus data siswa"
			}
		);
		auto button_exit = std::make_shared<button>("Exit", [&window_main]() { window_main.loop(false); });

		window_main.add(std::make_shared<text>("========================="));
		window_main.add(std::make_shared<text>("  MANAGEMENT DATA SISWA"));
		window_main.add(std::make_shared<text>("========================="));
		window_main.add(std::make_shared<space>());
		window_main.add(menu_main);
		window_main.add(button_exit);

		while (window_main.loop()) {
			self.render(window_main);

			switch (menu_main->index()) {
			case 0:
				break;
			}
		}
	}

	auto tambah_data_siswa() -> void {
		auto window_tambah_data = window(self.width, self.height);
		auto input_nama_siswa = std::make_shared<input>("Nama siswa : ");
		auto input_tempat_lahir = std::make_shared<input>("Tempat lahir : ");
		auto dropdown_tanggal = std::make_shared<dropdown>("Tanggal ", tanggal, 10, 5);
		auto dropdown_bulan = std::make_shared<dropdown>("Bulan ", bulan, 10);
		auto dropdown_tahun = std::make_shared<dropdown>("Tahun ", tahun, 10, 5);
		auto hlayout_ttl = std::make_shared<hlayout>();
		hlayout_ttl->add(dropdown_tanggal);
		hlayout_ttl->add(dropdown_bulan);
		hlayout_ttl->add(dropdown_tahun);
		auto input_alamat = std::make_shared<input>("Alamat : ");
	}
} csv;

int main() { csv.run(); return 0; }