#pragma once

#include <vector>
#include <unordered_map>
#include<random>

#include "types.h"
#include "rule.h"

using namespace std;

namespace UCT_RF
{

	typedef int NodeNum;

	struct NodeValue
	{
		int win = 0;
		int play = 0;
		double NN_score = 0;
		//double comp = MAX_VALUE;

		bool operator==(const NodeValue& right) const
		{
			return (win == right.win && play == right.play /*&& comp == right.comp*/);
		}
	};
	struct Node
	{
		Board board = {};
		NodeValue value = {};
		vector<NodeNum> nextnodes;	//�q�m�[�h��ۑ� (Nodes�̃n�b�V���l���Ǘ����邾��)
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
		unordered_map<Board, NodeNum, BoardHash> board_index;	//�Ֆʂɑ΂���m�[�h�̓Y������ۑ�����

		unordered_map<NodeNum, bool> used;
		NodeNum usenode[MAXPLAY + 2] = {};

		NodeNum nodenum;	//�ؑS�̂̃m�[�h��
		int total_play;	//�v���C�A�E�g�񐔂̑��a
		NodeNum playnodenum;	//�Q�[���i�s��̌��݂̃m�[�h
		int playnum;	//�Q�[���̃^�[����
		Board playboard;

		random_device rnd;
		mt19937 mt;
		uniform_int_distribution<> rand[65];

		//UCT�̊e�m�[�h�̉��l(?)���v�Z�E�ۑ�����
		//void UpdateBoardValue(NodeValue& v);

		int NextBoard_byNN(Board nextboards[32], bool nowPlayer, int nextboards_size, BitsStatus& bsta_pre, double& res_pre);
		//UCT���̃v���C�A�E�g������֐�
		int playout(bool nowPlayer, int playoutnum, Board nowboard);

		//�؂�"�e�m�[�h","�q�m�[�h"������ƁA�J�ڎ���MoveCommand��Ԃ�
		MoveCommand toMoveCommand(NodeNum from, NodeNum to);

		void search_tree(int& turnnum, int& search_node, bool& nowPlayer);
		int expansion(int search_node, bool nowPlayer);

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

}