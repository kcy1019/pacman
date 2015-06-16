#include<vector>
#include"timer.hxx"
#include"pacman.hxx"
#include"ghost.hxx"
#include"game.hxx"
#include"graphics.hxx"
#include"genetic.game.hxx"
using std::vector;

int main()
{
	GraphicsToolkit& pen = GraphicsToolkit::GetInstance();
	Game game(20, 20, 20 / 2);
	IntervalTimer main_clock(1. / 9.);
	PacManGene brain(3);
	brain.ImportWeights("gene1.dat");
	main_clock.Loop([&game, &brain](long long current_time) {
			game.Process(current_time, [&brain, &current_time](const vector<vector<Cell>>& field,
										  const vector<Ghost>& ghosts,
										  const PacMan& pacman) {
				return (int)brain.DecideMove(current_time, field, ghosts, pacman);
			});
	});
	return 0;
}
