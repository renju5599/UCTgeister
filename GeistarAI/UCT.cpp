//////////////////////
//todo
//
// コードが乱雑
// 動くか知らない
// 
// 
// 動きが遅い！！！
// ⇒ビットボード利用が雑！座標保存⇒全部ビットボードに変えても良さそう（入出力以外で不要）
// シフト演算の回数を減らしたい
// 
// (自・敵)駒の有無を8bitで表現
// 01234567
// 10010111		...5駒残ってることが簡単にわかる
// __popcnt()を使おう
// 
// 有効なMoveを32bitで表現	(vectorに入れてくのは遅い)
// 0    1    2    3...
// NEWS NEWS NEWS...
// 0111 0110 1111...		...プレイアウトに使える(ONの数で乱数取る⇒N番目のONの動きをする)
// 
// 出力が座標で済む...とか思ったけど違った。座標管理は出力で必要。
// 
//////////////////////

#include<vector>
#include<string>
#include<algorithm>
#include<time.h>
#include<cassert>

#include "types.h"
#include "method.h"
#include "UCT.h"

#define WIN_VALUE 1
#define LOSE_VALUE 0
#define DRAW_VALUE 0

using namespace std;


// すべての合法手後の盤面をvectorに入れる
// 敵駒を取った時の色は赤のときも青のときも入れる（確率とかは考慮していない）
void PossibleNextBoard(vector<Board>& nextpositions, bool nowPlayer, Board board)
{
	BitBoard nowmyblue = board.myblue;
	BitBoard nowmyred = board.myred;
	BitBoard nowmy = nowmyblue | nowmyred;
	BitBoard nowen = board.enemy;

	//getBottomBB()とoffBottomBB()を使って高速に調べるようにしたい
	if (nowPlayer == 0)
	{
		// どの位置の駒を動かすかのfor
		for (BitBoard piecebb = nowmy; piecebb != 0; piecebb = offBottomBB(piecebb))
		{
			BitBoard ppos = getBottomBB(piecebb);
			// 移動4方向を見るfor
			for (BitBoard nextbb = getNextPosBB(ppos); nextbb != 0; nextbb = offBottomBB(nextbb))
			{
				BitBoard npos = getBottomBB(nextbb);
				//合法かを見る
				if (Outside(npos) && !Goal(npos, nowPlayer))	//盤面外
					continue;
				if (Goal(npos, nowPlayer) && onPiece(nowmyred, ppos))	//赤駒が脱出しようとした
					continue;
				if (onPiece(nowmy, npos))	//自身の駒が重なる
					continue;

				if (Goal(npos, nowPlayer))	//脱出した
				{
					board.escape = true;
				}
				else	//脱出してない
				{
					board.escape = false;
				}
				if (onPiece(nowmyblue, ppos))	//青を動かした
				{
					board.myblue = change(nowmyblue, ppos, npos);	//駒の移動
					if (onPiece(nowen, npos))	//敵駒を取った
					{
						board.enemy ^= npos;	//敵駒を取った後の状態に
						if (board.enemy == 0)
						{
							cout << (int)board.dead_enblue << " " << (int)board.dead_enred << endl;
						}
						assert(board.enemy != 0);
						board.kill = true;
						//青駒を取った
						board.dead_enblue <<= 1;
						nextpositions.push_back(board);
						board.dead_enblue >>= 1;
						//赤駒を取った
						board.dead_enred <<= 1;
						nextpositions.push_back(board);
						board.dead_enred >>= 1;
						//元に戻す
						board.enemy ^= npos;
					}
					else	//敵駒を取らなかった
					{
						board.kill = false;
						nextpositions.push_back(board);
					}
					board.myblue = nowmyblue;	//動かす前に戻す
				}
				else	//赤を動かした
				{
					board.myred = change(nowmyred, ppos, npos);
					if (onPiece(nowen, npos))
					{
						board.enemy ^= npos;
						if (board.enemy == 0)
						{
							cout << (int)board.dead_enblue << " " << (int)board.dead_enred << endl;
						}
						assert(board.enemy != 0);
						board.kill = true;

						board.dead_enblue <<= 1;
						nextpositions.push_back(board);
						board.dead_enblue >>= 1;

						board.dead_enred <<= 1;
						nextpositions.push_back(board);
						board.dead_enred >>= 1;

						board.enemy ^= npos;
					}
					else
					{
						board.kill = false;
						nextpositions.push_back(board);
					}
					board.myred = nowmyred;

				} // 動かした色を見るif-else
			} // for nextbb
		} // for piecebb
	} // if nowPlayer == 0
	else
	{
		if (nowen == 0)
		{
			cout << board.dead_enblue << " " << board.dead_enred << endl;
		}
		assert(nowen != 0);
		for (BitBoard piecebb = nowen; piecebb != 0; piecebb = offBottomBB(piecebb))
		{
			BitBoard ppos = getBottomBB(piecebb);
			for (BitBoard nextbb = getNextPosBB(ppos); nextbb != 0; nextbb = offBottomBB(nextbb))
			{
				BitBoard npos = getBottomBB(nextbb);
				if (Outside(npos) && !Goal(npos, nowPlayer))
					continue;
				if (onPiece(nowen, npos))	//自身の駒が重なる
					continue;

				if (Goal(npos, nowPlayer))
				{
					board.escape = true;
				}
				else
				{
					board.escape = false;
				}

				board.enemy = change(nowen, ppos, npos);
				if (onPiece(nowmyblue, npos))	//青駒を取られた
				{
					board.myblue ^= npos;
					board.kill = true;

					board.dead_myblue <<= 1;
					nextpositions.push_back(board);
					board.dead_myblue >>= 1;

					board.myblue ^= npos;
				}
				else if(onPiece(nowmyred, npos))	//赤駒を取られた
				{
					board.myred ^= npos;
					board.kill = true;

					board.dead_myred <<= 1;
					nextpositions.push_back(board);
					board.dead_myred >>= 1;

					board.myred ^= npos;
				}
				else
				{
					board.kill = false;
					nextpositions.push_back(board);
				}
				board.enemy = nowen;

			}
		}

	} //nowPlayer == 1

	return;
}


