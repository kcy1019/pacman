#include"game.hxx"
#include"timer.hxx"
#include"graphics.hxx"

int main()
{
	Game game(33, 33);
	// TODO: auto adjust dimensions --
	// use GraphicsToolkit.width, GraphicsToolkit.height
	IntervalTimer main_clock(1. / 30.);
	main_clock.Loop([&game](long long current_time) {
			static const int KEY_ESC = 27;
			game.Process(current_time, [](void) {
				int ch = (int)getch();
				if (ch == KEY_DOWN) return Game::DOWN;
				if (ch == KEY_UP) return Game::UP;
				if (ch == KEY_LEFT) return Game::LEFT;
				if (ch == KEY_RIGHT) return Game::RIGHT;
				if (ch == KEY_ESC  ||
					ch == (int)'Q' ||
					ch == (int)'q') return Game::ESC;
				return Game::NONE;
			});
	});
	return 0;
}
