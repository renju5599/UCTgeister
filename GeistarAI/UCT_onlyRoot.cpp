//////////////////////
//todo
//
// 方策勾配法の形 127bit
// 青36bit
// 6bit	倒された数が1の{敵青,敵赤,自青,自赤},誰かが駒を取った,誰かが脱出した
// 赤36bit
// 6bit	倒された数が2の{敵青,敵赤,自青,自赤},空白,空白
// 敵36bit
// 6bit	倒された数が3の{敵青,敵赤,自青,自赤},空白,空白
// +1bit(=1)
// 
//////////////////////

#include<string>
#include<algorithm>
#include<time.h>
#include<cassert>
#include<iomanip>

#include "types.h"
#include "method.h"
#include "UCT_onlyRoot.h"
#include "NN_method.h"
#include "weight.h"
#include "Game.h"
#include "ColorGuess.h"

using namespace std;

using namespace UCT_RootRF;



//初期化
UCT::UCT(Recieve str)
{
	// ルートノードをつくる
	Nodes.resize(2);	//mapの関係で0番目は空にする
	Nodes[1].board = toBoard(str);
	//Nodes[1].value = { 0,0, MAX_VALUE };
	Nodes[1].value = { 0, 0, 0 };
	Nodes[1].nextnodes.clear();

	nodenum = 2;
	total_play = 0;
	playnodenum = 1;
	playnum = 0;
	//pieces = toPieces(str);

	mt.seed(rnd());
	for (int i = 1; i <= 64; i++)
	{
		uniform_int_distribution<>::param_type param(0, i-1);
		rand[i].param(param);
	}
}

// 実際の盤面のノードに移動
void UCT::SetNode(Recieve str)
{
	playnum++;	//ターン数を一つ進める
	
	// 探索木を構築しなおす
	Nodes.resize(2);
	Nodes[1].board = toBoard(str);
	//方策勾配法のモデルを通した値を保存
	BitsStatus bsta, bsta_null;
	toBitsStatus(bsta, Nodes[1].board);
	Nodes[1].value = { 0, 0, getNNres(bsta.TwoPiece, bsta_null.TwoPiece, 0) };
	Nodes[1].nextnodes.clear();

	nodenum = 2;
	total_play = 0;
	playnodenum = 1;
	//pieces = toPieces(str);

	board_index.clear();

	//色決め
	int bluenum = (4 - Nodes[1].board.dead_enblue) - __popcnt64(Nodes[1].board.enblue);	//わかっていない青の数
	int rednum = (4 - Nodes[1].board.dead_enred) - __popcnt64(Nodes[1].board.enred);	//わかっていない赤の数
	int ennum = __popcnt64(Nodes[1].board.en_mix);	//生きている敵の数
	int sum = 0;
	for (int i = 8; i < 16; i++)
	{
		sum += Red::eval[i];
	}
	bool colored[16] = {};
	while (rednum--)
	{
		int r = rnd() % sum;
		int border = 0;
		for (int i = 8; i < 16; i++)
		{
			if (colored[i])
				continue;
			border += Red::eval[i];
			if (border > r)
			{
				if (Nodes[1].board.pieces[i] == 63)
				{
					Nodes[1].board.dead_enred++;
				}
				else
				{
					Nodes[1].board.enred |= toBit(Nodes[1].board.pieces[i]);
					assert(onPiece(Nodes[1].board.en_mix, Nodes[1].board.pieces[i]));
				}
				sum -= Red::eval[i];
				colored[i] = 1;
				break;
			}
		}
	}
	Nodes[1].board.dead_enblue = (8 - ennum) - Nodes[1].board.dead_enred;
	Nodes[1].board.enblue = Nodes[1].board.en_mix & ~Nodes[1].board.enred;

	//脱出しなかったやつは決め打ちで赤
	//int red_num = Nodes[1].board.dead_enred;
	//for (int i = 8; i < 16; i++)
	//{
	//	if (Red::eval[i] == RED_CNTSTOP && red_num < 4)
	//	{
	//		red_num++;
	//		if (Nodes[1].board.pieces[i] == 63)
	//		{
	//			Nodes[1].board.dead_enred++;
	//			assert(false);
	//		}
	//		else
	//		{
	//			Nodes[1].board.enred |= toBit(Nodes[1].board.pieces[i]);
	//			assert(onPiece(Nodes[1].board.en_mix, Nodes[1].board.pieces[i]));
	//		}
	//	}
	//}
	//if (Red::decided)
	//{
	//	Nodes[1].board.en ^= Nodes[1].board.enred;
	//	assert(Nodes[1].board.en == (Nodes[1].board.en_mix ^ Nodes[1].board.enred ^ Nodes[1].board.enblue));
	//	Nodes[1].board.enblue |= Nodes[1].board.en;
	//	assert(Nodes[1].board.en_mix == (Nodes[1].board.enblue ^ Nodes[1].board.enred));
	//}

	//赤か青かの出力
	for (int i = 0; i < 64; i++)
	{
		if (Game_::piecenum[i] == -1)
			cout << '.' << "\t";
		else if (Game_::piecenum[i] < 8)
			cout << "my" << "\t";
		else
			cout << ((Nodes[1].board.enred & toBit(Nodes[1].board.pieces[Game_::piecenum[i]])) ? 'r' : '?') << "\t";
		if (i % 8 == 7) cout << endl;
	}
	//自分の赤がバレた
	for (int i = 0; i < 8; i++)
	{
		if (onPiece(Nodes[1].board.myred, Nodes[1].board.pieces[i]) && Red::eval[i] == RED_CNTSTOP)
		{
			Nodes[1].board.myred_eval = true;
			break;
		}
	}
}


