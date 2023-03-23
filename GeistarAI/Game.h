#pragma once
#include <iostream>
#include <string>

#include <random>
#include <algorithm>

#include "client.h"
#include "types.h"
#include "UCT.h"
#include "UCT_withRF.h"
#include "UCT_withRFall.h"
#include "UCT_onlyRoot.h"
#include "UCT_Afterstates.h"
#include "UCT_RedDec.h"
#include "UCTandAlphaBeta_RedDec.h"
#include "UCTwithRFandABRed.h"
#include "UCTrootRFandABRed.h"

using namespace std;

namespace Game_
{
	const int WON = 1; // ����
	const int LST = 2; // ����
	const int DRW = 3; // ��������
	extern PieceNum piecenum[64];
	extern Point pieces[16];	//��̔ԍ��ƈʒu���֘A�t����
	extern int turn;
	extern int ver_num;
	extern bool purple;
	extern Board gameboard;

	bool startWith(string &s, string t);				// s�̐擪��t�̈�v����
	int isEnd(string s);												// �Q�[���̏I������
	string getEndInfo(string recv_msg);					// �I���̌���
	string setPosition();												// �ԋ�̔z�u(�����_��)
	int playgame(int port, string destination, int AI_kind); // �Q�[�����s��
}
