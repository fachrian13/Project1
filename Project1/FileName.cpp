#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <conio.h>

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
	brightgreen = 92,
	brightyellow = 93,
	brightblue = 94,
	brightmagenta = 95,
	brightcyan = 96,
	brightwhite = 97
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

	friend bool operator!=(const pixel& current, const pixel& other) {
		return !(current == other);
	}

	friend std::ostream& operator <<(std::ostream& out, const pixel& value) {
		return out << "\x1b["
			<< std::to_string(static_cast<unsigned>(value.foreground))
			<< ";"
			<< std::to_string(static_cast<unsigned>(value.background) + 10)
			<< "m"
			<< value.character
			<< "\x1b[0m";
	}
};

enum class component_type {
	render_only,
	focusable
};

class component {
public:
	virtual std::vector<std::vector<pixel>> render() = 0;
	virtual bool on_focus(int key) { return false; }
	virtual void set_focus(bool value) { self.focus = value; }
	virtual bool get_focus() { return self.focus; }
	component_type get_type() { return self.type; }

protected:
	bool focus = false;
	component_type type = component_type::render_only;
};

class text : public component {
public:
	text(std::string value) :
		content(value), canvas(1, std::vector<pixel>(self.content.size(), pixel())) {}

	std::vector<std::vector<pixel>> render() override {
		for (size_t i = 0; i < self.content.size(); i++)
			canvas[0][i].character = self.content[i];

		return self.canvas;
	}

private:
	std::string content;
	std::vector<std::vector<pixel>> canvas;
};

class input : public component {
public:
	input(std::string name, unsigned length, std::string placeholder = "") :
		name(name), placeholder(placeholder) {
		self.type = component_type::focusable;
		self.canvas = std::vector<std::vector<pixel>>(1, std::vector<pixel>(length + name.size(), pixel(color::black, color::white)));
		self.length = name.size() + length;
		self.cursor.field = name.size();
	}

	std::vector<std::vector<pixel>> render() override {
		for (size_t i = 0; i < self.name.size(); i++)
			self.canvas[0][i].character = self.name[i];

		pixel focused = self.get_focus() ? pixel(color::white, color::black) : pixel(color::gray, color::black);
		for (unsigned i = name.size(); i < self.length; i++)
			self.canvas[0][i] = focused;

		if (!self.get_focus() && self.content.empty() && !self.placeholder.empty())
			for (unsigned i = self.cursor.start, f = self.name.size(); i < self.cursor.start + self.length + self.name.size(); i++, f++)
				if (i != self.placeholder.size())
					self.canvas[0][f].character = self.placeholder[i];
				else
					break;
		else if (self.password)
			for (unsigned i = self.cursor.start, f = self.name.size(); i < self.cursor.start + self.length - self.name.size(); i++, f++)
				if (i != self.content.size())
					self.canvas[0][f].character = '*';
				else
					break;
		else
			for (unsigned i = self.cursor.start, f = self.name.size(); i < self.cursor.start + self.length - self.name.size(); i++, f++)
				if (i != self.content.size())
					self.canvas[0][f].character = self.content[i];
				else
					break;

		if (self.get_focus())
			self.canvas[0][self.cursor.field].character = '_';

		return self.canvas;
	}

	bool on_focus(int key) override {
		switch (key) {
		case 77: // right arrow
			if (self.cursor.content < self.content.size()) {
				self.cursor.content++;

				if (self.cursor.field < self.length - 1)
					self.cursor.field++;
				else
					self.cursor.start++;
			}
			return true;
			break;
		case 75: // left arrow
			if (self.cursor.content > 0) {
				self.cursor.content--;

				if (self.cursor.field > self.name.size())
					self.cursor.field--;
				else
					self.cursor.start--;
			}
			return true;
			break;
		case 8: // backspace
			if (self.cursor.content > 0) {
				self.content.erase(--self.cursor.content, 1);

				if (self.cursor.start != 0)
					self.cursor.start--;
				else
					self.cursor.field--;
			}
			return true;
			break;
		default:
			if (key >= 32 && key <= 126) { // writeable character
				self.content.insert(self.cursor.content++, 1, (char)key);

				if (self.cursor.field <= self.length - 2)
					self.cursor.field++;
				else
					self.cursor.start++;
				return true;
			}
		}

		return false;
	}

