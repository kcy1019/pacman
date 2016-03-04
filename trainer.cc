#define __TRAINER__
#include"simple.genetic.hxx"
#include<fstream>

int main()
{
    PacManTrainer trainer(100000, 200, 100, 0.05, 0.75, 3, "game1.dat");
    auto&& res = trainer.Train();
    string outs[] = {"gene1.dat", "gene2.dat"};
    res[0].ExportWeights(outs[0]);
    res[1].ExportWeights(outs[1]);
    return 0;
}
