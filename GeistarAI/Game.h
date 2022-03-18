#pragma once
#include <iostream>
#include <string>

#include <random>
#include <algorithm>

#include "client.h"
#include "types.h"
#include "UCT.h"
#include "UCT_withRF.h"
#include "UCT_onlyRoot.h"

using namespace std;

namespace Game_
{
	const int WON = 1; // 勝ち
	const int LST = 2; // 負け
	const int DRW = 3; // 引き分け
	extern PieceNum piecenum[64];
	extern Point pieces[16];	//駒の番号と位置を関連付ける

	bool startWith(string &s, string t);				// sの先頭とtの一致判定
	int isEnd(string s);												// ゲームの終了判定
	string getEndInfo(string recv_msg);					// 終了の原因
	string setPosition();												// 赤駒の配置(ランダム)
	int playgame(int port, string destination, int AI_kind); // ゲームを行う
}