int UCT::NextBoard_byNN(Board nextboards[32], bool nowPlayer, int nextboards_size, BitsStatus& bsta_pre, double& res_pre)
{
	BitsStatus bsta;
	double res[32] = {};
	double value_e[32] = {}, sum_e = 0;
	for (int i = 0; i < nextboards_size; i++)
	{
		toBitsStatus(bsta, nextboards[i]);
		res[i] = getNNres(bsta.TwoPiece, bsta_pre.TwoPiece, res_pre);
		if (nowPlayer == 1) res[i] = -res[i];
		value_e[i] = exp(res[i]);
		sum_e += value_e[i];
	}
	double softmax[33] = {};
	for (int i = 0; i < nextboards_size; i++)
	{
		softmax[i + 1] = softmax[i] + (value_e[i] / sum_e);
	}
	uniform_real_distribution<> rand_d(0.0, 1.0);
	int nx = 0;
	double r = rand_d(mt);
	nx = lower_bound(softmax + 1, softmax + nextboards_size + 1, r) - softmax - 1;
	toBitsStatus(bsta_pre, nextboards[nx]);
	res_pre = res[nx];
	return nx;
}

//UCT内のプレイアウトをする関数
double UCT::playout(bool nowPlayer, int turnnum, Board nowboard)
{
	int bluenum = (4 - nowboard.dead_enblue) - __popcnt64(nowboard.enblue);	//わかっていない青の数
	int rednum = (4 - nowboard.dead_enred) - __popcnt64(nowboard.enred);	//わかっていない赤の数
	int undefinenum = bluenum + rednum;
	int ennum = __popcnt64(nowboard.en_mix);	//生きている敵の数
	
	//BitsStatus bsta_pre;
	//double res_pre;

	////倒されている敵駒を色決めする
	//while (ennum < undefinenum)
	//{
	//	int r = rand[undefinenum](mt);
	//	if (r < bluenum)
	//	{
	//		nowboard.dead_enblue++;
	//		bluenum--;
	//	}
	//	else
	//	{
	//		nowboard.dead_enred++;
	//		rednum--;
	//	}
	//	undefinenum--;
	//}

	////盤面上の敵駒を色決めする
	//while (bluenum--)
	//{
	//	int r = rand[undefinenum](mt);
	//	BitBoard bb = nowboard.en_mix & ~nowboard.enblue & ~nowboard.enred;
	//	while (r--)
	//	{
	//		offBottomBB(bb);
	//	}
	//	bb = getBottomBB(bb);
	//	nowboard.enblue |= bb;
	//	undefinenum--;
	//}
	//assert(undefinenum == rednum);
	//nowboard.enred = nowboard.en_mix & ~nowboard.enblue;

	//赤っぽさに従って色決め
	//int sum = 0;
	//for (int i = 8; i < 16; i++)
	//{
	//	sum += Red::eval[i];
	//}
	//bool colored[16] = {};
	//while (rednum--)
	//{
	//	int r = rnd() % sum;
	//	int border = 0;
	//	for (int i = 8; i < 16; i++)
	//	{
	//		if (colored[i])
	//			continue;
	//		border += Red::eval[i];
	//		if (border > r)
	//		{
	//			if (nowboard.pieces[i] == 63)
	//			{
	//				nowboard.dead_enred++;
	//			}
	//			else
	//			{
	//				nowboard.enred |= toBit(nowboard.pieces[i]);
	//			}
	//			sum -= Red::eval[i];
	//			colored[i] = 1;
	//			break;
	//		}
	//	}
	//	undefinenum--;
	//}
	//nowboard.dead_enblue = (8 - ennum) - nowboard.dead_enred;
	//nowboard.enblue = nowboard.en_mix & ~nowboard.enred;

	//確率1/2で色決め（数が4:4とは限らない）
	//if (!Red::decided)
	//{
	//	for (int i = 8; i < 16; i++)
	//	{
	//		if (Red::eval[i] == 0) continue;	//試合上で既にいない駒
	//		if (rnd() % 2 == 0)
	//		{
	//			if (nowboard.pieces[i] == 63)
	//			{
	//				nowboard.dead_enred++;
	//			}
	//			else
	//			{
	//				nowboard.enred |= toBit(nowboard.pieces[i]);
	//			}
	//		}
	//		else
	//		{
	//			if (nowboard.pieces[i] == 63)
	//			{
	//				nowboard.dead_enblue++;
	//			}
	//			else
	//			{
	//				nowboard.enblue |= toBit(nowboard.pieces[i]);
	//			}
	//		}
	//	}
	//}
	//assert(nowboard.en_mix == (nowboard.enblue ^ nowboard.enred));
	//assert(nowboard.dead_enblue + nowboard.dead_enred + __popcnt64(nowboard.enred) + __popcnt64(nowboard.enblue) == 8);
	//chmin(nowboard.dead_enred, (Dead)4);
	//chmin(nowboard.dead_enblue, (Dead)4);

	while (true)
	{
		//勝負がついているか見る
		if (turnnum > MAXPLAY)
			return DRAW_VALUE;
		if (__popcnt64(nowboard.enred) == 0 || nowboard.dead_enred >= 4)
			return LOSE_VALUE;
		if (nowboard.dead_myblue == 4)
			return LOSE_VALUE;
		if (nowboard.escape)
			return (nowPlayer == 0 ? LOSE_VALUE : WIN_VALUE);
		if (nowboard.dead_myred == 4)
			return (nowboard.myred_eval ? LOSE_VALUE : WIN_VALUE * 0.5);
		if (__popcnt64(nowboard.enblue) == 0)
			return WIN_VALUE;
		assert(__popcnt64(nowboard.en_mix) >= 2);
		assert(nowboard.en_mix == (nowboard.enblue ^ nowboard.enred));


		/////	方策勾配法を用いる

		//Board nextboards[32];
		//int nextboards_size = 0;
		//PossibleNextBoard(nextboards, nowPlayer, nowboard, nextboards_size);
		//int nx = NextBoard_byNN(nextboards, nowPlayer, nextboards_size, bsta_pre, res_pre);
		//nowboard = nextboards[nx];

		////

		//ここが遅そう
		//vector<Board> nextboards;
		//PossibleNextBoard(nextboards, nowPlayer, nowboard);
		//int nextmoves_size = nextboards.size();
		//assert(nextmoves_size > 0);
		//int r = rand() % nextmoves_size;
		//nowboard = nextboards[r];	//ランダムに次の手を決める

		//変えてみる
		BitBoard ppos = 0, npos = 0;
		//ゴールできたらゴールする
		if (nowPlayer == 0 && Goal(getNextPosBB(nowboard.myblue), 0))
		{
			ppos = getNextPosBB(MYGOAL) & nowboard.myblue;
			//assert(__popcnt64(ppos) == 1);	//探索木の構造上、両方の手前にあることがある
			ppos = getBottomBB(ppos);
			npos = getNextPosBB(ppos) & MYGOAL;
			assert(__popcnt64(npos) == 1);

		}
		else if (nowPlayer == 1 && Goal(getNextPosBB(nowboard.enblue), 1))
		{
			ppos = getNextPosBB(ENGOAL) & nowboard.enblue;
			//assert(__popcnt64(ppos) == 1);
			ppos = getBottomBB(ppos);
			npos = getNextPosBB(ppos) & ENGOAL;
			assert(__popcnt64(npos) == 1);
		}
		//通常の動き
		else
		{
			ppos = (nowPlayer == 0 ? nowboard.my_mix : nowboard.en_mix);
			npos = Inside(getNextPosBB(ppos) & ~ppos);	//移動先候補を抽出
			assert(ppos != 0);
			assert(npos != 0);
			//移動先をランダムに選択
			int r = rand[__popcnt64(npos)](mt);
			while (r--)
				offBottomBB(npos);
			npos = getBottomBB(npos);
			//選択された移動先に行ける駒の中でランダムに選択
			ppos &= getNextPosBB(npos);
			assert(ppos != 0);
			r = rand[__popcnt64(ppos)](mt);
			while (r--)
				offBottomBB(ppos);
			ppos = getBottomBB(ppos);
		}
		//決めた行動をboardに変換する
		toNextBoard(nowboard, ppos, npos, nowPlayer);
		
		
		//ターンを進める
		nowPlayer = !nowPlayer;
		turnnum++;
	} // while(true)
}