	void hide(bool value) { self.password = true; }

	std::string get_value() const { return self.content; }

private:
	bool password = false;
	unsigned length = 0;
	std::string name = "";
	std::string content = "";
	std::string placeholder = "";
	std::vector<std::vector<pixel>> canvas = {};

	struct {
		unsigned start = 0;
		unsigned content = 0;
		unsigned field = 0;
	} cursor;
};

class menu : public component {
public:
	menu(std::string name, std::vector<std::string> menu, unsigned width, unsigned show = 10) :
		name(name), menus(menu) {
		self.type = component_type::focusable;
		self.limit = menu.size() < show ? menu.size() : show;
		self.width = width + name.size();
		self.canvas = std::vector<std::vector<pixel>>(menu.size() < show ? menu.size() : show, std::vector<pixel>(self.width, pixel(color::black, color::white)));
	}

	std::vector<std::vector<pixel>> render() override {
		size_t name_size = self.name.size();

		for (size_t i = 0; i < name_size; i++)
			self.canvas[0][i].character = self.name[i];

		pixel focused = pixel(color::gray, color::black);
		for (unsigned h = 0; h < self.limit; h++)
			for (unsigned w = name_size; w < self.width; w++)
				self.canvas[h][w] = focused;

		for (unsigned h = 0, s = start; s < self.limit + start; h++, s++)
			for (unsigned w = 0, ns = name_size; w < self.menus[s].size(); w++, ns++)
				self.canvas[h][ns].character = self.menus[s][w];

		if (self.get_focus())
			for (unsigned w = name_size; w < self.canvas[cursor].size(); w++)
				self.canvas[self.cursor][w].background = color::white;

		return self.canvas;
	}

	bool on_focus(int key) override {
		switch (key) {
		case 106: // j
			if (self.cursor < limit - 1)
				self.cursor++;
			else if (start < self.menus.size() - limit)
				start++;
			return true;
			break;
		case 107: // k
			if (self.cursor > 0)
				self.cursor--;
			else if (start > 0)
				start--;
			return true;
			break;
		}
		return false;
	}

private:
	unsigned limit = 0;
	unsigned cursor = 0;
	unsigned start = 0;
	unsigned width = 0;
	std::string name = "";
	std::vector<std::string> menus = {};
	std::vector<std::vector<pixel>> canvas = {};
};

class button : public component {
public:
	button(std::string name, std::function<void()> onclick = []() {}) :
		name(name), onclick(onclick), canvas(1, std::vector<pixel>(name.size() + 2, pixel())) {
		self.type = component_type::focusable;
	}

	std::vector<std::vector<pixel>> render() override {
		self.canvas = std::vector<std::vector<pixel>>(1, std::vector<pixel>(self.name.size() + 2, self.get_focus() ? pixel(color::white, color::black) : pixel()));

		self.canvas[0].front().character = '[';
		for (size_t i = 0; i < self.name.size(); i++)
			self.canvas[0][i + 1].character = self.name[i];
		self.canvas[0].back().character = ']';

		return self.canvas;
	}

	bool on_focus(int key) {
		switch (key) {
		case 13:
			self.onclick();
			return true;
			break;
		}
		return false;
	}

private:
	std::string name;
	std::function<void()> onclick;
	std::vector<std::vector<pixel>> canvas;
};

class separator : public component {
public:
	separator(unsigned height = 1) :
		height(height), canvas(self.height, std::vector<pixel>(120, pixel())) {}

	std::vector<std::vector<pixel>> render() override {
		return self.canvas;
	}
private:
	unsigned height;
	std::vector<std::vector<pixel>> canvas;
};

class console {
public:
	console(unsigned width, unsigned height) :
		width(width), height(height), canvas(height, std::vector<pixel>(width, pixel())) {}

