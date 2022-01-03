//////////////////////
//todo
//
// 
//////////////////////

#include<vector>
#include<string>
#include<algorithm>
#include<time.h>
#include<cassert>
#include<random>

#include "types.h"
#include "method.h"
#include "UCT.h"

using namespace std;


//移動前の位置pposと移動先の位置nposから boardを更新する
void toNextBoard(Board& board, BitBoard ppos, BitBoard npos, bool nowPlayer)
{
	if (Goal(npos, nowPlayer))	//脱出した
		board.escape = true;
	else	//脱出してない
		board.escape = false;

	if (nowPlayer == 0)
	{
		if (onPiece(board.myblue, ppos))	//青を動かした
			change(board.myblue, ppos, npos);	//駒の移動
		else	//赤を動かした
			change(board.myred, ppos, npos);

		//青駒を取った
		if (onPiece(board.enblue, npos))
		{
			board.enemy ^= npos;
			board.enblue ^= npos;
			board.dead_enblue <<= 1;
			board.kill = true;
		}
		//赤駒を取った
		else if (onPiece(board.enred, npos))
		{
			board.enemy ^= npos;
			board.enred ^= npos;
			board.dead_enred <<= 1;
			board.kill = true;
		}
		else if(onPiece(board.enemy, npos))
		{
			board.enemy ^= npos;	//敵駒を取った後の状態に
			board.kill = true;
		}
		else	//敵駒を取らなかった
		{
			board.kill = false;
		}
		assert(board.enemy != 0);
	}
	else
	{
		change(board.enemy, ppos, npos);
		if (onPiece(board.enblue, ppos))
			change(board.enblue, ppos, npos);
		if (onPiece(board.enred, ppos))
			change(board.enred, ppos, npos);

		if (onPiece(board.myblue, npos))	//青駒を取られた
		{
			board.myblue ^= npos;
			board.kill = true;
			board.dead_myblue <<= 1;
		}
		else if (onPiece(board.myred, npos))	//赤駒を取られた
		{
			board.myred ^= npos;
			board.kill = true;
			board.dead_myred <<= 1;
		}
		else
		{
			board.kill = false;
		}
	}
}


// すべての合法手後の盤面をvectorに入れる
// 敵駒の色判定はしない
void PossibleNextBoard(vector<Board>& nextpositions, bool nowPlayer, Board board)
{
	BitBoard nowmy = board.myblue | board.myred;

	//getBottomBB()とoffBottomBB()を使って高速に調べるようにしたい
	if (nowPlayer == 0)
	{
		// どの位置の駒を動かすかのfor
		for (BitBoard piecebb = nowmy; piecebb != 0; offBottomBB(piecebb))
		{
			BitBoard ppos = getBottomBB(piecebb);
			assert(ppos != 0);
			// 移動4方向を見るfor
			for (BitBoard nextbb = getNextPosBB(ppos); nextbb != 0; offBottomBB(nextbb))
			{
				BitBoard npos = getBottomBB(nextbb);
				assert(npos != 0);

				//合法かを見る
				if (!Inside(npos) && !Goal(npos, nowPlayer))	//盤面外
					continue;
				if (Goal(npos, nowPlayer) && onPiece(board.myred, ppos))	//赤駒が脱出しようとした
					continue;
				if (onPiece(nowmy, npos))	//自身の駒が重なる
					continue;

				Board nextboard = board;
				toNextBoard(nextboard, ppos, npos, nowPlayer);
				nextpositions.push_back(nextboard);

			} // for nextbb
		} // for piecebb
	} // if nowPlayer == 0
	else
	{
		for (BitBoard piecebb = board.enemy; piecebb != 0; offBottomBB(piecebb))
		{
			BitBoard ppos = getBottomBB(piecebb);
			for (BitBoard nextbb = getNextPosBB(ppos); nextbb != 0; offBottomBB(nextbb))
			{
				BitBoard npos = getBottomBB(nextbb);
				if (!Inside(npos) && !Goal(npos, nowPlayer))	//盤面外
					continue;
				if (Goal(npos, nowPlayer) && onPiece(board.enred, ppos))	//赤駒が脱出しようとした
					continue;
				if (onPiece(board.enemy, npos))	//自身の駒が重なる
					continue;

				Board nextboard = board;
				toNextBoard(nextboard, ppos, npos, nowPlayer);
				nextpositions.push_back(nextboard);

			}
		}

	} //nowPlayer == 1

	return;
}