void UCT::search_tree(int& turnnum, int& search_node, bool& nowPlayer)
{
	while (turnnum < MAXPLAY && !Nodes[search_node].nextnodes.empty())	//木の端まで見る
	{
		// 勝負がついていたら終わり
		if (
			__popcnt64(Nodes[search_node].board.en_mix) < 2 ||
			(Red::decided && (__popcnt64(Nodes[search_node].board.enblue) == 0 || __popcnt64(Nodes[search_node].board.enred) == 0)) ||
			Nodes[search_node].board.dead_enred == 4 ||
			Nodes[search_node].board.dead_myblue == 4 ||
			Nodes[search_node].board.dead_myred == 4 ||
			Nodes[search_node].board.escape
			)
		{
			break;
		}

		NodeNum choice_nodenum = -1;
		double maxvalue = -MAX_VALUE, minvalue = MAX_VALUE;
		double softmax_sum = 0;
		for (NodeNum nextnode : Nodes[search_node].nextnodes)
		{
			//softmax_sum += exp((nowPlayer == 0 ? 1 : -1) * Nodes[nextnode].value.NN_score);
			if (nowPlayer == 0) softmax_sum += exp(Nodes[nextnode].value.NN_score);
		}
		for (NodeNum nextnode : Nodes[search_node].nextnodes)
		{
			if (used[nextnode])	//2度同じ盤面は見ない
				continue;
			// 自分の手番⇒大きいのを選ぶ  相手の手番⇒小さいのを選ぶ
			if (Nodes[nextnode].value.play == 0)
			{
				choice_nodenum = nextnode;
				break;
			}
			if ((nowPlayer == 0 ?
				//chmax(maxvalue, compare_RF(Nodes[nextnode].value.win / 2.0, Nodes[nextnode].value.play, Nodes[search_node].value.play, exp(Nodes[nextnode].value.NN_score) / softmax_sum * 0.10)) :
				//chmax(maxvalue, compare_RF(Nodes[nextnode].value.play - Nodes[nextnode].value.win / 2.0, Nodes[nextnode].value.play, Nodes[search_node].value.play, exp(-Nodes[nextnode].value.NN_score) / softmax_sum))))
				chmax(maxvalue, compare(Nodes[nextnode].value.win / 2.0, Nodes[nextnode].value.play, total_play)) :
				chmax(maxvalue, compare(Nodes[nextnode].value.play - Nodes[nextnode].value.win / 2.0, Nodes[nextnode].value.play, total_play))))
			{
				choice_nodenum = nextnode;
			}
		}
		if (choice_nodenum == -1)	//見たことある盤面しかなければプレイアウト
		{
			break;
		}

		turnnum++;
		used[choice_nodenum] = 1;
		search_node = choice_nodenum;
		nowPlayer = !nowPlayer;
		usenode[turnnum] = search_node;
	}//while(!empty())
}

