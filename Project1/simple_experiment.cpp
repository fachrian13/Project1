#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>

#define self (*this)

enum class color : unsigned short {
	black = 30,
	red = 31,
	green = 32,
	yellow = 33,
	blue = 34,
	magenta = 35,
	cyan = 36,
	white = 37,
	original = 39,
	gray = 90,
	brightred = 91,
	bright_green = 92,
	bright_yellow = 93,
	bright_blue = 94,
	bright_magenta = 95,
	bright_cyan = 96,
	bright_white = 97
};
enum class component_type : unsigned short {
	render,
	focus
};
class pixel {
public:
	color background = color::black;
	color foreground = color::white;
	char character = ' ';

	pixel() {}
	pixel(color background, color foreground, char character = ' ') :
		background(background), foreground(foreground), character(character) {}
	inline std::string tostring() const {
		return "\x1b["
			+ std::to_string(static_cast<unsigned>(self.foreground))
			+ ";"
			+ std::to_string(static_cast<unsigned>(self.background) + 10)
			+ "m"
			+ self.character
			+ "\x1b[0m";
	}
	friend bool operator ==(const pixel& current, const pixel& other) {
		return current.background == other.background && current.foreground == other.foreground;
	}
	friend bool operator !=(const pixel& current, const pixel& other) {
		return !(current == other);
	}
};
class component {
public:
	bool focus = false;
	component_type type = component_type::render;

public:
	virtual std::vector<std::vector<pixel>>& render() = 0;
	virtual bool on_event(const KEY_EVENT_RECORD& input_event) { return false; }
};
class text final : public component {
public:
	text(std::string value) :
		value(value), canvas(1, std::vector<pixel>(value.size(), pixel())) {}
	std::vector<std::vector<pixel>>& render() override {
		for (size_t i = 0; i < self.value.size(); i++)
			self.canvas[0][i].character = self.value[i];

		return self.canvas;
	}

private:
	std::string value = "";
	std::vector<std::vector<pixel>> canvas = {};
};
class button final : public component {
public:
	button(std::string name, std::function<void()> logic = []() {}) :
		name(name), logic(logic), focused(1, std::vector<pixel>(name.size() + 2, pixel(color::white, color::black))), release(1, std::vector<pixel>(name.size() + 2, pixel())) {
		self.type = component_type::focus;
	}
	std::vector<std::vector<pixel>>& render() override {
		self.focus ? self.canvas = self.focused : self.canvas = release;

		self.canvas[0].front().character = '[';
		for (size_t i = 0; i < self.name.size(); i++)
			self.canvas[0][i + 1].character = self.name[i];
		self.canvas[0].back().character = ']';

		return self.canvas;
	}
	bool on_event(const KEY_EVENT_RECORD& input_event) override {
		switch (input_event.wVirtualKeyCode) {
		case VK_RETURN:
			self.logic();
			return true;
		}

		return false;
	}

private:
	std::string name = "";
	std::function<void()> logic = []() {};
	std::vector<std::vector<pixel>> canvas = {};
	std::vector<std::vector<pixel>> focused = {};
	std::vector<std::vector<pixel>> release = {};
};
class space final : public component {
public:
	space(short width, short height) :
		width(width), height(height), canvas(height, std::vector<pixel>(width, pixel())) {}
	std::vector<std::vector<pixel>>& render() override {
		return self.canvas;
	}

private:
	short width = 0;
	short height = 0;
	std::vector<std::vector<pixel>> canvas{};
};
class input final : public component {
public:
	bool password = false;

public:
	input(std::string name, short length, std::string placeholder = "") {
		self.name = name;
		self.placeholder = placeholder;
		self.type = component_type::focus;
		self.length = static_cast<short>(name.size()) + length;
		self.cursor.field = static_cast<short>(name.size());
		self.canvas = std::vector<std::vector<pixel>>(1, std::vector<pixel>(length + name.size(), pixel()));
	}
	std::vector<std::vector<pixel>>& render() override {
		pixel focused_color = self.focus ? pixel(color::white, color::black) : pixel(color::gray, color::black);

		for (size_t i = 0; i < self.name.size(); i++)
			self.canvas[0][i].character = self.name[i];

		for (size_t i = self.name.size(); i < self.length; i++)
			self.canvas[0][i] = focused_color;

		if (!self.focus && self.content.empty() && !self.placeholder.empty())
			for (size_t i = self.cursor.start, f = self.name.size(); i < self.cursor.start + self.length - self.name.size(); i++, f++)
				if (i != self.placeholder.size())
					self.canvas[0][f].character = self.placeholder[i];
				else
					break;
		else if (self.password)
			for (size_t i = self.cursor.start, f = self.name.size(); i < self.cursor.start + self.length - self.name.size(); i++, f++)
				if (i != self.content.size())
					self.canvas[0][f].character = '*';
				else
					break;
		else
			for (size_t i = self.cursor.start, f = self.name.size(); i < self.cursor.start + self.length - self.name.size(); i++, f++)
				if (i != self.content.size())
					self.canvas[0][f].character = self.content[i];
				else
					break;

		if (self.focus)
			self.canvas[0][self.cursor.field].character = '_';

		return self.canvas;
	}
	bool on_event(const KEY_EVENT_RECORD& input_event) override {
		switch (input_event.wVirtualKeyCode) {
		case VK_RIGHT: /* right arrow */
			if (self.cursor.content < self.content.size()) {
				self.cursor.content++;

				if (self.cursor.field < self.length - 1)
					self.cursor.field++;
				else
					self.cursor.start++;
			}
			return true;
		case VK_LEFT: /* left arrow */
			if (self.cursor.content > 0) {
				self.cursor.content--;

				if (self.cursor.field > self.name.size())
					self.cursor.field--;
				else
					self.cursor.start--;
			}
			return true;
		case VK_BACK: /* backspace */
			if (self.cursor.content > 0) {
				self.content.erase(--self.cursor.content, 1);

				if (self.cursor.start != 0)
					self.cursor.start--;
				else
					self.cursor.field--;
			}
			return true;
		}

		if (input_event.uChar.AsciiChar >= 32 && input_event.uChar.AsciiChar <= 126) {
			self.content.insert(self.cursor.content++, 1, input_event.uChar.AsciiChar);

			if (self.cursor.field <= self.length - 2)
				self.cursor.field++;
			else
				self.cursor.start++;

			return true;
		}

		return false;
	}
	std::string value() const { return self.content; }

private:
	short length = 0;
	std::string name = "";
	std::string content = "";
	std::string placeholder = "";
	std::vector<std::vector<pixel>> canvas = {};
	class {
	public:
		short start = 0;
		short content = 0;
		short field = 0;
	} cursor;
};
class dropdown final : public component {
public:
	dropdown(std::string name, std::initializer_list<std::string> value, short show = 10) {
		self.type = component_type::focus;
		self.name = name;
		self.content = value;
		self.width = 58 + static_cast<short>(name.size());
		// self.width = static_cast<short>(std::max_element(value.begin(), value.end(), [](const std::string& v1, const std::string& v2) { return v1.size() < v2.size(); })->size()) + static_cast<short>(name.size());
		self.limit = static_cast<short>(value.size()) < show ? static_cast<short>(value.size()) : show;
		self.canvas = std::vector<std::vector<pixel>>(value.size() < show ? value.size() : show, std::vector<pixel>(self.width, pixel()));
		self.placeholder = std::vector<std::vector<pixel>>(1, std::vector<pixel>(self.width, pixel()));
	}
	std::vector<std::vector<pixel>>& render() override {
		/* add name into first of canvas */
		for (size_t i = 0; i < self.name.size(); i++)
			self.canvas[0][i].character = self.name[i];

		if (self.focus) {
			pixel focused_color = pixel(color::gray, color::black);
			for (short h = 0; h < self.limit; h++)
				for (size_t w = self.name.size(); w < self.width; w++)
					self.canvas[h][w] = focused_color;

			for (size_t w = self.name.size(); w < self.canvas[self.cursor].size(); w++)
				self.canvas[self.cursor][w].background = color::white;

			for (size_t h = 0, s = start; s < self.limit + start; h++, s++)
				for (size_t w = 0, ns = self.name.size(); w < self.content[s].size(); w++, ns++)
					if (ns == self.width)
						break;
					else
						self.canvas[h][ns].character = self.content[s][w];

			return self.canvas;
		}
		else {
			self.placeholder = std::vector<std::vector<pixel>>(1, std::vector<pixel>(self.width, pixel()));

			for (size_t i = 0; i < self.name.size(); i++)
				self.placeholder[0][i].character = self.name[i];

			for (size_t w = name.size(), i = 0; w < self.width; w++, i++)
				if (w == self.width || i == self.content[self.current_index].size())
					break;
				else
					self.placeholder[0][w].character = self.content[self.current_index][i];

			return self.placeholder;
		}
	}
	bool on_event(const KEY_EVENT_RECORD& input_event) {
		switch (input_event.uChar.AsciiChar) {
		case 'j':
		case 'J':
			if (self.cursor < limit - 1)
				self.cursor++, self.current_index++;
			else if (self.start < self.content.size() - limit)
				self.start++, self.current_index++;
			return true;
		case 'k':
		case 'K':
			if (self.cursor > 0)
				self.cursor--, self.current_index--;
			else if (start > 0)
				self.start--, self.current_index--;
			return true;
		}

		return false;
	}
	std::string value() const {
		return self.content[self.current_index];
	}

private:
	short width = 0;
	short cursor = 0;
	short start = 0;
	short limit = 0;
	short current_index = 0;
	std::string name = "";
	std::vector<std::string> content = {};
	std::vector<std::vector<pixel>> canvas = {};
	std::vector<std::vector<pixel>> placeholder = {};
};
class window {
public:
	window(short width, short height) :
		width(width), height(height), current_component(0), canvas(height, std::vector<pixel>(width, pixel())), clear(canvas) {}
	void add(std::shared_ptr<component> value) {
		self.components.push_back(value);

		if (value->type == component_type::focus)
			self.focusable_component.push_back(self.components.size() - 1);
	}

private:
	friend class console;
	short width = 0;
	short height = 0;
	short current_component = 0;
	std::vector<std::vector<pixel>> canvas = {};
	std::vector<std::vector<pixel>> clear = {};
	std::vector<std::shared_ptr<component>> components = {};
	std::vector<size_t> focusable_component = {};
};
class console {
public:
	void run() {
		self.main();
	}

protected:
	console(short width, short height) :
		width(width), height(height) {
		SMALL_RECT window_size = { 0, 0, width - 1, height - 1 };
		SMALL_RECT default_size = { 0, 0, 119, 29 };
		COORD max_size = GetLargestConsoleWindowSize(output_handle);

		if (width < max_size.X && height < max_size.Y) {
			SetConsoleWindowInfo(self.output_handle, true, &window_size);
			SetConsoleScreenBufferSize(self.output_handle, { width, height });
			SetConsoleWindowInfo(self.output_handle, true, &window_size);
		}
		else {
			SetConsoleWindowInfo(self.output_handle, true, &default_size);
			SetConsoleScreenBufferSize(self.output_handle, { 120, 30 });
			SetConsoleWindowInfo(self.output_handle, true, &default_size);
		}
	}
	virtual void main() = 0;
	void render(window& window) {
		/* clearing environment */
		self.current_height = 0;
		self.output = "\x1b[0;0H";
		self.component_rendered.clear();
		window.canvas = window.clear;

		window.components[window.focusable_component[window.current_component]]->focus = true;

		/* render every single component */
		for (const auto& component : window.components)
			self.component_rendered.push_back(component->render());

		/* apply rendered component into canvas */
		for (size_t i = 0; i < self.component_rendered.size(); i++) {
			for (size_t h = 0; h < self.component_rendered[i].size(); h++) {
				if (self.current_height == window.height)
					break;

				for (size_t w = 0; w < self.component_rendered[i][h].size(); w++) {
					if (w == window.width)
						break;

					window.canvas[self.current_height][w] = self.component_rendered[i][h][w];
				}

				self.current_height++;
			}

			if (self.component_rendered[i].size() < 1)
				self.current_height++;
		}

		/* print into buffer */
		for (size_t h = 0; h < window.canvas.size(); h++) {
			for (size_t w = 0; w < window.canvas[h].size(); w++)
				self.output += window.canvas[h][w].tostring();

			self.output += '\n';
		}
		self.output.pop_back();
		std::cout << self.output;

		/* event handling */
		ReadConsoleInput(self.input_handle, &self.input_event, 1, &self.input_size);

		/* extract event */
		if (self.input_event.EventType == KEY_EVENT && self.input_event.Event.KeyEvent.bKeyDown) {
			// if key is overrided by component
			if (window.components[window.focusable_component[window.current_component]]->on_event(self.input_event.Event.KeyEvent))
				return;

			// if shift+tab is pressed
			if (GetKeyState(VK_SHIFT) & 0x8000 && self.input_event.Event.KeyEvent.wVirtualKeyCode == VK_TAB) {
				if (window.current_component > 0)
					window.components[window.focusable_component[window.current_component--]]->focus = false;
				else
					window.components[window.focusable_component[window.current_component]]->focus = false, window.current_component = static_cast<short>(window.focusable_component.size() - 1);
				return;
			}
			/* common key by virtual */
			else {
				switch (self.input_event.Event.KeyEvent.wVirtualKeyCode) {
				case VK_DOWN:
					if (window.current_component < window.focusable_component.size() - 1)
						window.components[window.focusable_component[window.current_component++]]->focus = false;
					return;
				case VK_UP:
					if (window.current_component > 0)
						window.components[window.focusable_component[window.current_component--]]->focus = false;
					return;
				case VK_TAB:
					if (window.current_component < (window.focusable_component.size() - 1))
						window.components[window.focusable_component[window.current_component++]]->focus = false;
					else
						window.components[window.focusable_component[window.current_component]]->focus = false, window.current_component = 0;
					return;
				}
			}

			/* common key by ascii */
			switch (self.input_event.Event.KeyEvent.uChar.AsciiChar) {
			case 'j':
			case 'J':
				if (window.current_component < (window.focusable_component.size() - 1))
					window.components[window.focusable_component[window.current_component++]]->focus = false;
				return;
			case 'k':
			case 'K':
				if (window.current_component > 0)
					window.components[window.focusable_component[window.current_component--]]->focus = false;
				return;
			}
		}
	}
	void disable_maximize_button() {
		SetWindowLong(GetConsoleWindow(), GWL_STYLE, GetWindowLong(GetConsoleWindow(), GWL_STYLE) & ~WS_MAXIMIZEBOX);
	}
	void disable_minimize_button() {
		SetWindowLong(GetConsoleWindow(), GWL_STYLE, GetWindowLong(GetConsoleWindow(), GWL_STYLE) & ~WS_MINIMIZEBOX);
	}
	void disable_resize_window() {
		SetWindowLong(GetConsoleWindow(), GWL_STYLE, GetWindowLong(GetConsoleWindow(), GWL_STYLE) & ~WS_SIZEBOX);
	}
	void cursor_visible(bool flag) {
		flag ? std::cout << "\x1b[?25h" : std::cout << "\x1b[?25l";
	}

protected:
	HANDLE output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE input_handle = GetStdHandle(STD_INPUT_HANDLE);
	INPUT_RECORD input_event = {};
	DWORD input_size = 0;
	short width = 0;
	short height = 0;
	short current_height = 0;
	std::string output = "";
	std::vector<std::vector<std::vector<pixel>>> component_rendered = {};
};
class application final : public console {
public:
	application() :
		console(150, 37) {
		self.disable_maximize_button();
		self.disable_minimize_button();
		self.disable_resize_window();
		self.cursor_visible(false);
	}

private:
	void main() override {
		auto main_window = window(self.width, self.height);
		auto input_username = std::make_shared<input>("Username         : ", 20, "John Doe");
		auto input_password = std::make_shared<input>("Password         : ", 20, "******");
		input_password->password = true;
		auto dropdown_lain = std::make_shared<dropdown>(
			"Pilih salah satu : ",
			std::initializer_list<std::string> {
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
				"Vision - oriented secondary project",
				"Adaptive bi - directional hierarchy",
				"Adaptive intangible frame",
				"Advanced static process improvement",
				"Ameliorated directional emulation",
				"Assimilated 24 / 7 archive",
				"Automated 4thgeneration website",
				"Balanced analyzing groupware",
				"Balanced multimedia knowledgebase",
				"Centralized attitude - oriented capability",
		}
		);
		auto button_login = std::make_shared<button>("Login", [&]() {
			bool loop = true;
			auto window_confirm = window(self.width, self.height);
			auto button_ok = std::make_shared<button>("Ok", [&]() { loop = false; });

			window_confirm.add(std::make_shared<text>("======================"));
			window_confirm.add(std::make_shared<text>("   SIMPLE DASHBOARD"));
			window_confirm.add(std::make_shared<text>("======================"));

			if (input_username->value() == "fahrisyn13" && input_password->value() == "localhost")
				window_confirm.add(std::make_shared<text>("Selamat anda berhasil login!"));
			else
				window_confirm.add(std::make_shared<text>("Username atau password salah!"));

			window_confirm.add(button_ok);

			while (loop) {
				self.render(window_confirm);
			}
			});
		auto button_exit = std::make_shared<button>("Exit", []() { exit(0); });

		main_window.add(std::make_shared<text>("======================"));
		main_window.add(std::make_shared<text>("   SIMPLE DASHBOARD"));
		main_window.add(std::make_shared<text>("======================"));
		main_window.add(std::make_shared<text>("Silakan masuk menggunakan akun yang sudah terdaftar."));
		main_window.add(std::make_shared<space>(self.width, 1));
		main_window.add(input_username);
		main_window.add(input_password);
		main_window.add(dropdown_lain);
		main_window.add(button_login);
		main_window.add(button_exit);

		while (true) {
			self.render(main_window);

		}
	}
} app;

int main() {
	app.run();
}