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

#include "types.h"
#include "method.h"
#include "UCT.h"
#include "ColorGuess.h"
#include "Game.h"

using namespace std;

using namespace UCT_simple;

//double UCT::compare(int win, int play)
//{
//	if (play == 0) return MAX_VALUE;
//	return ((double)win / play) + FACTOR * pow(log(total_play) / play, 0.5);
//}


//初期化
UCT::UCT(Recieve str)
{
	// ルートノードをつくる
	Nodes.resize(2);	//mapの関係で0番目は空にする
	Nodes[1].board = toBoard(str);
	Nodes[1].value = { 0, 0 };
	Nodes[1].nextnodes.clear();
	Nodes[1].finish = 0;

	nodenum = 2;
	total_play = 0;
	playnodenum = 1;
	playnum = 0;
	//pieces = toPieces(str);
	red_rate.resize(8, 0);

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
	Nodes[1].value = { 0, 0 };
	Nodes[1].nextnodes.clear();
	Nodes[1].finish = 0;

	nodenum = 2;
	total_play = 0;
	playnodenum = 1;
	//pieces = toPieces(str);

	board_index.clear();
}


//UCT内のプレイアウトをする関数
double UCT::playout(bool nowPlayer, int turnnum, Board nowboard)
{
	int bluenum = (4 - nowboard.dead_enblue) - __popcnt64(nowboard.enblue);	//わかっていない青の数
	int rednum = (4 - nowboard.dead_enred) - __popcnt64(nowboard.enred);	//わかっていない赤の数
	int undefinenum = bluenum + rednum;

	int ennum = __popcnt64(nowboard.en_mix);	//生きている敵の数
	

	//倒されている敵駒を色決めする
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

	//盤面上の敵駒を色決めする
	//while (bluenum--)
	//{
	//	int r = rand[undefinenum](mt);
	//	BitBoard bb = nowboard.enemy & ~nowboard.enblue & ~nowboard.enred;
	//	while (r--)
	//	{
	//		offBottomBB(bb);
	//	}
	//	bb = getBottomBB(bb);
	//	nowboard.enblue |= bb;
	//	undefinenum--;
	//}
	//assert(undefinenum == rednum);
	//nowboard.enred = nowboard.enemy & ~nowboard.enblue;
	
	// Game_::pieces[]をノードに合わせて更新する必要がある
	int sum = 0;
	for (int i = 8; i < 16; i++)
	{
		sum += Red::eval[i];
	}
	bool colored[16] = {};
	while (rednum--)
	{
		//BitBoard bb = nowboard.enemy & ~nowboard.enblue & ~nowboard.enred;
		int r = rnd() % sum;
		int border = 0;
		for (int i = 8; i < 16; i++)
		{
			if (colored[i])
				continue;
			border += Red::eval[i];
			if (border > r)
			{
				if (nowboard.pieces[i] == 63)
				{
					nowboard.dead_enred++;
				}
				else
				{
					nowboard.enred |= toBit(nowboard.pieces[i]);
					assert(onPiece(nowboard.en_mix, nowboard.pieces[i]));
				}
				sum -= Red::eval[i];
				colored[i] = 1;
				break;
			}
		}
		undefinenum--;
	}
	nowboard.dead_enblue = (8 - ennum) - nowboard.dead_enred;
	nowboard.enblue = nowboard.en_mix & ~nowboard.enred;
	assert(nowboard.en_mix == (nowboard.enblue ^ nowboard.enred));

	while (true)
	{
		//勝負がついているか見る
		if (turnnum > MAXPLAY)
			return DRAW_VALUE + (nowboard.dead_enblue - nowboard.dead_enred) * 0.5;
		if (nowboard.escape)
			return (nowPlayer == 0 ? LOSE_VALUE : WIN_VALUE) + (nowboard.dead_enblue - nowboard.dead_enred) * 0.5;
		if (nowboard.dead_myblue == 4)
			return LOSE_VALUE + (nowboard.dead_enblue - nowboard.dead_enred) * 0.5;
		if (nowboard.dead_enred == 4)
			return LOSE_VALUE + (nowboard.dead_enblue - nowboard.dead_enred) * 0.5;
		if (nowboard.dead_myred == 4)
			return WIN_VALUE + (nowboard.dead_enblue - nowboard.dead_enred) * 0.5;
		if (nowboard.dead_enblue == 4)
			return WIN_VALUE + (nowboard.dead_enblue - nowboard.dead_enred) * 0.5;
		assert(__popcnt64(nowboard.en_mix) >= 2);
		assert(nowboard.en_mix == (nowboard.enblue ^ nowboard.enred));


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
		NodeNum choice_nodenum = -1;
		double maxvalue = -MAX_VALUE, minvalue = MAX_VALUE;
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
				chmax(maxvalue, compare(Nodes[nextnode].value.win, Nodes[nextnode].value.play, total_play)) :
				chmax(maxvalue, compare(-Nodes[nextnode].value.win, Nodes[nextnode].value.play, total_play))))
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

int UCT::expansion(int search_node, bool nowPlayer)
{
	// 打てる手の候補を抽出する
	Board nextboards[32];
	int nextboards_size = 0;
	PossibleNextBoard(nextboards, nowPlayer, Nodes[search_node].board, nextboards_size);
	assert(nextboards_size > 0);

	//Nodesの拡張
	for(int i = 0; i < nextboards_size; i++)
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

		//勝負がついているか
		nextnode.finish = (
			__popcnt64(nextnode.board.en_mix) < 2 ||
			nextnode.board.dead_enblue == 4 ||
			nextnode.board.dead_enred == 4 ||
			nextnode.board.dead_myblue == 4 ||
			nextnode.board.dead_myred == 4 ||
			nextnode.board.escape
			);

		Nodes.emplace_back(nextnode);
		Nodes[search_node].nextnodes.push_back(nodenum);
		assert(Nodes[nodenum] == nextnode);
		nodenum++;
	}	//for nextmoves_size

	//次の手
	return Nodes[search_node].nextnodes[0];
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
	expansion(search_node, nowPlayer);
	
	clock_t start_time = clock();
	while (clock() - start_time < (THINKING_TIME - 1) * CLOCKS_PER_SEC)	// UCTのループ
	{
		//木の端まで見る
		search_tree(turnnum, search_node, nowPlayer);

		//展開
		if (total_play == 0 || (!Nodes[search_node].finish && Nodes[search_node].value.play >= 1))
		{
			search_node = expansion(search_node, nowPlayer);
			//次の手
			turnnum++;
			//assert(!Nodes[search_node].nextnodes.empty());
			nowPlayer = !nowPlayer;
			usenode[turnnum] = search_node;
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
	for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
	{
		// 自分の手番⇒大きいのを選ぶ
		if (Nodes[nextnode].value.play == 0) continue;
		if (chmax(maxvalue, (double)Nodes[nextnode].value.win / Nodes[nextnode].value.play))
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
		cout << "Value:" << Nodes[nextnode].value.win << " Play:" << Nodes[nextnode].value.play << endl;
	}
	cout << "}\nChoose" << endl;
	cout << "Value:" << Nodes[choice_nodenum].value.win << " Play:" << Nodes[choice_nodenum].value.play;
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