void UCT::search_RF(int& turnnum, int& search_node, bool& nowPlayer)
{
	double maxvalue = -MAX_VALUE, minvalue = MAX_VALUE;
	double softmax_sum = 0;
	for (NodeNum nextnode : Nodes[search_node].nextnodes)
	{
		softmax_sum += exp((nowPlayer == 0 ? 1 : -1) * Nodes[nextnode].value.NN_score);
	}
	for (NodeNum nextnode : Nodes[search_node].nextnodes)
	{
		if (used[nextnode])	//2度同じ盤面は見ない
			continue;
		// 自分の手番⇒大きいのを選ぶ  相手の手番⇒小さいのを選ぶ
		// NN : 10%
		//if ((nowPlayer == 0 ? chmax(maxvalue, Nodes[nextnode].value.comp) : chmin(minvalue, Nodes[nextnode].value.comp)))
		if ((nowPlayer == 0 ?
			chmax(maxvalue, compare_RF(Nodes[nextnode].value.win / 2.0, Nodes[nextnode].value.play, Nodes[search_node].value.play, exp(Nodes[nextnode].value.NN_score) / softmax_sum * 0.10)) :
			chmax(maxvalue, compare_RF(-Nodes[nextnode].value.win / 2.0, Nodes[nextnode].value.play, Nodes[search_node].value.play, exp(-Nodes[nextnode].value.NN_score) / softmax_sum * 0.10))))
			//chmax(maxvalue, compare(-Nodes[nextnode].value.win, Nodes[nextnode].value.play, 1))))
		{
			search_node = nextnode;
		}
	}

	turnnum++;
	used[search_node] = 1;
	nowPlayer = !nowPlayer;
	usenode[turnnum] = search_node;
}