//初期化
UCT::UCT(Recieve str)
{
	// ルートノードをつくる
	Nodes.resize(2);	//mapの関係で0番目は空にする
	Nodes[1].board = toBoard(str);
	Nodes[1].value = { 0,0, MAX_VALUE };
	Nodes[1].nextnodes.resize(0);
	Nodes[1].finish = 0;

	nodenum = 2;
	total_play = 0;
	playnodenum = 1;
	playnum = 0;
	pieces = toPieces(str);
}

// 実際の盤面のノードに移動
void UCT::SetNode(Recieve str)
{
	playnum++;	//ターン数を一つ進める
	
	if (!Nodes[playnodenum].nextnodes.empty())
	{
		pieces = toPieces(str);
		Board nowboard = toBoard(str);
		if (nowboard == Nodes[playnodenum].board)
			return;
		for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
		{
			if (Nodes[nextnode].board == nowboard)
			{
				//total_play = Nodes[nextnode].value.play;
				playnodenum = nextnode;
				break;
			}
		}
		if (Nodes[playnodenum].board == nowboard)
			return;
	}
	// 探索木にノードが無ければ構築しなおす
	Nodes.resize(2);
	Nodes[1].board = toBoard(str);
	Nodes[1].value = { 0,0, MAX_VALUE };
	Nodes[1].nextnodes.resize(0);
	Nodes[1].finish = 0;

	nodenum = 2;
	total_play = 0;
	playnodenum = 1;
	pieces = toPieces(str);

	board_index.clear();
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
	//v.comp = ((double)v.win / v.play) + FACTOR * pow(log(total_play) / v.play, 0.5);
	v.comp = compare(v.win, v.play);
}

int livenum(Dead d)
{
	switch (d)
	{
		case 1:
			return 4;
		case 2:
			return 3;
		case 4:
			return 2;
		case 8:
			return 1;
		case 16:
			return 0;
	}
	assert(false);
	return -1;
}

//UCT内のプレイアウトをする関数
int UCT::playout(bool nowPlayer, int turnnum, Board nowboard)
{
	int bluenum = livenum(nowboard.dead_enblue) - __popcnt64(nowboard.enblue);	//わかっていない青の数
	int rednum = livenum(nowboard.dead_enred) - __popcnt64(nowboard.enred);	//わかっていない赤の数
	int undefinenum = bluenum + rednum;

	int ennum = __popcnt64(nowboard.enemy);	//生きている敵の数
	
	//倒されている敵駒を色決めする
	while (ennum < undefinenum)
	{
		int r = rand() % undefinenum;
		if (r < bluenum)
		{
			nowboard.dead_enblue <<= 1;
			bluenum--;
		}
		else
		{
			nowboard.dead_enred <<= 1;
			rednum--;
		}
		undefinenum--;
	}

	//盤面上の敵駒を色決めする
	while (bluenum--)
	{
		int r = rand() % undefinenum;
		BitBoard bb = nowboard.enemy & ~nowboard.enblue & ~nowboard.enred;
		while (r--)
		{
			offBottomBB(bb);
		}
		bb = getBottomBB(bb);
		nowboard.enblue |= bb;
		undefinenum--;
	}
	assert(undefinenum == rednum);
	nowboard.enred = nowboard.enemy & ~nowboard.enblue;

	while (true)
	{

		//勝負がついているか見る
		if (turnnum > MAXPLAY)
			return DRAW_VALUE;
		if (nowboard.escape)
			return (nowPlayer == 0 ? LOSE_VALUE : WIN_VALUE);
		if (nowboard.dead_myblue == 16 || nowboard.dead_enred == 16)
			return LOSE_VALUE;
		if (nowboard.dead_myred == 16 || nowboard.dead_enblue == 16)
			return WIN_VALUE;
		assert(__popcnt64(nowboard.enemy) >= 2);
		assert(nowboard.enemy == (nowboard.enblue ^ nowboard.enred));

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
			assert(__popcnt64(ppos) == 1);
			npos = getNextPosBB(nowboard.myblue) & MYGOAL;
			assert(__popcnt64(npos) == 1);
		}
		else if (nowPlayer == 1 && Goal(getNextPosBB(nowboard.enblue), 1))
		{
			ppos = getNextPosBB(ENGOAL) & nowboard.enblue;
			assert(__popcnt64(ppos) == 1);
			npos = getNextPosBB(nowboard.enblue) & ENGOAL;
			assert(__popcnt64(npos) == 1);
		}
		//通常の動き
		else
		{
			ppos = (nowPlayer == 0 ? (nowboard.myblue | nowboard.myred) : nowboard.enemy);
			npos = Inside(getNextPosBB(ppos) & ~ppos);	//移動先候補を抽出
			assert(ppos != 0);
			assert(npos != 0);
			//移動先をランダムに選択
			int r = rand() % __popcnt64(npos);
			while (r--)
				offBottomBB(npos);
			npos = getBottomBB(npos);
			//選択された移動先に行ける駒の中でランダムに選択
			ppos &= getNextPosBB(npos);
			assert(ppos != 0);
			r = rand() % __popcnt64(ppos);
			while (r--)
				offBottomBB(ppos);
			ppos = getBottomBB(ppos);
		}
		//決めた行動をboardに変換する
		toNextBoard(nowboard, ppos, npos, nowPlayer);
		//
		
		//ターンを進める
		nowPlayer = !nowPlayer;
		turnnum++;
	} // while(true)
}

