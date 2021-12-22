#pragma once

#include <vector>

#include "types.h"

#define MAXPLAY 300	//最大手数 (ここで引き分け)
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
	vector<NodeNum> nextnodes;	//子ノードを保存 (Nodesの添え字を管理するだけ)
	bool finish = 0;

	bool operator==(const Node& right) const
	{
		return (board == right.board && value == right.value && nextnodes == right.nextnodes);
	}
};

class UCT	//ゲーム終了まで1つの探索木で完結させるつもり
{
private:
	//探索木
	vector<Node> Nodes;	//盤面を保存するノード達 
	//vector<vector<NodeNum>> NextNodes;	//Nodesにまとめればよいのではと思ったので消す
	//
	NodeNum nodenum;	//木全体のノード数
	int total_play;	//プレイアウト回数の総和
	NodeNum playnodenum;	//ゲーム進行上の現在のノード
	int playnum;	//ゲームのターン数
	Pieces pieces;	//駒の番号と位置を関連付ける

	//UCTで要らなくなったノードを（入れると）消す
	void NodeErase(NodeNum root);

	//UCTの各ノードの価値(?)を計算・保存する
	void UpdateBoardValue(NodeValue& v);

	//UCT内のプレイアウトをする関数
	int playout(bool nowPlayer, int playoutnum, Board nowboard);

	//木の"親ノード","子ノード"を入れると、遷移時のMoveCommandを返す
	MoveCommand toMoveCommand(NodeNum from, NodeNum to);

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