int UCT::expansion(int search_node, bool nowPlayer)
{
	// 打てる手の候補を抽出する
	Board nextboards[32];
	int nextboards_size = 0;
	PossibleNextBoard(nextboards, nowPlayer, Nodes[search_node].board, nextboards_size);
	assert(nextboards_size > 0);

	//Nodesの拡張
	for (int i = 0; i < nextboards_size; i++)
	{
		Board nextboard = nextboards[i];
		if (board_index[nextboard] != 0)	//ハッシュが設定済み（探索済みのノード）は繋げるだけ
		{
			Nodes[search_node].nextnodes.push_back(board_index[nextboard]);
			continue;
		}
		board_index[nextboard] = nodenum;	//新たにハッシュを設定する

		Node nextnode;
		nextnode.board = nextboard;
		nextnode.nextnodes.clear();


		Nodes.emplace_back(nextnode);
		Nodes[search_node].nextnodes.push_back(nodenum);
		assert(Nodes[nodenum] == nextnode);
		nodenum++;
	}	//for nextmoves_size

	//次の手
	return Nodes[search_node].nextnodes[0];
}

int UCT::expansion_RF(int search_node, bool nowPlayer)
{
	// 打てる手の候補を抽出する
	Board nextboards[32];
	int nextboards_size = 0;
	PossibleNextBoard(nextboards, nowPlayer, Nodes[search_node].board, nextboards_size);
	assert(nextboards_size > 0);

	//Nodesの拡張
	//for (const Board& nextboard : nextboards)
	BitsStatus bsta, bsta_pre;
	double res_pre = Nodes[search_node].value.NN_score;
	toBitsStatus(bsta_pre, Nodes[search_node].board);
	int nx = 0;
	double res_max = -MAX_VALUE;
	for (int i = 0; i < nextboards_size; i++)
	{
		Board nextboard = nextboards[i];
		if (board_index[nextboard] != 0)	//ハッシュが設定済み（探索済みのノード）は繋げるだけ
		{
			Nodes[search_node].nextnodes.push_back(board_index[nextboard]);
			continue;
		}
		board_index[nextboard] = nodenum;	//新たにハッシュを設定する

		Node nextnode;
		nextnode.board = nextboard;
		nextnode.nextnodes.clear();

		//方策勾配法のモデルを通した値を保存
		toBitsStatus(bsta, nextboard);
		nextnode.value.NN_score = getNNres(bsta.TwoPiece, bsta_pre.TwoPiece, res_pre);
		//最も値が良い手にする
		if (chmax(res_max, (nowPlayer == 0 ? nextnode.value.NN_score : -nextnode.value.NN_score)))
		{
			nx = i;
		}

		Nodes.emplace_back(nextnode);
		Nodes[search_node].nextnodes.push_back(nodenum);
		assert(Nodes[nodenum] == nextnode);
		nodenum++;
	}	//for nextmoves_size

	//次の手
	return Nodes[search_node].nextnodes[nx];
}

