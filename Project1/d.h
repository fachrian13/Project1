#pragma once
#include <conio.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <iostream>

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
};
enum class component_type {
	render_only,
	focusable
};
class component {
public:
	virtual std::vector<std::vector<pixel>> render() = 0;
	virtual bool on_focus(int key) { return false; }
	bool get_focus() const { return self.focus; }
	void set_focus(const bool value) { self.focus = value; }
	component_type get_type() const { return self.type; }

protected:
	component_type type = component_type::render_only;
	bool focus = false;
};
class text final : public component {
public:
	text(std::string value) :
		content(value), canvas(1, std::vector<pixel>(value.size(), pixel())) {}
	std::vector<std::vector<pixel>> render() override {
		for (size_t i = 0; i < self.content.size(); i++)
			canvas[0][i].character = self.content[i];

		return self.canvas;
	}

private:
	std::string content;
	std::vector<std::vector<pixel>> canvas;
};
class input final : public component {
public:
	input(std::string name, unsigned length, std::string placeholder = "") :
		name(name), placeholder(placeholder) {
		self.type = component_type::focusable;
		self.length = static_cast<unsigned>(name.size()) + length;
		self.cursor.field = static_cast<unsigned>(name.size());
		self.canvas = std::vector<std::vector<pixel>>(1, std::vector<pixel>(length + name.size(), pixel()));

		/* adding name into front of canvas */
		for (size_t i = 0; i < self.name.size(); i++)
			self.canvas[0][i].character = self.name[i];
	}
	std::vector<std::vector<pixel>> render() override {
		// set color when focused or not
		pixel focused_color = self.focus ? pixel(color::white, color::black) : pixel(color::gray, color::black);
		for (size_t i = name.size(); i < self.length; i++)
			self.canvas[0][i] = focused_color;

		if (self.focus && self.content.empty() && self.placeholder.empty())
			for (size_t i = self.cursor.start, f = self.name.size(); i < self.cursor.start + self.length + self.name.size(); i++, f++)
				if (i != self.placeholder.size())
					self.canvas[0][f].character = self.placeholder[i];
				else
					break;
		else if (self.password)
			for (size_t i = self.cursor.start, f = self.name.size(); i < self.cursor.start + self.length + self.name.size(); i++, f++)
				if (i != self.content.size())
					self.canvas[0][f].character = '*';
				else
					break;
		else
			for (size_t i = self.cursor.start, f = self.name.size(); i < self.cursor.start + self.length + self.name.size(); i++, f++)
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
		case 77: /* right arrow */
			if (self.cursor.content < self.content.size()) {
				self.cursor.content++;

				if (self.cursor.field < self.length - 1)
					self.cursor.field++;
				else
					self.cursor.start++;
				return true;
			}

		case 75: /* left arrow */
			if (self.cursor.content > 0) {
				self.cursor.content--;

				if (self.cursor.field > self.name.size())
					self.cursor.field--;
				else
					self.cursor.start--;
				return true;
			}

		case 8: /* backspace */
			if (self.cursor.content > 0) {
				self.content.erase(--self.cursor.content, 1);

				if (self.cursor.start != 0)
					self.cursor.start--;
				else
					self.cursor.field--;
				return true;
			}

		default:
			if (key >= 32 && key <= 126) { /* writeable character */
				self.content.insert(self.cursor.content++, 1, static_cast<char>(key));

				if (self.cursor.field <= self.length - 2)
					self.cursor.field++;
				else
					self.cursor.start++;
				return true;
			}
		}

		return false;
	}
	std::string get_value() const { return self.content; }
	void hide(const bool value) { self.password = value; }