//初期化
UCT::UCT(Recieve str)
{
	// ルートノードをつくる
	Nodes.resize(1);
	Nodes[0].board = toBoard(str);
	Nodes[0].value = { 0,0, MAX_VALUE };
	Nodes[0].nextnodes.resize(0);
	Nodes[0].finish = 0;
	nodenum = 1;
	total_play = 0;
	playnodenum = 0;
	playnum = 0;
	pieces = toPieces(str);
}

//UCTで要らなくなったノードを（入れると）消す
//意味がなさそうなので消す
//void UCT::NodeErase(NodeNum root)
//{
//	vector<int> v;
//	NodeNum nd = root;
//	while (!Nodes[root].nextnodes.empty())
//	{
//		while (!Nodes[nd].nextnodes.empty())
//		{
//			v.push_back(nd);
//			nd = Nodes[nd].nextnodes.back();
//		}
//		nd = v.back();
//		Nodes[nd].nextnodes.pop_back();
//		nodenum--;
//	}
//}

// 実際の盤面のノードに移動
void UCT::SetNode(Recieve str)
{
	playnum++;	//ターン数を一つ進める

	// 探索木にノードが無ければ構築しなおす
	if (Nodes[playnodenum].nextnodes.empty())
	{
		Nodes.resize(1);
		Nodes[0].board = toBoard(str);
		Nodes[0].value = { 0,0, MAX_VALUE };
		Nodes[0].nextnodes.resize(0);
		Nodes[0].finish = 0;
		nodenum = 1;
		total_play = 0;
		playnodenum = 0;
		pieces = toPieces(str);
	}
	else
	{
		pieces = toPieces(str);
		Board nowboard = toBoard(str);
		if (nowboard == Nodes[playnodenum].board)
			return;
		for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
		{
			if (Nodes[nextnode].board == nowboard)
			{
				//不要なノードを消す
				//for (NodeNum another : Nodes[playnodenum].nextnodes)
				//{
				//	if (another == nextnode) continue;

				//	NodeErase(another);
				//}

				total_play = Nodes[nextnode].value.play;
				playnodenum = nextnode;
				break;
			}
		}
		assert(Nodes[playnodenum].board == nowboard);
	}
}

