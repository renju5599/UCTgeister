#pragma once

#include <vector>
#include <map>
#include <thread>
#include <mutex>

#include "types.h"

#define MAXPLAY 300	//�ő�萔 (�����ň�������)
#define MAX_VALUE (1LL << 30)
#define FACTOR (pow(2, 0.5))
#define WIN_VALUE 1
#define LOSE_VALUE -1
#define DRAW_VALUE 0

using namespace std;

typedef int NodeNum;

struct NodeValue
{
	int win = 0;
	int play = 0;
	double comp = MAX_VALUE;

	bool operator<(const NodeValue& right) const
	{
		return comp < right.comp;
	}
	bool operator>(const NodeValue& right) const
	{
		return comp > right.comp;
	}
	bool operator==(const NodeValue& right) const
	{
		return (win == right.win && play == right.play && comp == right.comp);
	}
};
struct Node
{
	Board board = {};
	NodeValue value = {};
	vector<Hash> nextnodes;	//�q�m�[�h��ۑ� (Nodes�̃n�b�V���l���Ǘ����邾��)
	bool finish = 0;

	bool operator==(const Node& right) const
	{
		return (board == right.board && value == right.value && nextnodes == right.nextnodes);
	}
};

class UCT	//�Q�[���I���܂�1�̒T���؂Ŋ������������
{
private:
	//�T����
	vector<Node> Nodes;	//�Ֆʂ�ۑ�����m�[�h�B 
	map<Board, int> board_index;	//�Ֆʂɑ΂���m�[�h�̓Y������ۑ�����

	vector<thread> threads;
	mutex mtx;

	NodeNum nodenum;	//�ؑS�̂̃m�[�h��
	int total_play;	//�v���C�A�E�g�񐔂̑��a
	NodeNum playnodenum;	//�Q�[���i�s��̌��݂̃m�[�h
	int playnum;	//�Q�[���̃^�[����
	Pieces pieces;	//��̔ԍ��ƈʒu���֘A�t����
	Board playboard;

	//UCT�̊e�m�[�h�̉��l(?)���v�Z�E�ۑ�����
	void UpdateBoardValue(NodeValue& v);
	inline double compare(int win, int play)
	{
		return ((double)win / play) + FACTOR * pow(log(total_play) / play, 0.5);
	}

	//UCT���̃v���C�A�E�g������֐�
	int playout(bool nowPlayer, int playoutnum, Board nowboard);

	//�؂�"�e�m�[�h","�q�m�[�h"������ƁA�J�ڎ���MoveCommand��Ԃ�
	MoveCommand toMoveCommand(NodeNum from, NodeNum to);

public:
	//������
	UCT(Recieve str);

	// ���ۂ̔Ֆʂ̃m�[�h�Ɉړ�
	void SetNode(Recieve str);

	//UCT�T��������
	void Search();
	void ParallelSearch();

	//root�m�[�h����ǂ̎��I��Ŏw�����B�m�[�h�̓Y������Ԃ�
	NodeNum Choice();

	//�m�[�h��J�ڂ��āA�T�[�o�[�ɑ��镶�����Ԃ��i�����Ă͂Ȃ��j
	Send MoveNode(NodeNum move_nodenum);

	void PrintStatus();

}; //class MCT