	void add(std::shared_ptr<component> value) {
		self.components.push_back(value);

		if (value->get_type() == component_type::focusable)
			self.focusable_component.push_back(self.components.size() - 1);
	}


	void stop() { self.loop = false; }

	void run() {
		if (!focusable_component.empty()) {
			unsigned current = 0;

			while (loop) {
				self.components[self.focusable_component[current]]->set_focus(true);
				self.write();

				int key = _getch();

				if (key == 224) {
					key = _getch();

					switch (key) {
					case 80: // down
						if (current < self.focusable_component.size() - 1)
							self.components[self.focusable_component[current++]]->set_focus(false);
						break;
					case 72: // up
						if (current > 0)
							self.components[self.focusable_component[current--]]->set_focus(false);
						break;
					default:
						if (self.components[self.focusable_component[current]]->on_focus(key))
							continue;
					}
				}
				else {
					if (self.components[self.focusable_component[current]]->on_focus(key))
						continue;
					else if (key == 106 && current < self.focusable_component.size() - 1)
						self.components[self.focusable_component[current++]]->set_focus(false);
					else if (key == 107 && current > 0)
						self.components[self.focusable_component[current--]]->set_focus(false);
					else if (key == 9) {
						if (current < self.focusable_component.size() - 1)
							self.components[self.focusable_component[current++]]->set_focus(false);
						else
							self.components[self.focusable_component[current]]->set_focus(false), current = 0;
					}
				}
			}
		}
		else {
			self.write();
			std::cin.get();
		}
	}

private:
	void write() {
		unsigned cursor_height = 0;
		std::string output = "\x1b[0;0H";
		std::vector<std::vector<std::vector<pixel>>> rendered;

		for (size_t i = 0; i < self.components.size(); i++)
			rendered.push_back(self.components[i]->render());

		for (unsigned i = 0; i < rendered.size(); i++) {
			for (unsigned h = 0; h < rendered[i].size(); h++) {
				if (cursor_height == self.height)
					break;

				for (unsigned w = 0; w < rendered[i][h].size(); w++) {
					if (w == self.width)
						break;

					self.canvas[cursor_height][w] = rendered[i][h][w];
				}
				cursor_height++;
			}
			if (rendered[i].size() < 1) cursor_height++;
		}

		for (const auto& height : self.canvas) {
			for (const auto& width : height)
				output += width.tostring();

			output += '\n';
		}
		output.pop_back();
		std::cout << output;
	}

private:
	bool loop = true;
	unsigned width = 0;
	unsigned height = 0;
	std::vector<std::shared_ptr<component>> components;
	std::vector<unsigned> focusable_component;
	std::vector<std::vector<pixel>> canvas = {};
};

int main() {
	auto c_form = console(120, 20);
	auto i_nama_lengkap = std::make_shared<input>("Nama Lengkap  : ", 50);
	auto i_gelar = std::make_shared<input>("Gelar Akademik: ", 50);
	auto m_pendidikan = std::make_shared<menu>("Pendidikan    : ", std::vector<std::string>{ "SD", "SMP", "SMA", "D1", "D2", "D3", "S1", "S2", "S3" }, 50);
	auto i_email = std::make_shared<input>("Email         : ", 50);
	auto i_hp = std::make_shared<input>("Nomor HP/WA   : ", 50);
	auto i_tempat_lahir = std::make_shared<input>("Tempat Lahir  : ", 50);
	auto i_alamat = std::make_shared<input>("Alamat        : ", 50);
	auto b_exit = std::make_shared<button>("Exit", [&c_form]() { c_form.stop(); });

	c_form.add(std::make_shared<text>("==========================="));
	c_form.add(std::make_shared<text>("   SILAKAN ISI DATA DIRI"));
	c_form.add(std::make_shared<text>("==========================="));
	c_form.add(std::make_shared<separator>());
	c_form.add(i_nama_lengkap);
	c_form.add(i_gelar);
	c_form.add(m_pendidikan);
	c_form.add(i_email);
	c_form.add(i_hp);
	c_form.add(i_tempat_lahir);
	c_form.add(i_alamat);
	c_form.add(b_exit);

	c_form.run();
}