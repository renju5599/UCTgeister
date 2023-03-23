#include<string>
#include<algorithm>
#include<time.h>
#include<cassert>

#include "types.h"
#include "method.h"
#include "NN_method.h"
#include "weight.h"
#include "UCTrootRFandABRed.h"
#include "ColorGuess.h"
#include "Game.h"

using namespace std;

using namespace UCTrootRFandABRed;


//初期化
UCTandAlphaBeta::UCTandAlphaBeta(Recieve str)
{
	mt.seed(rnd());
	for (int i = 1; i <= 64; i++)
	{
		uniform_int_distribution<>::param_type param(0, i - 1);
		rand[i].param(param);
	}

	// ルートノードをつくる
	Nodes.resize(2);	//mapの関係で0番目は空にする
	Nodes[1].board = toBoard(str);
	Nodes[1].value = { 0, 0, 0 };
	Nodes[1].nextnodes.clear();
	Nodes[1].finish = 0;

	nodenum = 2;
	total_play = 0;
	playnodenum = 1;
	playnum = 0;
	//red_rate.resize(8, 0);

	//赤駒を決める
	int r = rand[__popcnt64(Nodes[1].board.en)](mt);
	red_pos = Nodes[1].board.en;
	while (r--)
	{
		offBottomBB(red_pos);
	}
	red_pos = getBottomBB(red_pos);
	blue_pos = Nodes[1].board.en ^ red_pos;
	Nodes[1].board.enblue = blue_pos;
	Nodes[1].board.enred = red_pos;
	Nodes[1].board.en = 0;
	assert(Nodes[1].board.enblue ^ Nodes[1].board.enred == Nodes[1].board.en_mix);

	start_time = 0;

	resp = resn = 0;
	nodesize = 0;
	maxdepth = 0;
}

// 実際の盤面のノードに移動
void UCTandAlphaBeta::SetNode(Recieve str)
{
	playnum = Game_::turn;	//ターン数を進める

	// 探索木を構築しなおす
	Nodes.resize(2);
	Nodes[1].board = toBoard(str);
	BitsStatus bsta, bsta_null;
	toBitsStatus(bsta, Nodes[1].board);
	Nodes[1].value = { 0, 0, getNNres(bsta.TwoPiece, bsta_null.TwoPiece, 0) };
	Nodes[1].nextnodes.clear();
	Nodes[1].finish = 0;

	//赤駒を設定する
	if (onPiece(Nodes[1].board.en, red_pos))	//相手が青を動かした
		blue_pos = Nodes[1].board.en ^ red_pos;
	else	//相手が赤を動かした
		red_pos = Nodes[1].board.en ^ blue_pos;
	Nodes[1].board.enblue = blue_pos;
	Nodes[1].board.enred = red_pos;
	Nodes[1].board.en = 0;
	assert((Nodes[1].board.enblue ^ Nodes[1].board.enred) == Nodes[1].board.en_mix);
	assert(__popcnt64(Nodes[1].board.enred) == 1);
	assert((Nodes[1].board.enblue & Nodes[1].board.enred) == 0);
	cout << (Red::decided ? "decided" : "not decide") << endl;
	for (Point i = 0; i < 64; i++)
	{
		if (onPiece(Nodes[1].board.myblue, i))
			cout << "Mb  ";
		else if (onPiece(Nodes[1].board.myred, i))
			cout << "Mr  ";
		else if (onPiece(blue_pos, i))
			cout << "Eb  ";
		else if (onPiece(red_pos, i))
			cout << "Er* ";
		else
			cout << ".   ";
		if (i % 8 == 7)
			cout << endl;
	}

	nodenum = 2;
	total_play = 0;
	playnodenum = 1;

	board_index.clear();

	//AB
	playboard = Nodes[1].board;
}


