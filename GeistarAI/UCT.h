#pragma once

#include <vector>

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
	vector<NodeNum> nextnodes;	//�q�m�[�h��ۑ� (Nodes�̓Y�������Ǘ����邾��)
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
	//vector<vector<NodeNum>> NextNodes;	//Nodes�ɂ܂Ƃ߂�΂悢�̂ł͂Ǝv�����̂ŏ���
	//
	NodeNum nodenum;	//�ؑS�̂̃m�[�h��
	int total_play;	//�v���C�A�E�g�񐔂̑��a
	NodeNum playnodenum;	//�Q�[���i�s��̌��݂̃m�[�h
	int playnum;	//�Q�[���̃^�[����
	Pieces pieces;	//��̔ԍ��ƈʒu���֘A�t����

	//UCT�ŗv��Ȃ��Ȃ����m�[�h���i�����Ɓj����
	void NodeErase(NodeNum root);

	//UCT�̊e�m�[�h�̉��l(?)���v�Z�E�ۑ�����
	void UpdateBoardValue(NodeValue& v);

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

	//root�m�[�h����ǂ̎��I��Ŏw�����B�m�[�h�̓Y������Ԃ�
	NodeNum Choice();

	//�m�[�h��J�ڂ��āA�T�[�o�[�ɑ��镶�����Ԃ��i�����Ă͂Ȃ��j
	Send MoveNode(NodeNum move_nodenum);

	void PrintStatus();

}; //class MCT

