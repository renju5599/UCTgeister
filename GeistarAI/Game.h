#pragma once
#include <iostream>
#include <string>

#include <random>
#include <algorithm>

#include "client.h"
#include "types.h"
#include "UCT.h"

using namespace std;

namespace Game_
{
	const int WON = 1; // ¿
	const int LST = 2; // ¯
	const int DRW = 3; // ø«ª¯

	bool startWith(string &s, string t);				// sÌæªÆtÌêv»è
	int isEnd(string s);												// Q[ÌI¹»è
	string getEndInfo(string recv_msg);					// I¹ÌŽö
	string setPosition();												// ÔîÌzu(_)
	int playgame(int port, string destination); // Q[ðs€
}