private:
	bool password = false;
	unsigned length = 0;
	std::string name = "";
	std::string content = "";
	std::string placeholder = "";
	std::vector<std::vector<pixel>> canvas = {};
	class {
	public:
		unsigned start = 0;
		unsigned content = 0;
		unsigned field = 0;
	} cursor;
};
class dropdown final : public component {
public:
	dropdown(std::string name, std::vector<std::string> value, unsigned length, unsigned show = 10) :
		name(name), content(value) {
		self.type = component_type::focusable;
		self.limit = static_cast<unsigned>(value.size()) < show ? static_cast<unsigned>(value.size()) : show;
		self.width = static_cast<unsigned>(name.size()) + length;
		self.canvas = std::vector<std::vector<pixel>>(value.size() < show ? value.size() : show, std::vector<pixel>(self.width, pixel(color::black, color::white)));
	}
	std::vector<std::vector<pixel>> render() override {
		/* adding name into front of canvas */
		for (size_t i = 0; i < self.name.size(); i++)
			self.canvas[0][i].character = self.name[i];

		if (self.focus) {
			/* erase all content in canvas */
			pixel focused_color = pixel(color::gray, color::black);
			for (unsigned h = 0; h < self.limit; h++)
				for (unsigned w = static_cast<unsigned>(self.name.size()); w < self.width; w++)
					self.canvas[h][w] = focused_color;

			/* set focused index background to white */
			for (size_t w = self.name.size(); w < self.canvas[self.cursor].size(); w++)
				self.canvas[self.cursor][w].background = color::white;

			/* adding content into field */
			for (unsigned h = 0, s = start; s < self.limit + start; h++, s++)
				for (size_t w = 0, ns = self.name.size(); w < self.content[s].size(); w++, ns++)
					if (ns == self.width)
						break;
					else self.canvas[h][ns].character = self.content[s][w];

			return self.canvas;
		}
		else {
			auto placeholder = std::vector<pixel>(self.width, pixel());

			/* adding name into front of canvas */
			for (size_t i = 0; i < self.name.size(); i++)
				placeholder[i].character = self.name[i];

			/* adding content into canvas */
			for (size_t w = name.size(), i = 0; w < self.width; w++, i++)
				if (w == self.width || i == self.content[self.current_index].size())
					break;
				else
					placeholder[w].character = self.content[self.current_index][i];

			return std::vector<std::vector<pixel>>(1, placeholder);
		}
	}
	bool on_focus(int key) override {
		switch (key) {
		case 106: /* j */
			if (self.cursor < limit - 1)
				self.cursor++, self.current_index++;
			else if (self.start < self.content.size() - limit)
				self.start++, self.current_index++;
			return true;

		case 107: /* k */
			if (self.cursor > 0)
				self.cursor--, self.current_index--;
			else if (start > 0)
				self.start--, self.current_index--;
			return true;
		}

		return false;
	}
	std::string get_value() const { return self.content[self.current_index]; }

private:
	unsigned limit = 0;
	unsigned cursor = 0;
	unsigned start = 0;
	unsigned width = 0;
	unsigned current_index = 0;
	std::string name = "";
	std::vector<std::string> content = {};
	std::vector<std::vector<pixel>> canvas = {};
};
class button final : public component {
public:
	button(std::string name, std::function<void()> on_click = {}) :
		name(name), on_click(on_click), canvas(1, std::vector<pixel>(name.size() + 2, pixel())) {
		self.type = component_type::focusable;
	}
	std::vector<std::vector<pixel>> render() override {
		self.canvas = std::vector<std::vector<pixel>>(1, std::vector<pixel>(self.name.size() + 2, self.focus ? pixel(color::white, color::black) : pixel()));

		self.canvas[0].front().character = '[';
		for (size_t i = 0; i < self.name.size(); i++)
			self.canvas[0][i + 1].character = self.name[i];
		self.canvas[0].back().character = ']';

		return self.canvas;
	}
	bool on_focus(int key) override {
		switch (key) {
		case 13: /* return */
			self.on_click();
			return true;
		}

		return false;
	}

private:
	std::string name = "";
	std::function<void()> on_click = {};
	std::vector<std::vector<pixel>> canvas = {};
};
class separator final : public component {
public:
	separator(unsigned height = 1, unsigned width = 120) :
		height(height), width(width), canvas(self.height, std::vector<pixel>(width, pixel())) {}
	std::vector<std::vector<pixel>> render() override { return self.canvas; }

private:
	unsigned height = 0;
	unsigned width = 0;
	std::vector<std::vector<pixel>> canvas = {};
};
class console {
public:
	console(unsigned width, unsigned height) :
		width(width), height(height), canvas(height, std::vector<pixel>(width, pixel())) {
	}
	void add(std::shared_ptr<component> value) {
		self.components.push_back(value);

		if (value->get_type() == component_type::focusable)
			self.focusable_components.push_back(self.components.size() - 1);
	}
	void run() {
		self.running = true;

		if (focusable_components.empty()) {
			self._write_();
			int key = _getch();
		}
		else {
			while (self.running) {
				self.components[self.focusable_components[self.current_component]]->set_focus(true);
				self._write_();

				int key = _getch();
				if (key == 224) {
					key = _getch();
					if (key == 80 && self.current_component < self.focusable_components.size() - 1) /* down */
						self.components[self.focusable_components[self.current_component++]]->set_focus(false);
					else if (key == 72 && self.current_component > 0) /* up */
						self.components[self.focusable_components[self.current_component--]]->set_focus(false);
					else if (self.components[self.focusable_components[self.current_component]]->on_focus(key))
						continue;
				}
				else {
					if (self.components[self.focusable_components[self.current_component]]->on_focus(key))
						continue;
					else if (key == 106 && self.current_component < self.focusable_components.size() - 1) /* j */
						self.components[self.focusable_components[self.current_component++]]->set_focus(false);
					else if (key == 107 && self.current_component > 0)
						self.components[self.focusable_components[self.current_component--]]->set_focus(false);
					else if (key == 9) {
						if (self.current_component < self.focusable_components.size() - 1)
							self.components[self.focusable_components[self.current_component++]]->set_focus(false);
						else
							self.components[self.focusable_components[self.current_component]]->set_focus(false), self.current_component = 0;
					}
				}
			}
		}
	}
	void stop() { self.running = false; }

private:
	void _write_() {
		unsigned cursor_height = 0;
		std::string output = "\x1b[0;0H";
		std::vector<std::vector<std::vector<pixel>>> rendered;
		self.canvas = std::vector<std::vector<pixel>>(height, std::vector<pixel>(self.width, pixel()));

		for (size_t i = 0; i < self.components.size(); i++)
			rendered.push_back(self.components[i]->render());

		for (size_t i = 0; i < rendered.size(); i++) {
			for (size_t h = 0; h < rendered[i].size(); h++) {
				if (cursor_height == self.height)
					break;

				for (size_t w = 0; w < rendered[i][h].size(); w++)
					if (w == self.width)
						break;
					else
						self.canvas[cursor_height][w] = rendered[i][h][w];

				cursor_height++;
			}

			if (rendered[i].size() < 1)
				cursor_height++;
		}

		for (size_t h = 0; h < self.canvas.size(); h++) {
			for (size_t w = 0; w < self.canvas[h].size(); w++)
				output += self.canvas[h][w].tostring();

			output += '\n';
		}

		output.pop_back();
		std::cout << output;
	}

private:
	unsigned width = 0;
	unsigned height = 0;
	unsigned current_component = 0;
	bool running = true;
	std::vector<std::shared_ptr<component>> components = {};
	std::vector<size_t> focusable_components = {};
	std::vector<std::vector<pixel>> canvas = {};
};