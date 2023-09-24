#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>

#define self (*this)

enum class color : unsigned {
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
class pixel {
public:
	color background = color::black;
	color foreground = color::white;
	char character = ' ';

	pixel() {}
	pixel(color background, color foreground, char character = ' ') {
		self.background = background;
		self.foreground = foreground;
		self.character = character;
	}
	inline std::string to_string() const {
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

using line = std::vector<pixel>;
using rectangle = std::vector<line>;

enum class component_type : unsigned {
	render,
	focus
};
class component {
public:
	bool focus = false;
	component_type type = component_type::render;

public:
	virtual rectangle& render() = 0;
	virtual bool on_event(const KEY_EVENT_RECORD&) { return false; }
};
class text final : public component {
public:
	text(std::string value) {
		self.value = value;
		self.canvas = rectangle(1, line(value.size(), pixel()));
	}
	rectangle& render() override {
		for (size_t i = 0; i < self.value.size(); i++) self.canvas[0][i].character = self.value[i];

		return self.canvas;
	}

private:
	std::string value = "";
	rectangle canvas = {};
};
class button final : public component {
public:
	button(std::string name, std::function<void()> logic = []() {}) {
		self.type = component_type::focus;
		self.name = name;
		self.logic = logic;
	}
	rectangle& render() override {
		self.canvas = self.focus ? rectangle(1, line(name.size() + 2, pixel(color::white, color::black))) : rectangle(1, line(name.size() + 2, pixel()));

		self.canvas[0].front().character = '[';
		for (size_t i = 0; i < self.name.size(); i++) self.canvas[0][i + 1].character = self.name[i];
		self.canvas[0].back().character = ']';

		return self.canvas;
	}
	bool on_event(const KEY_EVENT_RECORD& key) override {
		switch (key.wVirtualKeyCode) {
		case VK_RETURN:
			self.logic();
			return true;
		}

		return false;
	}

private:
	std::string name = "";
	std::function<void()> logic = []() {};
	rectangle canvas = {};
};
class space final : public component {
public:
	space(short height = 1) {
		self.height = height;
		self.canvas = rectangle(height, line(1, pixel()));
	}
	rectangle& render() override { return self.canvas; }

private:
	short height = 0;
	rectangle canvas = {};
};
class input final : public component {
public:
	bool password = false;

public:
	input(std::string name, short width, std::string placeholder = "") {
		self.type = component_type::focus;
		self.name = name;
		self.width = width + static_cast<short>(name.size());
		self.placeholder = placeholder;
		self.cursor.field = static_cast<short>(name.size());
		self.canvas = rectangle(1, line(self.width, pixel()));

		/* add name into first section of canvas */
		for (size_t i = 0; i < name.size(); i++) self.canvas[0][i].character = self.name[i];
	}
	rectangle& render() override {
		pixel color = self.focus ? pixel(color::white, color::black) : pixel(color::gray, color::black);

		/* set color */
		for (size_t f = self.name.size(); f < self.width; f++) self.canvas[0][f] = color;

		/* add content into canvas */
		if (!self.focus && self.content.empty() && !self.placeholder.empty()) {
			for (size_t i = 0, f = self.name.size(); i < placeholder.size(); i++, f++) {
				if (f == self.width) break;

				self.canvas[0][f].character = self.placeholder[i];
			}
		}
		else if (self.password) {
			for (size_t i = 0, f = self.name.size(); i < self.content.size(); i++, f++) {
				if (f == self.width) break;

				self.canvas[0][f].character = '*';
			}
		}
		else {
			for (size_t i = self.cursor.start, f = self.name.size(); i < self.cursor.start + self.width - self.name.size(); i++, f++) {
				if (i == self.content.size()) break;

				self.canvas[0][f].character = self.content[i];
			}
		}

		/* set cursor into current index of content */
		if (self.focus) self.canvas[0][self.cursor.field].character = '_';

		return self.canvas;
	}
	bool on_event(const KEY_EVENT_RECORD& key) override {
		switch (key.wVirtualKeyCode) {
		case VK_RIGHT:
			if (self.cursor.content < self.content.size()) {
				self.cursor.content++;

				if (self.cursor.field < self.width - 1) self.cursor.field++;
				else self.cursor.start++;
			}
			return true;
		case VK_LEFT:
			if (self.cursor.content > 0) {
				self.cursor.content--;

				if (self.cursor.field > self.name.size()) self.cursor.field--;
				else self.cursor.start--;
			}
			return true;
		case VK_BACK:
			if (self.cursor.content > 0) {
				self.content.erase(--self.cursor.content, 1);

				if (self.cursor.start != 0) self.cursor.start--;
				else self.cursor.field--;
			}
			return true;
		}

		if (key.uChar.AsciiChar >= 32 && key.uChar.AsciiChar <= 126) {
			self.content.insert(self.cursor.content++, 1, key.uChar.AsciiChar);

			if (self.cursor.field <= self.width - 2) self.cursor.field++;
			else self.cursor.start++;

			return true;
		}

		return false;
	}
	std::string value() const { return self.content; }

private:
	std::string name = "";
	short width = 0;
	std::string placeholder = "";
	std::string content = "";
	rectangle canvas = {};
	struct {
		short start = 0;
		short content = 0;
		short field = 0;
	} cursor;
};
class dropdown final : public component {
public:
	dropdown(std::string name, std::initializer_list<std::string> value, short width = 0, short show = 10) {
		self.type = component_type::focus;
		self.name = name;
		self.content = value;
		self.show = value.size() < show ? static_cast<short>(value.size()) : show;
		self.width = width == 0 ? static_cast<short>(std::max_element(value.begin(), value.end(), [](const std::string& v1, const std::string& v2) { return v1.size() < v2.size(); })->size()) + static_cast<short>(name.size()) : width + static_cast<short>(name.size());
		self.canvas = rectangle(self.show, line(self.width, pixel()));
		self.placeholder = rectangle(1, line(self.width, pixel()));

		/* add name into first section of canvas and placeholder */
		for (size_t i = 0; i < name.size(); i++) self.canvas[0][i].character = self.name[i];
		for (size_t i = 0; i < name.size(); i++) self.placeholder[0][i].character = self.name[i];
	}
	rectangle& render() override {
		/* menu section */
		if (self.focus) {
			/* set pixel */
			for (short h = 0; h < self.show; h++) {
				for (size_t f = self.name.size(); f < self.width; f++) self.canvas[h][f] = pixel(color::gray, color::black);
			}

			/* set color for current index */
			for (size_t f = self.name.size(); f < self.canvas[self.cursor].size(); f++) self.canvas[self.cursor][f].background = color::white;

			for (size_t h = 0, s = show_start; s < self.show + show_start; h++, s++) {
				for (size_t w = 0, f = self.name.size(); w < self.content[s].size(); w++, f++) {
					if (f == self.width) break;

					self.canvas[h][f].character = self.content[s][w];
				}
			}

			return self.canvas;
		}

		/* placeholder section */
		/* set pixel */
		for (size_t f = self.name.size(); f < self.width; f++) self.canvas[0][f] = pixel();

		for (size_t i = 0, f = self.name.size(); f < self.width; i++, f++) {
			if (i == self.content[self.index].size()) break;

			self.placeholder[0][f].character = self.content[self.index][i];
		}

		return self.placeholder;
	}
	bool on_event(const KEY_EVENT_RECORD& key) override {
		switch (key.uChar.AsciiChar) {
		case 'j':
		case 'J':
			if (self.cursor < show - 1) self.cursor++, self.index++;
			else if (self.show_start < self.content.size() - show) self.show_start++, self.index++;
			return true;
		case 'k':
		case 'K':
			if (self.cursor > 0) self.cursor--, self.index--;
			else if (show_start > 0) self.show_start--, self.index--;
			return true;
		}

		return false;
	}
	std::string value() const { return self.content[self.index]; }

private:
	std::string name = "";
	std::vector<std::string> content = {};
	short show = 0;
	short width = 0;
	rectangle canvas = {};
	rectangle placeholder = {};
	short cursor = 0;
	short show_start = 0;
	short index = 0;
};

class window {
public:
	window(short width, short height) {
		self.width = width;
		self.height = height;
		self.canvas = rectangle(height, line(width, pixel()));
	}
	void add(std::shared_ptr<component> value) {
		self.components.push_back(value);

		if (value->type == component_type::focus) self.focusable_component.push_back(self.components.size() - 1);
	}

private:
	short width = 0;
	short height = 0;
	short current_component = 0;
	rectangle canvas = {};
	std::vector<std::shared_ptr<component>> components;
	std::vector<size_t> focusable_component = {};
	friend class console;
};

class console {
public:
	void run() { self.main(); }

protected:
	HANDLE output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE input_handle = GetStdHandle(STD_INPUT_HANDLE);
	INPUT_RECORD input_event = {};
	DWORD input_size = 0;
	short width = 0;
	short height = 0;
	short current_height = 0;
	std::string output = "";
	std::vector<rectangle> component_rendered = {};

protected:
	console(short width, short height) {
		self.width = width;
		self.height = height;

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
	virtual void main() = 0;
	void render(window& window) {
		/* clearing environment */
		self.current_height = 0;
		self.output = "\x1b[0;0H";
		self.component_rendered.clear();
		window.canvas = rectangle(window.height, line(window.width, pixel()));

		/* set focus into current component */
		if (!window.focusable_component.empty()) window.components[window.focusable_component[window.current_component]]->focus = true;

		/* render every single component */
		for (const auto& component : window.components) self.component_rendered.push_back(component->render());

		/* apply rendered component into canvas */
		for (size_t i = 0; i < self.component_rendered.size(); i++) {
			for (size_t h = 0; h < self.component_rendered[i].size(); h++) {
				if (self.current_height == window.height) break;

				for (size_t w = 0; w < self.component_rendered[i][h].size(); w++) {
					if (w == window.width) break;

					window.canvas[self.current_height][w] = self.component_rendered[i][h][w];
				}
				self.current_height++;
			}

			if (self.component_rendered[i].size() < 1) self.current_height++;
		}

		/* write into buffer */
		for (short h = 0; h < window.height; h++) {
			for (short w = 0; w < window.width; w++)
				self.output += window.canvas[h][w].to_string();

			self.output += '\n';
		}
		self.output.pop_back();
		std::cout << self.output;

		/* event handling */
		if (!window.focusable_component.empty()) {
			ReadConsoleInput(self.input_handle, &self.input_event, 1, &self.input_size);

			if (self.input_event.EventType == KEY_EVENT && self.input_event.Event.KeyEvent.bKeyDown) {
				/* if key is overrided by component */
				if (window.components[window.focusable_component[window.current_component]]->on_event(self.input_event.Event.KeyEvent)) return;

				/* if shift+tab is pressed */
				if (GetKeyState(VK_SHIFT) & 0x8000 && self.input_event.Event.KeyEvent.wVirtualKeyCode == VK_TAB) {
					if (window.current_component > 0) window.components[window.focusable_component[window.current_component--]]->focus = false;
					else window.components[window.focusable_component[window.current_component]]->focus = false, window.current_component = static_cast<short>(window.focusable_component.size() - 1);
					return;
				}

				/* common virtual key */
				switch (self.input_event.Event.KeyEvent.wVirtualKeyCode) {
				case VK_DOWN:
					if (window.current_component < window.focusable_component.size() - 1) window.components[window.focusable_component[window.current_component++]]->focus = false;
					return;
				case VK_UP:
					if (window.current_component > 0)window.components[window.focusable_component[window.current_component--]]->focus = false;
					return;
				case VK_TAB:
					if (window.current_component < window.focusable_component.size() - 1) window.components[window.focusable_component[window.current_component++]]->focus = false;
					else window.components[window.focusable_component[window.current_component]]->focus = false, window.current_component = 0;
					return;
				}

				/* common ascii key */
				switch (self.input_event.Event.KeyEvent.uChar.AsciiChar) {
				case 'j':
				case 'J':
					if (window.current_component < (window.focusable_component.size() - 1)) window.components[window.focusable_component[window.current_component++]]->focus = false;
					return;
				case 'k':
				case 'K':
					if (window.current_component > 0) window.components[window.focusable_component[window.current_component--]]->focus = false;
					return;
				}
			}
		}
	}
};