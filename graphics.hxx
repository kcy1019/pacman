#pragma once
#ifndef __GRAPHICS__
#define __GRAPHICS__
#define Singleton
#include<string>
#include<ncurses.h>
using std::string;

Singleton class GraphicsToolkit {
private:
	GraphicsToolkit()
	{
		putenv("TERM=xterm-256color");
		WINDOW *ret = initscr();
		getmaxyx(ret, height, width);
		nodelay(ret, TRUE);
		timeout(0);
		keypad(stdscr, TRUE);
		start_color();
		curs_set(0);
		noecho();
		for (int i = 0; i < COLORS; i++) {
			init_pair(i, i, 0);
		}
		refresh();
	}

	GraphicsToolkit(GraphicsToolkit const&) = delete;
	void operator = (GraphicsToolkit const&) = delete;

public:
	int width, height;

	static const enum {BLACK = 16, LEMON = 184, BLUE = 23,
					   RED = 124, CYAN = 122, YELLOW = 100,
					   PINK = 168, GREEN = 53, WHITE = 230,
					   GRAY = 242} ColorSet;

	static GraphicsToolkit& GetInstance()
	{
		static GraphicsToolkit instance;
		return instance;
	}

	inline void DrawPoint(int x, int y, int color, char ch = '#') {
		attron(COLOR_PAIR(color));
		mvprintw(y, x+x+0, "%c", ch);
		mvprintw(y, x+x+1, "%c", ch);
		attroff(COLOR_PAIR(color));
		refresh();
	}

	inline void DrawHLine(int y, int sx, int ex, int color, char ch = '-') {
		while (sx <= ex) {
			DrawPoint(sx, y, color, ch);
			++sx;
		}
	}

	inline void DrawVLine(int x, int sy, int ey, int color, char ch = '|') {
		while (sy <= ey) {
			DrawPoint(x, sy, color, ch);
			++sy;
		}
	}

	inline void DrawBox(int sx, int sy, int ex, int ey, int color) {
		DrawHLine(sy, sx, ex, color);
		DrawVLine(sx, sy, ey, color);
		DrawVLine(ex, sy, ey, color);
		DrawHLine(ey, sx, ex, color);
		DrawPoint(sx, sy, color, '+');
		DrawPoint(ex, sy, color, '+');
		DrawPoint(ex, ey, color, '+');
		DrawPoint(sx, ey, color, '+');
	}

	inline void DrawText(int x, int y, int color, const string& message) {
		attron(COLOR_PAIR(color));
		mvprintw(y, x+x, "%s", message.c_str());
		attroff(COLOR_PAIR(color));
		refresh();
	}

	inline void DrawMessageBox(int ldx, int ldy, int color, const string& message) {
		int rux = ldx + ((int)message.size() + 1) / 2 + 1,
			ruy = ldy + 2;
		DrawBox(ldx, ldy, rux, ruy, color);
		DrawText(ldx + 1, ldy + 1, color, message);
	}

	inline void Clear(void) {
		clear();
		refresh();
	}
};
#undef Singleton
#endif
