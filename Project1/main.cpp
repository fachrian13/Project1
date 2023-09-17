#include "simple.h"

int main() {
	auto c_form = console(120, 30);
	auto i_nama_lengkap = std::make_shared<input>("Nama Lengkap   : ", 50);
	auto i_gelar = std::make_shared<input>("Gelar Akademik : ", 50);
	auto m_pendidikan = std::make_shared<dropdown>(
		"Pendidikan     : ",
		std::vector<std::string> {
		"SD",
			"SMP",
			"SMA",
			"D1",
			"D2",
			"D3",
			"S1",
			"S2",
			"S3"
	},
		50,
		5
	);
	auto i_email = std::make_shared<input>("Email          : ", 50);
	auto i_hp = std::make_shared<input>("Nomor HP/WA    : ", 50);
	auto m_desc = std::make_shared<dropdown>(
		"Description    : ",
		std::vector<std::string> {
		"Adaptive bi - directional hierarchy",
			"Adaptive intangible frame",
			"Advanced static process improvement",
			"Ameliorated directional emulation",
			"Assimilated 24 / 7 archive",
			"Automated 4thgeneration website",
			"Balanced analyzing groupware",
			"Balanced multimedia knowledgebase",
			"Centralized attitude - oriented capability",
			"Centralized leadingedge moratorium",
			"Centralized non - volatile capability",
			"Centralized secondary time - frame",
			"Compatible analyzing intranet",
			"Configurable zero administration Graphical User Interface",
			"Cross - group user - facing focus group",
			"Customer - focused explicit frame",
			"De - engineered fault - tolerant challenge",
			"De - engineered systemic artificial intelligence",
			"De - engineered transitional strategy",
			"Distributed impactful customer loyalty",
			"Diverse exuding installation",
			"Enhanced foreground collaboration",
			"Enhanced intangible time - frame",
			"Enterprise - wide executive installation",
			"Extended content - based methodology",
			"Extended human - resource intranet",
			"Face - to - face high - level conglomeration",
			"Face - to - face well - modulated customer loyalty",
			"Front - line clear - thinking encryption",
			"Front - line systematic help - desk",
			"Function - based fault - tolerant concept",
			"Fundamental asynchronous capability",
			"Fundamental stable info - mediaries",
			"Future - proofed radical implementation",
			"Grass - roots methodical info - mediaries",
			"Grass - roots radical parallelism",
			"Horizontal empowering knowledgebase",
			"Innovative background definition",
			"Intuitive local adapter",
			"Managed demand - driven website",
			"Managed human - resource policy",
			"Mandatory coherent synergy",
			"Monitored client - server implementation",
			"Multi - channeled 3rdgeneration open system",
			"Multi - lateral scalable protocol",
			"Multi - layered composite paradigm",
			"Multi - tiered secondary productivity",
			"Object - based optimizing model",
			"Object - based value - added database",
			"Open - architected well - modulated capacity",
			"Open - source zero administration hierarchy",
			"Optional exuding superstructure",
			"Optional non - volatile open system",
			"Organic logistical leverage",
			"Organic non - volatile hierarchy",
			"Organized empowering forecast",
			"Persevering contextually - based approach",
			"Persevering exuding budgetary management",
			"Persistent interactive circuit",
			"Persistent real - time customer loyalty",
			"Persistent tertiary focus group",
			"Persistent tertiary website",
			"Phased next generation adapter",
			"Proactive foreground paradigm",
			"Profit - focused coherent installation",
			"Profit - focused dedicated frame",
			"Profound client - server frame",
			"Progressive modular hub",
			"Quality - focused client - server Graphical User Interface",
			"Re - contextualized dynamic hierarchy",
			"Reactive attitude - oriented toolset",
			"Realigned didactic function",
			"Reverse - engineered composite moratorium",
			"Reverse - engineered heuristic alliance",
			"Reverse - engineered mission - critical moratorium",
			"Right - sized clear - thinking flexibility",
			"Right - sized zero tolerance focus group",
			"Seamless disintermediate collaboration",
			"Secured foreground emulation",
			"Secured logistical synergy",
			"Secured zero tolerance hub",
			"Self - enabling fresh - thinking installation",
			"Self - enabling multi - tasking process improvement",
			"Sharable optimal functionalities",
			"Stand - alone static implementation",
			"Streamlined 6thgeneration function",
			"Switchable scalable moratorium",
			"Synchronized needs - based challenge",
			"Synergistic background access",
			"Synergistic web - enabled framework",
			"Team - oriented tangible complexity",
			"Universal human - resource collaboration",
			"User - centric 4thgeneration system engine",
			"User - centric heuristic focus group",
			"User - centric modular customer loyalty",
			"User - centric system - worthy leverage",
			"User - friendly clear - thinking productivity",
			"User - friendly exuding migration",
			"Virtual holistic methodology",
			"Vision - oriented secondary project"
	},
		50,
		20
	);
	auto i_tempat_lahir = std::make_shared<input>("Tempat Lahir   : ", 50);
	auto i_alamat = std::make_shared<input>("Alamat         : ", 50);
	auto f_proses = std::function<void()>();
	auto b_proses = std::make_shared<button>("Proses", [&]() {
		auto c_print = console(120, 30);
		auto b_ya = std::make_shared<button>("Ya", [&]() {
			c_form.stop();
			c_print.stop();
			});
		auto b_no = std::make_shared<button>("Tidak", [&]() { c_print.stop(); });

		c_print.add(std::make_shared<text>("=========================="));
		c_print.add(std::make_shared<text>("   KONFIRMASI DATA DIRI"));
		c_print.add(std::make_shared<text>("=========================="));
		c_print.add(std::make_shared<separator>());
		c_print.add(std::make_shared<text>("Nama Lengkap   : " + i_nama_lengkap->get_value()));
		c_print.add(std::make_shared<text>("Gelar Akademik : " + i_gelar->get_value()));
		c_print.add(std::make_shared<text>("Pendidikan     : " + m_pendidikan->get_value()));
		c_print.add(std::make_shared<text>("Email          : " + i_email->get_value()));
		c_print.add(std::make_shared<text>("Nomor HP/WA    : " + i_hp->get_value()));
		c_print.add(std::make_shared<text>("Description    : " + m_desc->get_value()));
		c_print.add(std::make_shared<text>("Tempat Lahir   : " + i_tempat_lahir->get_value()));
		c_print.add(std::make_shared<text>("Alamat         : " + i_alamat->get_value()));
		c_print.add(b_ya);
		c_print.add(b_no);
		c_print.run();
		});
	auto b_exit = std::make_shared<button>("Exit", [&]() { c_form.stop(); });


	c_form.add(std::make_shared<text>("==========================="));
	c_form.add(std::make_shared<text>("   SILAKAN ISI DATA DIRI"));
	c_form.add(std::make_shared<text>("==========================="));
	c_form.add(std::make_shared<separator>());
	c_form.add(i_nama_lengkap);
	c_form.add(i_gelar);
	c_form.add(m_pendidikan);
	c_form.add(i_email);
	c_form.add(i_hp);
	c_form.add(m_desc);
	c_form.add(i_tempat_lahir);
	c_form.add(i_alamat);
	c_form.add(b_proses);
	c_form.add(b_exit);

	std::cout << "\x1b[?25l";
	c_form.run();
}