void UCT::ParallelSearch()
{
	cout << "最大スレッド数: " << thread::hardware_concurrency() << endl;
	for (int i = 0; i < thread::hardware_concurrency(); i++)
	{
		threads.emplace_back(&UCT::Search, this);
	}
	for (auto& t : threads)
	{
		t.join();
	}
}

//UCT探索をする
void UCT::Search()
{
	//事前にSetNode(str)をしておく
	NodeNum search_node = playnodenum;
	vector<NodeNum> usenode(1, playnodenum);
	map<NodeNum, bool> used;
	bool nowPlayer = 0; // 0:自分  1:相手
	int turnnum = playnum;

	clock_t start_time = clock();
	while (clock() - start_time < 1.0 * CLOCKS_PER_SEC)	// UCTのループ
	{
		while (!Nodes[search_node].nextnodes.empty())	//木の端まで見る
		{
			turnnum++;
			NodeNum choice_nodenum = -1;
			double maxvalue = 0, minvalue = MAX_VALUE;
			for (NodeNum nextnode : Nodes[search_node].nextnodes)
			{
				if (used[nextnode])	//2度同じ盤面は見ない
					continue;
				// 自分の手番⇒大きいのを選ぶ  相手の手番⇒小さいのを選ぶ
				// 相手番でプレイアウト回数を考慮できていなかったのを修正
				if (Nodes[nextnode].value.play == 0)
				{
					choice_nodenum = nextnode;
					break;
				}
				//if ((nowPlayer == 0 ? chmax(maxvalue, Nodes[nextnode].value.comp) : chmin(minvalue, Nodes[nextnode].value.comp)))
				if ((nowPlayer == 0 ? 
					chmax(maxvalue, compare(Nodes[nextnode].value.win, Nodes[nextnode].value.play)) : 
					chmax(maxvalue, compare(-Nodes[nextnode].value.win, Nodes[nextnode].value.play))))
				{
					choice_nodenum = nextnode;
				}
			}
			if (choice_nodenum == -1)	//見たことある盤面しかなければプレイアウト
			{
				break;
			}

			used[choice_nodenum] = 1;
			search_node = choice_nodenum;
			nowPlayer = !nowPlayer;
			usenode.push_back(search_node);
		}//while(!empty())

		if (!Nodes[search_node].finish && Nodes[search_node].value.play >= 1)	//展開
		{
			// 打てる手の候補を抽出する
			vector<Board> nextboards;
			PossibleNextBoard(nextboards, nowPlayer, Nodes[search_node].board);
			int nextboards_size = nextboards.size();
			assert(nextboards_size > 0);

			//Nodesの拡張
			for (int i = 0; i < nextboards_size; i++)
			{
				if (board_index[nextboards[i]] != 0)
				{
					Nodes[search_node].nextnodes.push_back(board_index[nextboards[i]]);
					continue;
				}
				board_index[nextboards[i]] = nodenum;

				Node nextnode;
				nextnode.board = nextboards[i];
				nextnode.nextnodes.resize(0);

				//勝負がついているか
				nextnode.finish = (
					__popcnt64(nextnode.board.enemy) < 2 ||
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
			int reward = playout(nowPlayer, turnnum, Nodes[search_node].board);

			//backpropagation();
			//mtx.lock();
			total_play++;
			for (NodeNum node : usenode)
			{
				Nodes[node].value.play++;
				Nodes[node].value.win += reward;
				//UpdateBoardValue(Nodes[node].value);
			}
			//mtx.unlock();
			//探索情報をリセット
			usenode.resize(1, playnodenum);
			search_node = playnodenum;
			nowPlayer = 0;
			turnnum = playnum;
			used.clear();
		}	// else

		/*
		turnnum++;

		// 打てる手の候補を抽出する
		vector<Board> nextboards;
		PossibleNextBoard(nextboards, nowPlayer, Nodes[search_node].board);
		int nextboards_size = nextboards.size();
		assert(nextboards_size > 0);

		if (Nodes[search_node].nextnodes.empty())	//葉ノード
		{

			if (!Nodes[search_node].finish && Nodes[search_node].value.play >= 1)	//展開
			{
				//Nodesの拡張
				for (int i = 0; i < nextboards_size; i++)
				{
					if (board_index[nextboards[i]] != 0)
					{
						Nodes[search_node].nextnodes.push_back(board_index[nextboards[i]]);
						continue;
					}
					board_index[nextboards[i]] = nodenum;

					Node nextnode;
					nextnode.board = nextboards[i];
					nextnode.nextnodes.resize(0);

					//勝負がついているか
					nextnode.finish = (
						__popcnt64(nextnode.board.enemy) < 2 ||
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
				PLAYOUT:
				//playout();
				int reward = playout(nowPlayer, turnnum, Nodes[search_node].board);

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
				turnnum = playnum;
				used.clear();
			}	// else
		} // if 葉ノード

		else	//木の探索
		{
			NodeNum choice_nodenum = -1;
			double maxvalue = 0, minvalue = MAX_VALUE;
			for (NodeNum nextnode : Nodes[search_node].nextnodes)
			{
				if (used[nextnode])
					continue;
				// 自分の手番⇒大きいのを選ぶ  相手の手番⇒小さいのを選ぶ
				if (Nodes[nextnode].value.comp == MAX_VALUE)
				{
					choice_nodenum = nextnode;
					break;
				}
				if ((nowPlayer == 0 ? chmax(maxvalue, Nodes[nextnode].value.comp) : chmin(minvalue, Nodes[nextnode].value.comp)))
				{
					choice_nodenum = nextnode;
				}
			}
			if (choice_nodenum == -1)
			{
				//許してください。。。。。。。。。。。。。。。。。。。。。
				goto PLAYOUT;
			}

			used[choice_nodenum] = 1;
			search_node = choice_nodenum;
			nowPlayer = !nowPlayer;
			usenode.push_back(search_node);
		}
		*/
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
	double maxvalue = (double)Nodes[choice_nodenum].value.win / Nodes[choice_nodenum].value.play;
	for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
	{
		// 自分の手番⇒大きいのを選ぶ
		if (chmax(maxvalue, (double)Nodes[nextnode].value.win / Nodes[nextnode].value.play))
		{
			choice_nodenum = nextnode;
		}
	}
	assert(playnodenum != choice_nodenum);
	cout << "Value:" << Nodes[choice_nodenum].value.win << " Play:" << Nodes[choice_nodenum].value.play << endl;
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
	printf("Total Playout Times : %d\n", total_play);
	printf("Node Num : %d\n", nodenum);
}