//UCT内のプレイアウトをする関数
//double UCTandAlphaBeta::playout(bool nowPlayer, int turnnum, Board nowboard)
int UCTandAlphaBeta::playout(bool nowPlayer, int turnnum, Board nowboard)
{
	while (true)
	{
		if (clock() - start_time > (THINKING_TIME - 1) * CLOCKS_PER_SEC)
		{
			return DRAW_VALUE;
		}

		int def = 0;
		if (Game_::purple && nowboard.kill && nowPlayer == 1)
		{
			if (!(Red::decided || nowboard.dead_enred < 3))
			{
				def = -16384;
			}
		}
		//勝負がついているか見る
		/*
		if (turnnum > MAXPLAY)
			return DRAW_VALUE;
		if (nowboard.escape)
			return (nowPlayer == 0 ? LOSE_VALUE : WIN_VALUE);
		if (nowboard.dead_myblue == 4)
			return LOSE_VALUE;
		if (nowboard.enred == 0)
			return LOSE_VALUE;
		if (nowboard.dead_myred == 4)
			return WIN_VALUE;
		if (nowboard.enblue == 0)
			return WIN_VALUE;
		assert(__popcnt64(nowboard.en_mix) >= 2);
		assert(nowboard.en_mix == (nowboard.enblue ^ nowboard.enred));
		*/
		if (nowboard.enred == 0)
			return 0 + def;
		else if (nowboard.myblue == 0)
			return 2000 + def;
		else if (Goal(nowboard.enblue, 1))
			return 1000 + def;
		else if (nowboard.enblue == 0)
		{
			if (!Game_::purple || Red::decided || nowboard.dead_enred < 3)
				return 65500 + def;
			else
				return 42768 + def;
		}
		else if (nowboard.myred == 0)
			return 60000 + def;
		else if (Goal(nowboard.myblue, 0))
			return 65535 + def;
		else if(turnnum > MAXPLAY)
			return 32768 + def;
		

		//変えてみる
		BitBoard ppos = 0, npos = 0;
		//ゴールできたらゴールする
		if (nowPlayer == 0 && Goal(getNextPosBB(nowboard.myblue), 0))
		{
			ppos = getNextPosBB(MYGOAL) & nowboard.myblue;
			ppos = getBottomBB(ppos);
			npos = getNextPosBB(ppos) & MYGOAL;
			assert(__popcnt64(npos) == 1);

		}
		else if (nowPlayer == 1 && Goal(getNextPosBB(nowboard.enblue), 1))
		{
			ppos = getNextPosBB(ENGOAL) & nowboard.enblue;
			ppos = getBottomBB(ppos);
			npos = getNextPosBB(ppos) & ENGOAL;
			assert(__popcnt64(npos) == 1);
		}
		//通常の動き
		else
		{
			ppos = (nowPlayer == 0 ? nowboard.my_mix : nowboard.en_mix);
			//npos = Inside(getNextPosBB(ppos) & ~ppos);	//移動先候補を抽出
			npos = Inside(getNextPosBB(ppos) & ~ppos & ~nowboard.enred);	//移動先候補を抽出
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


void UCTandAlphaBeta::search_tree(int& turnnum, int& search_node, bool& nowPlayer)
{
	while (turnnum < MAXPLAY && !Nodes[search_node].nextnodes.empty() && !Nodes[search_node].finish)	//木の端まで見る
	{
		NodeNum choice_nodenum = -1;
		double maxvalue = -MAX_VALUE, minvalue = MAX_VALUE;
		for (NodeNum nextnode : Nodes[search_node].nextnodes)
		{
			if (used[nextnode])	//2度同じ盤面は見ない
				continue;
			// 自分の手番⇒大きいのを選ぶ  相手の手番⇒小さいのを選ぶ
			if ((nowPlayer == 0 ?
				chmax(maxvalue, compare(Nodes[nextnode].value.win, Nodes[nextnode].value.play, total_play)) :
				chmax(maxvalue, compare(Nodes[nextnode].value.play - Nodes[nextnode].value.win, Nodes[nextnode].value.play, total_play))))
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
void UCTandAlphaBeta::search_RF(int& turnnum, int& search_node, bool& nowPlayer)
{
	NodeNum choice_nodenum = -1;
	double maxvalue = -MAX_VALUE, minvalue = MAX_VALUE;
	double softmax_sum = 0;
	if (nowPlayer == 0)
	{
		for (NodeNum nextnode : Nodes[search_node].nextnodes)
		{
			softmax_sum += exp((nowPlayer == 0 ? 1 : -1) * Nodes[nextnode].value.NN_score);
			//if (nowPlayer == 0) softmax_sum += exp(Nodes[nextnode].value.NN_score);
		}
	}
	for (NodeNum nextnode : Nodes[search_node].nextnodes)
	{
		if (used[nextnode])	//2度同じ盤面は見ない
			continue;
		// 自分の手番⇒大きいのを選ぶ  相手の手番⇒小さいのを選ぶ
		// (相手の評価値はわからんので自分視点の符号逆)
		// PUCTは相手側はランダム(回数最小)っぽい
		if ((nowPlayer == 0 ?
			chmax(maxvalue, compare_RF(Nodes[nextnode].value.win, Nodes[nextnode].value.play, Nodes[search_node].value.play, exp(Nodes[nextnode].value.NN_score) / softmax_sum * 0.10)) :
			chmax(maxvalue, -(double)Nodes[nextnode].value.play)))
		{
			choice_nodenum = nextnode;
		}
	}
	if (choice_nodenum == -1)	//見たことある盤面しかなければプレイアウト
	{
		return;
	}

	turnnum++;
	used[choice_nodenum] = 1;
	search_node = choice_nodenum;
	nowPlayer = !nowPlayer;
	usenode[turnnum] = search_node;
}

int UCTandAlphaBeta::expansion(int search_node, bool nowPlayer, bool useRF)
{
	// 打てる手の候補を抽出する
	Board nextboards[32];
	int nextboards_size = 0;
	PossibleNextBoard(nextboards, nowPlayer, Nodes[search_node].board, nextboards_size);
	assert(nextboards_size > 0);

	//Nodesの拡張
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

		//勝負がついているか
		/*
		nextnode.finish = (
			__popcnt64(nextnode.board.en_mix) < 2 ||
			(Red::decided && (__popcnt64(Nodes[search_node].board.enblue) == 0 || __popcnt64(Nodes[search_node].board.enred) == 0)) ||
			nextnode.board.dead_enred == 4 ||
			nextnode.board.dead_myblue == 4 ||
			nextnode.board.dead_myred == 4 ||
			nextnode.board.escape
			);
		*/
		nextnode.finish = (
			__popcnt64(nextnode.board.en_mix) < 2 ||
			nextnode.board.enblue == 0 ||
			nextnode.board.enred == 0 ||
			nextnode.board.dead_myblue == 4 ||
			nextnode.board.dead_myred == 4 ||
			nextnode.board.escape
			);

		if (nowPlayer == 0 && useRF)
		{
			//方策勾配法のモデルを通した値を保存
			toBitsStatus(bsta, nextboard);
			nextnode.value.NN_score = getNNres(bsta.TwoPiece, bsta_pre.TwoPiece, res_pre);
			//最も値が良い手にする
			if (chmax(res_max, (nowPlayer == 0 ? nextnode.value.NN_score : -nextnode.value.NN_score)))
			{
				nx = i;
			}
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
void UCTandAlphaBeta::Search()
{
	//事前にSetNode(str)をしておく
	NodeNum search_node = playnodenum;
	bool nowPlayer = 0; // 0:自分  1:相手
	int turnnum = playnum;
	fill(usenode, usenode + 302, 0);
	usenode[playnum] = playnodenum;


	//ルートの展開
	expansion(search_node, nowPlayer, true);

	start_time = clock();
	while (clock() - start_time < (THINKING_TIME - 1) * CLOCKS_PER_SEC)	// UCTのループ
	{
		//木の端まで見る
		search_RF(turnnum, search_node, nowPlayer);
		search_tree(turnnum, search_node, nowPlayer);
		if (clock() - start_time > (THINKING_TIME - 1) * CLOCKS_PER_SEC)
		{
			break;
		}

		//展開
		if (!Nodes[search_node].finish && Nodes[search_node].value.play >= 1)
		{
			search_node = expansion(search_node, nowPlayer, false);
			//次の手
			turnnum++;
			nowPlayer = !nowPlayer;
			usenode[turnnum] = search_node;
		}

		//プレイアウトとバックプロパゲーション
		{
			//double reward = playout(nowPlayer, turnnum, Nodes[search_node].board);
			double reward = 0;
			reward = playout(nowPlayer, turnnum, Nodes[search_node].board) / 65536.0;

			//backpropagation();
			total_play++;
			while (turnnum >= playnum)
			{
				Nodes[usenode[turnnum]].value.play++;
				Nodes[usenode[turnnum]].value.win += reward;
				turnnum--;
			}
			//探索情報をリセット
			search_node = playnodenum;
			nowPlayer = 0;
			turnnum = playnum;
			used.clear();
		}


	}	// while(true)	UCTのループ

}	// Search()

//木の"親ノード","子ノード"を入れると、遷移時のMoveCommandを返す
MoveCommand UCTandAlphaBeta::toMoveCommand(NodeNum from, NodeNum to)
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

//MoveNode()の前に
void UCTandAlphaBeta::changeRed_pos(NodeNum to)
{
	Board nxboard = Nodes[to].board;
	blue_pos = nxboard.enblue;
	red_pos = nxboard.enred;
	if (red_pos == 0)
	{
		cout << "RedPos is Empty!" << endl;
		int r = rand[__popcnt64(blue_pos)](mt);
		red_pos = blue_pos;
		while (r--)
		{
			offBottomBB(red_pos);
		}
		red_pos = getBottomBB(red_pos);
	}
}

//rootノードからどの手を選んで指すか。ノードの添え字を返す
NodeNum UCTandAlphaBeta::Choice()
{
	assert(playnodenum < Nodes.size());
	assert(!Nodes[playnodenum].nextnodes.empty());
	NodeNum choice_nodenum = -1;
	double maxvalue = -MAX_VALUE;
	int maxvalue_i = 0;
	double softmax = 0;
	for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
	{
		softmax += exp(Nodes[nextnode].value.NN_score);
	}
	for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
	{
		// 自分の手番⇒大きいのを選ぶ
		// NN : 10%
		if (Nodes[nextnode].value.play == 0) continue;
		if (chmax(maxvalue, (double)Nodes[nextnode].value.win / Nodes[nextnode].value.play + exp(Nodes[nextnode].value.NN_score) / softmax * 0.1))
			//if (chmax(maxvalue, (double)Nodes[nextnode].value.win / Nodes[nextnode].value.play))
		{
			choice_nodenum = nextnode;
		}
	}
	assert(choice_nodenum != -1);
	assert(playnodenum != choice_nodenum);
	cout << "All {" << endl;
	for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
	{
		if (nextnode == choice_nodenum)
			printf("***");
		printf("\t");
		printf("Value:%8f", Nodes[nextnode].value.win);
		printf("\tPlay:%8d", Nodes[nextnode].value.play);
		printf("\tComp:%.6f", (Nodes[nextnode].value.play == 0 ? 0 : (double)Nodes[nextnode].value.win / Nodes[nextnode].value.play + exp(Nodes[nextnode].value.NN_score) / softmax * 0.1));
		printf("\tNNScore:%.6f", exp(Nodes[nextnode].value.NN_score) / softmax);
		printf("\n");
	}
	cout << "}\nChoose" << endl;
	cout << "Value:" << Nodes[choice_nodenum].value.win << " Play:" << Nodes[choice_nodenum].value.play;
	return choice_nodenum;
}

//ノードを遷移して、サーバーに送る文字列を返す（送ってはない）
Send UCTandAlphaBeta::MoveNode(NodeNum move_nodenum)
{
	assert(playnodenum != move_nodenum);
	MoveCommand move = toMoveCommand(playnodenum, move_nodenum);
	Send send = toSend(move, Game_::piecenum);
	Game_::pieces[Game_::piecenum[move.xy]] = nextpos(move);
	Game_::piecenum[nextpos(move)] = Game_::piecenum[move.xy];
	Game_::piecenum[move.xy] = -1;
	playnodenum = move_nodenum;
	return send;
}

void UCTandAlphaBeta::PrintStatus()
{
	printf("Turn:%d\n", playnum);
	printf("Total Playout Times : %d\n", total_play);
	printf("Node Num : %d\n", nodenum);
}


//AB
int UCTandAlphaBeta::evaluate(Board& board, int depth)
{
	int res = 0;
	if (Goal(board.myblue, 0))
		return 65535 + depth - MAXPLAY;
	else if (board.enred == 0)
		res = 0 - depth + MAXPLAY;
	else if (board.myblue == 0)
		res = 2000 - depth + MAXPLAY;
	else if (Goal(board.enblue, 1))
		res = 1000 - depth + MAXPLAY;
	else if (board.enblue == 0)
	{
		if (!Game_::purple || Red::decided || board.dead_enred < 3)
			return 65500 + depth - MAXPLAY;
		else
			res = 42768 + depth - MAXPLAY;
	}
	else if (board.myred == 0)
		res = 60000 + depth - MAXPLAY;
	else
		res = 32768 - depth + MAXPLAY + 1;
	//res += board.dead_myred * 40;
	if (!Game_::purple || board.dead_enred < 3 || Red::decided)
		res += board.dead_enblue * 400;
	else
		res -= board.dead_enblue * 1000;
	//res += board.dead_enblue * 400;

	BitBoard mix = board.my_mix;
	int mydist = (board.dead_myblue + board.dead_myred) * 12, endist = (board.dead_enblue + board.dead_enred) * 12;
	for (BitBoard bb = 0; mix != 0; offBottomBB(mix))
	{
		bb = getBottomBB(mix);
		mydist += (min(x(bb) - 1, 6 - x(bb)) + y(bb));
		//mydist += min(abs(x((BitBoard)MYGOAL_L) - x(bb)) + abs(y((BitBoard)MYGOAL_L) - y(bb)), abs(x((BitBoard)MYGOAL_R) - x(bb)) + abs(y((BitBoard)MYGOAL_R) - y(bb)));
	}
	mix = board.en_mix;
	for (BitBoard bb = 0; mix != 0; offBottomBB(mix))
	{
		bb = getBottomBB(mix);
		endist += (min(x(bb) - 1, 6 - x(bb)) + 7 - y(bb));
		//endist += min(abs(x((BitBoard)ENGOAL_L) - x(bb)) + abs(y((BitBoard)ENGOAL_L) - y(bb)), abs(x((BitBoard)ENGOAL_R) - x(bb)) + abs(y((BitBoard)ENGOAL_R) - y(bb)));
	}
	res += endist - mydist;
	res += rand[50](mt);	//動きを固定しないようにする（読まれないようにする）
	return res;
}


int UCTandAlphaBeta::alpha_beta(Board& search_board, bool nowPlayer, int depth, int alpha, int beta)
{
	nodesize++;
	if (depth == 0)
	{
		return evaluate(search_board, depth);
	}
	if (search_board.enred == 0)
	{
		if (4 - Game_::gameboard.dead_enred > (search_board.dead_enblue - Game_::gameboard.dead_enblue) + 1)
		{
			//探索を続行
		}
		else
		{
			return evaluate(search_board, depth);
		}
	}
	if (
		search_board.enblue == 0 ||
		search_board.dead_myblue == 4 ||
		search_board.dead_myred == 4 ||
		search_board.escape
		)
	{
		return evaluate(search_board, depth);
	}
	if (clock() - start_time >= (THINKING_TIME - 1) * CLOCKS_PER_SEC)
	{
		return -1;
	}

	Board prev = search_board;
	if (nowPlayer == 0)
	{
		BitBoard nposes = getNextPosBB(search_board.my_mix) & ~search_board.my_mix;
		nposes = InsideOrGoal(nposes, nowPlayer);	//盤面内
		for (BitBoard npos = 0; nposes != 0; offBottomBB(nposes))
		{
			npos = getBottomBB(nposes);
			assert(InsideOrGoal(npos, nowPlayer));
			BitBoard pposes = getNextPosBB(npos) & search_board.my_mix;
			for (BitBoard ppos = 0; pposes != 0; offBottomBB(pposes))
			{
				ppos = getBottomBB(pposes);
				if (Goal(npos, nowPlayer) && onPiece(search_board.myred, ppos))	//赤駒が脱出しようとした
					continue;
				bool deadred = onPiece(search_board.enred, npos);
				bool deadblue = onPiece(search_board.enblue, npos);
				assert(onPiece(search_board.my_mix, ppos));
				toNextBoard(search_board, ppos, npos, nowPlayer);
				if (chmax(alpha, alpha_beta(search_board, !nowPlayer, depth - 1, alpha, beta)))
				{
					if (depth == maxdepth)
					{
						resn = npos;
						resp = ppos;
					}
				}
				assert(onPiece(search_board.my_mix, npos));
				toNextBoard(search_board, npos, ppos, nowPlayer);
				if (deadred)
				{
					search_board.dead_enred--;
					assert(search_board.enred == 0);
					search_board.enred |= npos;
					search_board.en_mix |= npos;
				}
				else if (deadblue)
				{
					search_board.dead_enblue--;
					search_board.enblue |= npos;
					search_board.en_mix |= npos;
				}
				if (!(prev == search_board))
				{
					assert(false);
				}
				assert(prev == search_board);
				//search_board = prev;
				if (alpha >= beta) goto betacut;
			}
		}
	betacut:
		return alpha;
	}
	else
	{
		BitBoard nposes = getNextPosBB(search_board.en_mix) & ~search_board.en_mix;
		nposes = InsideOrGoal(nposes, nowPlayer);	//盤面内
		for (BitBoard npos = 0; nposes != 0; offBottomBB(nposes))
		{
			npos = getBottomBB(nposes);
			BitBoard pposes = getNextPosBB(npos) & search_board.en_mix;
			for (BitBoard ppos = 0; pposes != 0; offBottomBB(pposes))
			{
				ppos = getBottomBB(pposes);
				if (Goal(npos, nowPlayer) && onPiece(search_board.enred, ppos))	//赤駒が脱出しようとした
					continue;
				bool deadred = onPiece(search_board.myred, npos);
				bool deadblue = onPiece(search_board.myblue, npos);
				assert(onPiece(search_board.en_mix, ppos));
				toNextBoard(search_board, ppos, npos, nowPlayer);
				chmin(beta, alpha_beta(search_board, !nowPlayer, depth - 1, alpha, beta));
				assert(onPiece(search_board.en_mix, npos));
				toNextBoard(search_board, npos, ppos, nowPlayer);
				if (deadred)
				{
					search_board.dead_myred--;
					search_board.myred |= npos;
					search_board.my_mix |= npos;
				}
				else if (deadblue)
				{
					search_board.dead_myblue--;
					search_board.myblue |= npos;
					search_board.my_mix |= npos;
				}
				assert(prev == search_board);
				//search_board = prev;
				if (alpha >= beta) goto alphacut;
			}
		}
	alphacut:
		return beta;
	}
}

//探索をする
Send UCTandAlphaBeta::Search_AB()
{
	//事前にSetNode(str)をしておく

	MoveCommand move;
	int depth = 1;
	start_time = clock();

	while (clock() - start_time < (THINKING_TIME - 1) * CLOCKS_PER_SEC)
	{
		cout << "depth:" << depth << endl;
		if (playnum + depth > MAXPLAY)
			break;

		nodesize = 0;
		maxdepth = depth;

		int score = alpha_beta(playboard, 0, depth, -65535, 65535);

		cout << "node size:" << nodesize << endl;
		if (clock() - start_time < (THINKING_TIME - 1) * CLOCKS_PER_SEC)
		{
			assert(InsideOrGoal(resn, 0));
			move = toMoveCommand_AB(resp, resn);
			cout << "score:" << score << endl;
		}

		depth++;
	}	// while(true)	UCTのループ


	assert(InsideOrGoal(toBit(nextpos(move)), 0));
	changeRed_pos_AB(toBit(nextpos(move)));
	Send send = toSend(move, Game_::piecenum);
	Game_::pieces[Game_::piecenum[move.xy]] = nextpos(move);
	Game_::piecenum[nextpos(move)] = Game_::piecenum[move.xy];
	Game_::piecenum[move.xy] = -1;
	return send;

}	// Search()

int UCTandAlphaBeta::Check_AB()
{
	//事前にSetNode(str)をしておく

	int res = -65535;
	int depth = 1;
	start_time = clock();
	start_time -= (THINKING_TIME - 1) * CLOCKS_PER_SEC * 9 / 10;

	while (clock() - start_time < (THINKING_TIME - 1) * CLOCKS_PER_SEC)
	{
		cout << "depth:" << depth << endl;
		if (playnum + depth > MAXPLAY)
			break;

		nodesize = 0;
		maxdepth = depth;

		int score = alpha_beta(playboard, 0, depth, -65535, 65535);

		cout << "node size:" << nodesize << endl;
		if (clock() - start_time < (THINKING_TIME - 1) * CLOCKS_PER_SEC)
		{
			assert(InsideOrGoal(resn, 0));
			res = score;
			cout << "score:" << score << endl;
		}

		depth++;
	}	// while(true)	UCTのループ

	return res;
}

MoveCommand UCTandAlphaBeta::toMoveCommand_AB(BitBoard ppos, BitBoard npos)
{
	MoveCommand move;
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

void UCTandAlphaBeta::changeRed_pos_AB(BitBoard npos)
{
	blue_pos &= ~npos;
	if (onPiece(npos, red_pos))
	{
		cout << "RedPos is Empty!" << endl;
		int r = rand[__popcnt64(blue_pos)](mt);
		red_pos = blue_pos;
		while (r--)
		{
			offBottomBB(red_pos);
		}
		red_pos = getBottomBB(red_pos);
	}

}