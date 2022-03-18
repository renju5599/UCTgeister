#include "weight.h"
#include <fstream>

namespace NN
{
	double weight[8128];
}

void NN::setweight()
{
	std::ifstream ifs("../weight.txt");
	for (int i = 0; i < 8128; i++)
	{
		ifs >> weight[i];
	}
	ifs.close();
}