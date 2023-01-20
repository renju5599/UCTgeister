#pragma once

#include <vector>
#include <unordered_map>
#include <random>

#include "types.h"
#include "rule.h"


using namespace std;

namespace UCTandAlphaBeta_RedDec
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
		vector<NodeNum> nextnodes;	//子ノードを保存 (Nodesのハッシュ値を管理するだけ)
		bool finish = 0;

		bool operator==(const Node& right) const
		{
			return (board == right.board && value == right.value && nextnodes == right.nextnodes);
		}
	};

	class UCTandAlphaBeta
	{
	private:
		//探索木
		vector<Node> Nodes;	//盤面を保存するノード達 
		unordered_map<Board, NodeNum, BoardHash> board_index;	//盤面に対するノードの添え字を保存する

		unordered_map<NodeNum, bool> used;
		NodeNum usenode[MAXPLAY + 2] = {};

		NodeNum nodenum;	//木全体のノード数
		int total_play;	//プレイアウト回数の総和
		NodeNum playnodenum;	//ゲーム進行上の現在のノード
		int playnum;	//ゲームのターン数
		Board playboard;
		//vector<int> red_rate; //赤駒率
		BitBoard red_pos;	//仮定の赤駒（場所）
		BitBoard blue_pos;	//red_pos以外の駒

		clock_t start_time;

		random_device rnd;
		mt19937 mt;
		uniform_int_distribution<> rand[65];

		//AB
		BitBoard resn, resp;
		long long nodesize;
		int maxdepth;


		//UCT内のプレイアウトをする関数
		//double playout(bool nowPlayer, int playoutnum, Board nowboard);
		int playout(bool nowPlayer, int playoutnum, Board nowboard);

		//木の"親ノード","子ノード"を入れると、遷移時のMoveCommandを返す
		MoveCommand toMoveCommand(NodeNum from, NodeNum to);

		void search_tree(int& turnnum, int& search_node, bool& nowPlayer);
		int expansion(int search_node, bool nowPlayer);


		MoveCommand toMoveCommand_AB(BitBoard ppos, BitBoard npos);
		int alpha_beta(Board& search_board, bool nowPlayer, int depth, int alpha, int beta);
		int evaluate(Board& board, int depth);
		void changeRed_pos_AB(BitBoard npos);

	public:
		//初期化
		UCTandAlphaBeta(Recieve str);

		// 実際の盤面のノードに移動
		void SetNode(Recieve str);

		//UCT探索をする
		void Search();

		//rootノードからどの手を選んで指すか。ノードの添え字を返す
		NodeNum Choice();

		//ノードを遷移して、サーバーに送る文字列を返す（送ってはない）
		Send MoveNode(NodeNum move_nodenum);

		//red_posを自分の行動に合わせて更新する
		void changeRed_pos(NodeNum to);

		void PrintStatus();

		//MinMax探索をする
		Send Search_AB();


	}; //class UCT

}