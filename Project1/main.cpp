#include <iostream>
#include <vector>
#include <string>
#include <functional>
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
	virtual void on_focus(int key) {}
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
		content(value), canvas(1, std::vector<pixel>(value.size(), pixel())) {}

	std::vector<std::vector<pixel>> render() override {
		for (size_t i = 0; i < self.content.size(); i++)
			self.canvas[0][i].character = self.content[i];

		return self.canvas;
	}

private:
	std::string content;
	std::vector<std::vector<pixel>> canvas;
};

class button : public component {
public:
	button(std::string name, std::function<void()> on_click) :
		name(name), on_click(on_click) {
		self.type = component_type::focusable;
	}

	virtual std::vector<std::vector<pixel>> render() override {
		self.canvas = std::vector<std::vector<pixel>>(1, std::vector<pixel>(name.size() + 2, self.get_focus() ? pixel(color::white, color::black) : pixel()));

		self.canvas[0].front().character = '[';
		for (size_t i = 0; i < self.name.size(); i++)
			self.canvas[0][i + 1].character = self.name[i];
		self.canvas[0].back().character = ']';

		return self.canvas;
	}

	void on_focus(int key) {
		switch (key) {
		case 13:
			self.on_click();
			break;
		}
	}

private:
	std::string name;
	std::function<void()> on_click;
	std::vector<std::vector<pixel>> canvas;
};

class window {
public:
	window(unsigned width, unsigned height) :
		width(width), height(height), canvas(height, std::vector<pixel>(width, pixel())) {}

	void add(std::shared_ptr<component> value) {
		self.components.push_back(value);
	}

	void render() {
		unsigned cursor_height = 0;
		std::string output = "\x1b[0;0H";
		std::vector<std::vector<std::vector<pixel>>> rendered;

		for (const auto& i : self.components)
			rendered.push_back(i->render());

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
	unsigned width;
	unsigned height;
	std::vector<std::shared_ptr<component>> components;
	std::vector<std::vector<pixel>> canvas;
};

class console {
public:
	console(unsigned width, unsigned height) :
		width(width), height(height), running(true) {
		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), {static_cast<short>(self.width), static_cast<short>(self.height)});
	}

	void run() {
		main();
	}

protected:
	virtual void main() = 0;

	void render(window& window) {
		window.render();
	}

	void stop() {
		running = false;
	}

	void exit() {
		running = false;
	}

private:
	unsigned width;
	unsigned height;
	bool running;
};

class application : public console {
public:
	application() : console(120, 30) {}

	void main() override {
		auto homepage = window(120, 30);
		auto login = button("Login", []() { true; });
		auto exit = button("Exit", [&]() { self.exit(); });

		homepage.add(std::make_shared<text>(text("hello world")));
		homepage.add(std::make_shared<button>(login));

		self.render(homepage);
	}
} app;

int main() {
	app.run();
	std::cin.get();
}