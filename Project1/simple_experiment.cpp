#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <conio.h>

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
	std::string value() const {		return self.content;	}

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
		auto input_username = std::make_shared<input>("Username: ", 20, "John Doe");
		auto input_password = std::make_shared<input>("Password: ", 20, "******");
		input_password->password = true;
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