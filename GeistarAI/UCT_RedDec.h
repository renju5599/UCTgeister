#pragma once

#include <vector>
#include <unordered_map>
#include <random>

#include "types.h"
#include "rule.h"


using namespace std;

namespace UCT_RedDec
{

	typedef int NodeNum;

	struct NodeValue
	{
		//double win = 0;
		int win = 0;
		int play = 0;

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

	class UCT
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
		//vector<int> red_rate; //�ԋ
		BitBoard red_pos;	//����̐ԋ�i�ꏊ�j
		BitBoard blue_pos;	//red_pos�ȊO�̋�

		clock_t start_time;

		random_device rnd;
		mt19937 mt;
		uniform_int_distribution<> rand[65];


		//UCT���̃v���C�A�E�g������֐�
		//double playout(bool nowPlayer, int playoutnum, Board nowboard);
		int playout(bool nowPlayer, int playoutnum, Board nowboard);
		int playout_P(bool nowPlayer, int playoutnum, Board nowboard);

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
		void Search(int playout_type);

		//root�m�[�h����ǂ̎��I��Ŏw�����B�m�[�h�̓Y������Ԃ�
		NodeNum Choice();

		//�m�[�h��J�ڂ��āA�T�[�o�[�ɑ��镶�����Ԃ��i�����Ă͂Ȃ��j
		Send MoveNode(NodeNum move_nodenum);
		
		//red_pos�������̍s���ɍ��킹�čX�V����
		void changeRed_pos(NodeNum to);

		void PrintStatus();

	}; //class UCT
}