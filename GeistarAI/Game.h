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
	const int WON = 1; // ����
	const int LST = 2; // ����
	const int DRW = 3; // ��������

	bool startWith(string &s, string t);				// s�̐擪��t�̈�v����
	int isEnd(string s);												// �Q�[���̏I������
	string getEndInfo(string recv_msg);					// �I���̌���
	string setPosition();												// �ԋ�̔z�u(�����_��)
	int playgame(int port, string destination); // �Q�[�����s��
}