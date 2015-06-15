#define __GRAPHICS__
#define __TRAINER__
#include"genetic.game.hxx"
#include<fstream>

int main()
{
	PacManTrainer trainer;
	auto&& res = trainer.Train();
	string outs[] = {"gene1.dat", "gene2.dat"};
	res[0].ExportWeights(outs[0]);
	res[1].ExportWeights(outs[1]);
	return 0;
}