//UCTの各ノードの価値(?)を計算・保存する
void UCT::UpdateBoardValue(NodeValue& v)
{
	if (v.play == 0)
	{
		v.comp = MAX_VALUE;
		return;
	}
	assert(total_play != 0);
	v.comp = ((double)v.win / v.play) + FACTOR * pow(log(total_play) / v.play, 0.5);
}

//UCT内のプレイアウトをする関数
int UCT::playout(bool nowPlayer, int playoutnum, Board nowboard)
{

	if (playoutnum > MAXPLAY)
		return DRAW_VALUE;
	if (nowboard.escape)
		return (nowPlayer == 0 ? LOSE_VALUE : WIN_VALUE);
	if (nowboard.dead_myblue == 16 || nowboard.dead_enred == 16)
		return LOSE_VALUE;
	if (nowboard.dead_myred == 16 || nowboard.dead_enblue == 16)
		return WIN_VALUE;

	while (true)
	{
		vector<Board> nextboards;
		PossibleNextBoard(nextboards, nowPlayer, nowboard);
		int nextmoves_size = nextboards.size();
		if (nextmoves_size <= 0)
		{
			cout << "size:" << nextmoves_size << endl;
			cout << "nowplayer:" << nowPlayer << endl;
			cout << "playoutnum:" << playoutnum << endl;
		}
		assert(nextmoves_size > 0);
		int r = rand() % nextmoves_size;
		nowboard = nextboards[r];

		if (playoutnum > MAXPLAY)
			return DRAW_VALUE;
		if (nowboard.escape)
			return (nowPlayer == 0 ? WIN_VALUE : LOSE_VALUE);
		if (nowboard.dead_myblue == 16 || nowboard.dead_enred == 16)
			return LOSE_VALUE;
		if (nowboard.dead_myred == 16 || nowboard.dead_enblue == 16)
			return WIN_VALUE;

		nowPlayer = !nowPlayer;
		playoutnum++;
	} // while(true)
}

