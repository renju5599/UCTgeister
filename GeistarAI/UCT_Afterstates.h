#pragma once

#include <vector>
#include <unordered_map>
#include <random>

#include "types.h"
#include "rule.h"

using namespace std;

namespace UCT_Afterstates
{

	typedef int NodeNum;

	struct NodeValue
	{
		double win = 0;
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
		bool player = 0;

		bool operator==(const Node& right) const
		{
			return (board == right.board && value == right.value && nextnodes == right.nextnodes && player == right.player);
		}
	};

	class UCT	//ゲーム終了まで1つの探索木で完結させるつもり
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
		//Pieces pieces;	//駒の番号と位置を関連付ける
		Board playboard;

		random_device rnd;
		mt19937 mt;
		uniform_int_distribution<> rand[65];

		//UCTの各ノードの価値(?)を計算・保存する
		//void UpdateBoardValue(NodeValue& v);

		//UCT内のプレイアウトをする関数
		double playout(bool nowPlayer, int playoutnum, Board nowboard);

		//木の"親ノード","子ノード"を入れると、遷移時のMoveCommandを返す
		MoveCommand toMoveCommand(NodeNum from, NodeNum to);

		void search_tree(int& turnnum, int& search_node, bool& nowPlayer);
		int expansion(int search_node, bool nowPlayer);
		int expansion_afterstates(int search_node, bool nowPlayer);

	public:
		//初期化
		UCT(Recieve str);

		// 実際の盤面のノードに移動
		void SetNode(Recieve str);

		//UCT探索をする
		void Search();

		//rootノードからどの手を選んで指すか。ノードの添え字を返す
		NodeNum Choice();

		//ノードを遷移して、サーバーに送る文字列を返す（送ってはない）
		Send MoveNode(NodeNum move_nodenum);

		void PrintStatus();

	}; //class MCT

}