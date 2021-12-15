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
	const int WON = 1; // 勝ち
	const int LST = 2; // 負け
	const int DRW = 3; // 引き分け

	bool startWith(string &s, string t);				// sの先頭とtの一致判定
	int isEnd(string s);												// ゲームの終了判定
	string getEndInfo(string recv_msg);					// 終了の原因
	string setPosition();												// 赤駒の配置(ランダム)
	int playgame(int port, string destination); // ゲームを行う
}