//UCT探索をする
void UCT::Search()
{
	//事前にSetNode(str)をしておく
	NodeNum search_node = playnodenum;
	bool nowPlayer = 0; // 0:自分  1:相手
	int turnnum = playnum;
	fill(usenode, usenode + 302, 0);
	usenode[playnum] = playnodenum;


	//ルートの展開
	expansion_RF(search_node, nowPlayer);
	
	clock_t start_time = clock();
	while (clock() - start_time < (THINKING_TIME - 1) * CLOCKS_PER_SEC)	// UCTのループ
	{
		//木の端まで見る
		search_RF(turnnum, search_node, nowPlayer);
		search_tree(turnnum, search_node, nowPlayer);

		//展開 
		if (
			__popcnt64(Nodes[search_node].board.en_mix) < 2 ||
			(Red::decided && (__popcnt64(Nodes[search_node].board.enblue) == 0 || __popcnt64(Nodes[search_node].board.enred) == 0)) ||
			Nodes[search_node].board.dead_enred == 4 ||
			Nodes[search_node].board.dead_myblue == 4 ||
			Nodes[search_node].board.dead_myred == 4 ||
			Nodes[search_node].board.escape
			);
		else
		{
			if (total_play == 0 || Nodes[search_node].value.play >= 1)
			{
				search_node = expansion(search_node, nowPlayer);
				//次の手
				turnnum++;
				//assert(!Nodes[search_node].nextnodes.empty());
				nowPlayer = !nowPlayer;
				usenode[turnnum] = search_node;
			}
		}

		//プレイアウトとバックプロパゲーション
		{
			//playout();
			double reward = playout(nowPlayer, turnnum, Nodes[search_node].board);

			//backpropagation();
			total_play++;
			while (turnnum >= playnum)
			{
				Nodes[usenode[turnnum]].value.play++;
				Nodes[usenode[turnnum]].value.win += reward;
				turnnum--;
				//UpdateBoardValue(Nodes[node].value);
			}
			//探索情報をリセット
			//usenode.resize(1, playnodenum);
			search_node = playnodenum;
			nowPlayer = 0;
			turnnum = playnum;
			used.clear();
		}

		
	}	// while(true)	UCTのループ

}	// Search()

//木の"親ノード","子ノード"を入れると、遷移時のMoveCommandを返す
MoveCommand UCT::toMoveCommand(NodeNum from, NodeNum to)
{
	cout << "from:" << from << " to:" << to << endl;
	assert(from != to);
	MoveCommand move;
	BitBoard from_bb = Nodes[from].board.my_mix;
	BitBoard to_bb = Nodes[to].board.my_mix;
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
	NodeNum choice_nodenum = -1;
	double maxvalue = -MAX_VALUE;
	double softmax = 0;
	for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
	{
		softmax += exp(Nodes[nextnode].value.NN_score);
	}
	for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
	{
		// 自分の手番⇒大きいのを選ぶ
		// NN : 1%
		if (Nodes[nextnode].value.play == 0) continue;
		//if (chmax(maxvalue, (double)Nodes[nextnode].value.win / Nodes[nextnode].value.play))
		if (chmax(maxvalue, (double)Nodes[nextnode].value.win / 2.0 / Nodes[nextnode].value.play + exp(Nodes[nextnode].value.NN_score) / softmax * 0.10))
		{
			choice_nodenum = nextnode;
		}
	}
	assert(choice_nodenum != -1);
	assert(playnodenum != choice_nodenum);
	cout << "All {" << endl;
	for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
	{
		cout << "\t";
		cout << "Value:" << Nodes[nextnode].value.win << " Play:" << Nodes[nextnode].value.play << " NN_score:" << exp(Nodes[nextnode].value.NN_score) / softmax << endl;
	}
	cout << "}\nChoose" << endl;
	cout << "Value:" << Nodes[choice_nodenum].value.win << " Play:" << Nodes[choice_nodenum].value.play;
	cout << " NN_score:" << exp(Nodes[choice_nodenum].value.NN_score) / softmax << endl;
	cout << "maxvalue:" << maxvalue << endl;
	return choice_nodenum;
}

//ノードを遷移して、サーバーに送る文字列を返す（送ってはない）
Send UCT::MoveNode(NodeNum move_nodenum)
{
	assert(playnodenum != move_nodenum);
	MoveCommand move = toMoveCommand(playnodenum, move_nodenum);
	//Send send = toSend_piece(move, Game_::pieces);
	Send send = toSend(move, Game_::piecenum);
	//pieces.pos[getPieceNum(pieces, move.xy)] = nextpos(move);
	//cout << (int)move.xy << " " << (int)Game_::piecenum[move.xy] << endl;
	Game_::pieces[Game_::piecenum[move.xy]] = nextpos(move);
	Game_::piecenum[nextpos(move)] = Game_::piecenum[move.xy];
	Game_::piecenum[move.xy] = -1;
	playnodenum = move_nodenum;
	return send;
}

void UCT::PrintStatus()
{
	printf("Turn:%d\n", playnum);
	printf("Total Playout Times : %d\n", total_play);
	printf("Node Num : %d\n", nodenum);
}
