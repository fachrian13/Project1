#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>

#define self (*this)

enum class color : int {
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
class pixel final {
public:
	color background = color::black;
	color foreground = color::white;
	char character = ' ';

public:
	pixel() {}
	pixel(color background, color foreground, char character = ' ') {
		self.background = background;
		self.foreground = foreground;
		self.character = character;
	}
	inline auto to_string() const -> std::string {
		return "\x1b["
			+ std::to_string(static_cast<int>(self.foreground))
			+ ";"
			+ std::to_string(static_cast<int>(self.background) + 10)
			+ "m"
			+ self.character
			+ "\x1b[0m";
	}
	friend auto operator ==(const pixel& current, const pixel& other) -> bool {
		return current.background == other.background && current.foreground == other.foreground && current.character == other.character;
	}
	friend auto operator !=(const pixel& current, const pixel& other) -> bool {
		return !(current == other);
	}
};

using line = std::vector<pixel>;
using rectangle = std::vector<line>;

enum class component_type : int {
	render_only,
	has_focus
};
class component {
public:
	virtual auto render() -> rectangle & = 0;
	virtual auto on_event(const KEY_EVENT_RECORD&) -> bool { return false; }
	auto set_focus(bool value) -> void { self.focus = value; }
	auto get_focus() const -> bool { return self.focus; }
	auto set_type(component_type value) -> void { self.type = value; }
	auto get_type() const -> component_type { return self.type; }

private:
	bool focus = false;
	component_type type = component_type::render_only;
};

class button final : public component {
public:
	button(std::string name, std::function<void()> logic = []() {}) {
		self.set_type(component_type::has_focus);
		self.name = name;
		self.logic = logic;
	}
	auto render() -> rectangle & override {
		// fill canvas with empty pixel
		self.canvas = self.get_focus() ? rectangle(1, line(name.size() + 2, pixel(color::white, color::black))) : rectangle(1, line(name.size() + 2, pixel()));

		self.canvas[0].front().character = '[';
		for (size_t i = 0; i < self.name.size(); i++)
			self.canvas[0][i + 1].character = self.name[i];
		self.canvas[0].back().character = ']';

		return self.canvas;
	}
	auto on_event(const KEY_EVENT_RECORD& key) -> bool override {
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
class choice final : public component {
public:
	choice(std::string name, std::initializer_list<std::string> value) {
		self.set_type(component_type::has_focus);
		self.name = name;
		self.content = value;
		self.width = name.size() + value.size() - 1;

		for (const auto& i : value)
			self.width += i.size();
	}
	auto render() -> rectangle & override {
		size_t field = self.name.size();

		self.canvas = rectangle(1, line(self.width, pixel()));

		// add name into canvas
		for (size_t i = 0; i < self.name.size(); i++)
			self.canvas[0][i].character = self.name[i];

		// focus section
		if (self.get_focus()) {
			for (size_t i = 0; i < self.content.size(); i++) {
				for (size_t j = 0; j < self.content[i].size(); j++, field++)
					if (i == self.index)
						self.canvas[0][field] = pixel(color::white, color::black, self.content[i][j]);
					else
						self.canvas[0][field] = pixel(color::black, color::white, self.content[i][j]);

				if (field < self.width)
					self.canvas[0][field++].character = '|';
			}

			return self.canvas;
		}

		// not focus section
		for (size_t i = 0; i < self.content.size(); i++) {
			for (size_t j = 0; j < self.content[i].size(); j++, field++)
				if (i == self.index)
					self.canvas[0][field] = pixel(color::gray, color::black, self.content[i][j]);
				else
					self.canvas[0][field] = pixel(color::black, color::white, self.content[i][j]);

			if (field < self.width)
				self.canvas[0][field++].character = '|';
		}

		return self.canvas;
	}
	auto on_event(const KEY_EVENT_RECORD& key) -> bool {
		switch (key.uChar.AsciiChar) {
		case 'l':
		case 'L':
			if (self.index < self.content.size() - 1)
				index++;
			return true;
		case 'h':
		case 'H':
			if (self.index > 0)
				index--;
			return true;
		}

		return false;
	}
	auto value() const -> std::string { return self.content[self.index]; }

private:
	int width = 0;
	int index = 0;
	std::string name = "";
	std::vector<std::string> content = {};
	rectangle canvas = {};
};
class dropdown final : public component {
public:
	dropdown(std::string name, std::initializer_list<std::string> value, short show = 10, short width = 0) {
		self.set_type(component_type::has_focus);
		self.name = name;
		self.content = value;
		self.show = value.size() < show ? value.size() : show;
		self.width = width == 0 ? std::max_element(value.begin(), value.end(), [](const std::string& first, const std::string& second) { return first.size() < second.size(); })->size() + name.size() : width + name.size();
	}
	auto render() -> rectangle & override {
		// focus section
		if (self.get_focus()) {
			self.canvas = rectangle(self.show, line(self.width, pixel()));

			// add a name into canvas
			for (size_t i = 0; i < self.name.size(); i++)
				self.canvas[0][i].character = self.name[i];

			for (int h = 0; h < self.show; h++)
				for (size_t f = self.name.size(); f < self.width; f++)
					self.canvas[h][f] = pixel(color::gray, color::black);

			for (size_t h = 0, s = show_start; s < self.show + show_start; h++, s++)
				for (size_t w = 0, f = self.name.size(); w < self.content[s].size(); w++, f++)
					if (f == self.width)
						break;
					else
						self.canvas[h][f].character = self.content[s][w];

			for (size_t f = self.name.size(); f < self.canvas[self.cursor].size(); f++)
				self.canvas[self.cursor][f].background = color::white;

			return self.canvas;
		}

		// not focus section
		self.canvas = rectangle(1, line(self.width, pixel()));

		// add a name into canvas
		for (size_t i = 0; i < self.name.size(); i++)
			self.canvas[0][i].character = self.name[i];

		for (size_t f = self.name.size(); f < self.width; f++)
			self.canvas[0][f] = pixel(color::gray, color::black);

		for (size_t i = 0, f = self.name.size(); f < self.width; i++, f++) {
			if (i == self.content[self.index].size())
				break;
			self.canvas[0][f].character = self.content[self.index][i];
		}

		return self.canvas;
	}
	auto on_event(const KEY_EVENT_RECORD& key) -> bool override {
		switch (key.uChar.AsciiChar) {
		case 'j':
		case 'J':
			if (self.cursor < show - 1)
				self.cursor++, self.index++;
			else if (self.show_start < self.content.size() - show)
				self.show_start++, self.index++;
			return true;
		case 'k':
		case 'K':
			if (self.cursor > 0)
				self.cursor--, self.index--;
			else if (self.show_start > 0)
				self.show_start--, self.index--;
			return true;
		}

		return false;
	}
	auto value() const -> std::string { return self.content[self.index]; }

private:
	int show = 0;
	int width = 0;
	int cursor = 0;
	int show_start = 0;
	int index = 0;
	std::string name = "";
	std::vector<std::string> content = {};
	rectangle canvas = {};
};
class hlayout final : public component {
public:
	hlayout() {
		self.set_type(component_type::has_focus);
	}
	void add(std::shared_ptr<component> component) {
		self.components.push_back(component);

		if (component->get_type() == component_type::has_focus)
			self.focusable_components.push_back(self.components.size() - 1);
	}
	auto render() -> rectangle & override {
		int height = 0;
		int width = self.components.size() - 1;
		int component_width = 0;
		int field = 0;
		std::vector<rectangle> components_rendered = {};

		if (self.get_focus()) {
			for (size_t i = 0; i < self.components.size(); i++) {
				if (!self.focusable_components.empty() && i == self.current_component)
					self.components[self.focusable_components[self.current_component]]->set_focus(true);

				components_rendered.push_back(self.components[i]->render());
			}

			// width
			for (const auto& component : components_rendered)
				width += component[0].size();

			// height
			height = std::max_element(components_rendered.begin(), components_rendered.end(), [](const rectangle& a, const rectangle& b) { return a.size() < b.size(); })->size();

			self.canvas = rectangle(height, line(width, pixel()));

			for (size_t i = 0; i < components_rendered.size(); i++) {
				for (size_t h = 0; h < components_rendered[i].size(); h++) {
					field = component_width;

					for (size_t w = 0; w < components_rendered[i][h].size(); w++, field++)
						self.canvas[h][field] = components_rendered[i][h][w];
				}

				if (field < width)
					component_width++;

				component_width += components_rendered[i][0].size();
			}

			self.components[self.focusable_components[self.current_component]]->set_focus(false);

			return self.canvas;
		}

		// render all component
		for (const auto& component : self.components)
			components_rendered.push_back(component->render());

		// width
		for (const auto& component : components_rendered)
			width += component[0].size();

		// height
		height = std::max_element(components_rendered.begin(), components_rendered.end(), [](const rectangle& a, const rectangle& b) { return a.size() < b.size(); })->size();

		// not focus section
		self.canvas = rectangle(1, line(width, pixel()));

		for (size_t i = 0; i < components_rendered.size(); i++) {
			for (size_t h = 0; h < components_rendered[i].size(); h++)
				for (size_t w = 0; w < components_rendered[i][h].size(); w++, field++)
					self.canvas[0][field] = components_rendered[i][h][w];

			if (field < width)
				field++;
		}

		return self.canvas;
	}
	auto on_event(const KEY_EVENT_RECORD& key) -> bool override {
		if (self.components[self.focusable_components[self.current_component]]->on_event(key))
			return true;

		switch (key.uChar.AsciiChar) {
		case 'l':
			if (self.current_component < (self.focusable_components.size() - 1))
				self.components[self.focusable_components[self.current_component++]]->set_focus(false);
			return true;
		case 'h':
			if (self.current_component > 0)
				self.components[self.focusable_components[self.current_component--]]->set_focus(false);
			return true;
		}

		return false;
	}

private:
	size_t current_component = 0;
	std::vector<std::shared_ptr<component>> components = {};
	std::vector<size_t> focusable_components = {};
	rectangle canvas = {};
};
class input final : public component {
public:
	input(std::string name = "", int width = 30, std::string placeholder = "") {
		self.set_type(component_type::has_focus);
		self.name = name;
		self.width = width + name.size();
		self.placeholder = placeholder;
		self.cursor.field = name.size();
		self.canvas = rectangle(1, line(self.width, pixel()));

		for (size_t i = 0; i < name.size(); i++)
			self.canvas[0][i].character = self.name[i];
	}
	auto render() -> rectangle & override {
		pixel color = self.get_focus() ? pixel(color::white, color::black) : pixel(color::gray, color::black);

		for (size_t f = self.name.size(); f < self.width; f++)
			self.canvas[0][f] = color;

		if (self.content.empty() && !self.placeholder.empty()) {
			for (size_t i = 0, f = self.name.size(); i < placeholder.size(); i++, f++) {
				if (f == self.width)
					break;

				self.canvas[0][f].character = self.placeholder[i];
			}
		}
		else if (self.password) {
			for (size_t i = self.cursor.start, f = self.name.size(); i < self.cursor.start + self.width - self.name.size(); i++, f++) {
				if (i == self.content.size())
					break;

				self.canvas[0][f].character = '*';
			}
		}
		else {
			for (size_t i = self.cursor.start, f = self.name.size(); i < self.cursor.start + self.width - self.name.size(); i++, f++) {
				if (i == self.content.size())
					break;

				self.canvas[0][f].character = self.content[i];
			}
		}

		if (self.get_focus())
			self.canvas[0][self.cursor.field].character = '_';

		return self.canvas;
	}
	auto on_event(const KEY_EVENT_RECORD& key) -> bool override {
		switch (key.wVirtualKeyCode) {
		case VK_RIGHT:
			if (self.cursor.content < self.content.size()) {
				self.cursor.content++;

				if (self.cursor.field < self.width - 1)
					self.cursor.field++;
				else
					self.cursor.start++;
			}
			return true;
		case VK_LEFT:
			if (self.cursor.content > 0) {
				self.cursor.content--;

				if (self.cursor.field > self.name.size())
					self.cursor.field--;
				else
					self.cursor.start--;
			}
			return true;
		case VK_BACK:
			if (self.cursor.content > 0) {
				self.content.erase(--self.cursor.content, 1);

				if (self.cursor.start != 0)
					self.cursor.start--;
				else
					self.cursor.field--;
			}
			return true;
		}

		if (key.uChar.AsciiChar >= 32 && key.uChar.AsciiChar <= 126) {
			self.content.insert(self.cursor.content++, 1, key.uChar.AsciiChar);

			if (self.cursor.field <= self.width - 2)
				self.cursor.field++;
			else
				self.cursor.start++;

			return true;
		}

		return false;
	}
	auto value() const -> std::string { return self.content; }
	auto hide(bool value) -> void { self.password = value; }
	auto empty() const -> bool { return self.content.empty(); }

private:
	bool password = false;
	int width = 0;
	std::string name = "";
	std::string placeholder = "";
	std::string content = "";
	rectangle canvas = {};
	struct {
		int start = 0;
		int content = 0;
		int field = 0;
	} cursor;
};
class menu final : public component {
public:
	menu(std::initializer_list<std::string> menu, int show = 10, int width = 0) {
		self.set_type(component_type::has_focus);
		self.content = menu;
		self.show = menu.size() < show ? menu.size() : show;
		self.width = width == 0 ? std::max_element(menu.begin(), menu.end(), [](const std::string& first, const std::string& second) { return first.size() < second.size(); })->size() : width;
	}
	auto render() -> rectangle & override {
		self.canvas = rectangle(self.show, line(self.width, pixel()));

		for (size_t h = 0, s = show_start; s < self.show + show_start; h++, s++)
			for (size_t w = 0, f = 0; w < self.content[s].size(); w++, f++)
				if (f == self.width)
					break;
				else
					self.canvas[h][f].character = self.content[s][w];

		if (self.get_focus())
			for (size_t f = 0; f < self.canvas[self.cursor].size(); f++)
				self.canvas[self.cursor][f].background = color::white, self.canvas[self.cursor][f].foreground = color::black;

		return self.canvas;
	}
	auto on_event(const KEY_EVENT_RECORD& key) -> bool override {
		switch (key.uChar.AsciiChar) {
		case 'j':
		case 'J':
			if (self.cursor < show - 1)
				self.cursor++, self.current_index++;
			else if (self.show_start < self.content.size() - show)
				self.show_start++, self.current_index++;
			return true;
		case 'k':
		case 'K':
			if (self.cursor > 0)
				self.cursor--, self.current_index--;
			else if (self.show_start > 0)
				self.show_start--, self.current_index--;
			return true;
		}

		return false;
	}
	auto value() const -> std::string { return self.content[self.current_index]; }
	auto index() const -> int { return self.current_index; }

private:
	int show = 0;
	int width = 0;
	int cursor = 0;
	int show_start = 0;
	int current_index = 0;
	std::vector<std::string> content = {};
	rectangle canvas = {};
};
class space final : public component {
public:
	space(int height = 1) {
		self.height = height;
		self.canvas = rectangle(height, line(1, pixel()));
	}
	auto render() -> rectangle & override { return self.canvas; }

private:
	int height = 0;
	rectangle canvas = {};
};
class text final : public component {
public:
	text(std::string value) {
		self.value = value;
		self.canvas = rectangle(1, line(value.size(), pixel()));
	}
	auto render() -> rectangle & override {
		for (size_t i = 0; i < self.value.size(); i++)
			self.canvas[0][i].character = self.value[i];

		return self.canvas;
	}

private:
	std::string value = "";
	rectangle canvas = {};
};

class window final {
public:
	window(int width, int height) {
		self.width = width;
		self.height = height;
		self.canvas = rectangle(height, line(width, pixel()));
	}
	void add(std::shared_ptr<component> component) {
		self.components.push_back(component);

		if (component->get_type() == component_type::has_focus)
			self.focusable_components.push_back(self.components.size() - 1);
	}

private:
	int width = 0;
	int height = 0;
	size_t current_component = 0;
	std::vector<std::shared_ptr<component>> components = {};
	std::vector<size_t> focusable_components = {};
	rectangle canvas = {};
	friend class console;
};

class console {
public:
	void run() { self.main(); }

protected:
	short width = 0;
	short height = 0;
	HANDLE output_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE input_handle = GetStdHandle(STD_INPUT_HANDLE);
	INPUT_RECORD input_event = {};
	DWORD input_size = 0;

protected:
	console(COORD size = { 120, 30 }) {
		COORD max_size = GetLargestConsoleWindowSize(output_handle);

		if (size.X <= max_size.X && size.Y <= max_size.Y) {
			self.width = size.X;
			self.height = size.Y;

			SMALL_RECT window_size = { 0, 0, size.X - 1, size.Y - 1 };

			SetConsoleScreenBufferSize(self.output_handle, size);
			SetConsoleWindowInfo(self.output_handle, true, &window_size);
			SetConsoleScreenBufferSize(self.output_handle, size);
		}
		else {
			self.width = 120;
			self.height = 30;

			SMALL_RECT default_size = { 0, 0, 119, 29 };

			SetConsoleScreenBufferSize(self.output_handle, { 120, 30 });
			SetConsoleWindowInfo(self.output_handle, true, &default_size);
			SetConsoleScreenBufferSize(self.output_handle, { 120, 30 });
		}
	}
	auto disable_maximize_button() -> void { SetWindowLong(GetConsoleWindow(), GWL_STYLE, GetWindowLong(GetConsoleWindow(), GWL_STYLE) & ~WS_MAXIMIZEBOX); }
	auto disable_minimize_button() -> void { SetWindowLong(GetConsoleWindow(), GWL_STYLE, GetWindowLong(GetConsoleWindow(), GWL_STYLE) & ~WS_MINIMIZEBOX); }
	auto disable_resize_window() -> void { SetWindowLong(GetConsoleWindow(), GWL_STYLE, GetWindowLong(GetConsoleWindow(), GWL_STYLE) & ~WS_SIZEBOX); }
	auto set_cursor_visible(bool flag) -> void { flag ? std::cout << "\x1b[?25h" : std::cout << "\x1b[?25l"; }
	virtual auto main() -> void = 0;
	auto render(window& window) -> void {
		short current_height = 0;
		std::string output = "\x1b[0;0H";
		std::vector<rectangle> components_rendered = {};

		window.canvas = rectangle(window.height, line(window.width, pixel()));

		// render all component
		for (size_t i = 0; i < window.components.size(); i++) {
			if (!window.focusable_components.empty() && i == window.current_component)
				window.components[window.focusable_components[window.current_component]]->set_focus(true);

			components_rendered.push_back(window.components[i]->render());
		}

		// apply into canvas
		for (size_t i = 0; i < components_rendered.size(); i++) {
			for (size_t h = 0; h < components_rendered[i].size(); h++) {
				if (current_height == window.height)
					break;

				for (size_t w = 0; w < components_rendered[i][h].size(); w++) {
					if (w == window.width)
						break;

					window.canvas[current_height][w] = components_rendered[i][h][w];
				}

				current_height++;
			}

			if (components_rendered[i].size() < 1)
				current_height++;
		}

		// write out to buffer
		for (short h = 0; h < window.height; h++) {
			for (short w = 0; w < window.width; w++) {
				output += window.canvas[h][w].to_string();
			}

			output += '\n';
		}

		output.pop_back();
		std::cout << output;

		// event handling
		if (!window.focusable_components.empty()) {
			ReadConsoleInput(self.input_handle, &self.input_event, 1, &self.input_size);

			if (self.input_event.EventType == KEY_EVENT && self.input_event.Event.KeyEvent.bKeyDown) {
				// if key is overrided by component
				if (window.components[window.focusable_components[window.current_component]]->on_event(self.input_event.Event.KeyEvent))
					return;

				// shift+tab key
				if (GetKeyState(VK_SHIFT) & 0x8000 && self.input_event.Event.KeyEvent.wVirtualKeyCode == VK_TAB) {
					if (window.current_component > 0)
						window.components[window.focusable_components[window.current_component--]]->set_focus(false);
					else
						window.components[window.focusable_components[window.current_component]]->set_focus(false), window.current_component = window.focusable_components.size() - 1;
					return;
				}

				// common virtual key
				switch (self.input_event.Event.KeyEvent.wVirtualKeyCode) {
				case VK_DOWN:
					if (window.current_component < window.focusable_components.size() - 1) window.components[window.focusable_components[window.current_component++]]->set_focus(false);
					return;
				case VK_UP:
					if (window.current_component > 0)window.components[window.focusable_components[window.current_component--]]->set_focus(false);
					return;
				case VK_TAB:
					if (window.current_component < window.focusable_components.size() - 1) window.components[window.focusable_components[window.current_component++]]->set_focus(false);
					else window.components[window.focusable_components[window.current_component]]->set_focus(false), window.current_component = 0;
					return;
				}

				// common ascii key
				switch (self.input_event.Event.KeyEvent.uChar.AsciiChar) {
				case 'j':
				case 'J':
					if (window.current_component < (window.focusable_components.size() - 1)) window.components[window.focusable_components[window.current_component++]]->set_focus(false);
					return;
				case 'k':
				case 'K':
					if (window.current_component > 0) window.components[window.focusable_components[window.current_component--]]->set_focus(false);
					return;
				}
			}
		}
	}
};