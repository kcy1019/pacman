#include<vector>
#include"timer.hxx"
#include"pacman.hxx"
#include"ghost.hxx"
#include"game.hxx"
#include"graphics.hxx"
using std::vector;

int main()
{
	GraphicsToolkit& pen = GraphicsToolkit::GetInstance();
	Game game(25, 25, 25 / 2);
	IntervalTimer main_clock(1. / 33.);
	main_clock.Loop([&game](long long current_time) {
			static const int KEY_ESC = 27;
			game.Process(current_time, [](const vector<vector<Cell>>& field,
										  const vector<Ghost>& ghosts,
										  const PacMan& pacman) {
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