//UCT探索をする
void UCT::Search()
{
	//事前にSetNode(str)をしておく
	NodeNum search_node = playnodenum;
	vector<NodeNum> usenode(1, playnodenum);
	bool nowPlayer = 0; // 0:自分  1:相手
	int playoutnum = playnum;

	clock_t start_time = clock();
	while (clock() - start_time < 1.0 * CLOCKS_PER_SEC)	// UCTのループ
	{

		playoutnum++;

		// 打てる手の候補を抽出する
		vector<Board> nextboards;
		PossibleNextBoard(nextboards, nowPlayer, Nodes[search_node].board);
		int nextboards_size = nextboards.size();
		assert(nextboards_size > 0);

		if (Nodes[search_node].nextnodes.empty())	//葉ノード
		{

			if (!Nodes[search_node].finish && Nodes[search_node].value.play >= 3)	//展開
			{
				//Nodesの拡張
				for (int i = 0; i < nextboards_size; i++)
				{
					Node nextnode;
					nextnode.board = nextboards[i];
					nextnode.nextnodes.resize(0);
					nextnode.finish = (
						nextnode.board.dead_enblue == 16 ||
						nextnode.board.dead_enred == 16 ||
						nextnode.board.dead_myblue == 16 ||
						nextnode.board.dead_myred == 16 ||
						nextnode.board.escape
						);

					Nodes.push_back(nextnode);
					Nodes[search_node].nextnodes.push_back(nodenum);
					assert(Nodes[nodenum] == nextnode);
					nodenum++;
				}	//for nextmoves_size


				assert(!Nodes[search_node].nextnodes.empty());
				search_node = Nodes[search_node].nextnodes[0];
				nowPlayer = !nowPlayer;
				usenode.push_back(search_node);
			}	// if 展開

			else	//プレイアウトとバックプロパゲーション
			{
				//playout();
				int reward = playout(nowPlayer, playoutnum, Nodes[search_node].board);	//勝ち:3  分け:1  負け:0　(自分が)

				//backpropagation();
				total_play++;
				for (NodeNum node : usenode)
				{
					Nodes[node].value.play++;
					Nodes[node].value.win += reward;
					UpdateBoardValue(Nodes[node].value);
				}
				//探索情報をリセット
				usenode.resize(1, playnodenum);
				search_node = playnodenum;
				nowPlayer = 0;
				playoutnum = playnum;
			}	// else
		} // if 葉ノード

		else	//木の探索
		{
			NodeNum choice_nodenum = Nodes[search_node].nextnodes[0];
			NodeValue maxvalue = Nodes[choice_nodenum].value;
			for (NodeNum nextnode : Nodes[search_node].nextnodes)
			{
				// 自分の手番⇒大きいのを選ぶ  相手の手番⇒小さいのを選ぶ
				if ((nowPlayer == 0 ? chmax(maxvalue, Nodes[nextnode].value) : chmin(maxvalue, Nodes[nextnode].value)))
				{
					choice_nodenum = nextnode;
				}
			}
			search_node = choice_nodenum;
			nowPlayer = !nowPlayer;
			usenode.push_back(search_node);
		}
	}	// while(true)	UCTのループ


}	// Search()

//木の"親ノード","子ノード"を入れると、遷移時のMoveCommandを返す
MoveCommand UCT::toMoveCommand(NodeNum from, NodeNum to)
{
	cout << "from:" << from << " to:" << to << endl;
	assert(from != to);
	MoveCommand move;
	BitBoard from_bb = Nodes[from].board.myblue | Nodes[from].board.myred;
	BitBoard to_bb = Nodes[to].board.myblue | Nodes[to].board.myred;
	assert(from_bb != to_bb);
	BitBoard ppos = from_bb & ~to_bb;
	BitBoard npos = ~from_bb & to_bb;
	assert(ppos != 0);
	assert(__popcnt64(ppos) == 1);
	assert(npos != 0);
	assert(__popcnt64(npos) == 1);
	Point ppoint = toPoint(ppos);
	Point npoint = toPoint(npos);
	move.xy = ppoint;
	move.dir = toDir(ppoint, npoint);
	return move;
}

//rootノードからどの手を選んで指すか。ノードの添え字を返す
NodeNum UCT::Choice()
{
	assert(playnodenum < Nodes.size());
	assert(!Nodes[playnodenum].nextnodes.empty());
	NodeNum choice_nodenum = Nodes[playnodenum].nextnodes[0];
	NodeValue maxvalue = Nodes[choice_nodenum].value;
	for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
	{
		// 自分の手番⇒大きいのを選ぶ
		if (chmax(maxvalue, Nodes[nextnode].value))
		{
			choice_nodenum = nextnode;
		}
	}
	assert(playnodenum != choice_nodenum);
	cout << Nodes[choice_nodenum].value.win << " " << Nodes[choice_nodenum].value.play << endl;
	return choice_nodenum;
}

//ノードを遷移して、サーバーに送る文字列を返す（送ってはない）
Send UCT::MoveNode(NodeNum move_nodenum)
{
	assert(playnodenum != move_nodenum);
	MoveCommand move = toMoveCommand(playnodenum, move_nodenum);
	Send send = toSend(move, pieces);
	pieces.pos[getPieceNum(pieces, move.xy)] = nextpos(move);
	playnodenum = move_nodenum;
	return send;
}

void UCT::PrintStatus()
{
	printf("Turn:%d\n", playnum);
	printf("Playout Times : %d\n", total_play);
	printf("Node Num : %d\n", nodenum);
}
