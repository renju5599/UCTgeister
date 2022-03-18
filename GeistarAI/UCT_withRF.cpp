//////////////////////
//todo
//
// ������z�@�̌` 127bit
// ��36bit
// 6bit	�|���ꂽ����1��{�G��,�G��,����,����},�N������������,�N�����E�o����
// ��36bit
// 6bit	�|���ꂽ����2��{�G��,�G��,����,����},��,��
// �G36bit
// 6bit	�|���ꂽ����3��{�G��,�G��,����,����},��,��
// +1bit(=1)
// 
//////////////////////

#include<string>
#include<algorithm>
#include<time.h>
#include<cassert>

#include "types.h"
#include "method.h"
#include "NN_method.h"
#include "weight.h"
#include "UCT_withRF.h"
#include "Game.h"
#include "ColorGuess.h"

using namespace std;

using namespace UCT_RF;

//double UCT::compare(int win, int play, double NN_softmax)
//{
//	return ((win == 0 ? 0 : (double)win / play)) + NN_softmax * FACTOR * pow(total_play, 0.5) / (1 + play);
//}



//������
UCT::UCT(Recieve str)
{
	// ���[�g�m�[�h������
	Nodes.resize(2);	//map�̊֌W��0�Ԗڂ͋�ɂ���
	Nodes[1].board = toBoard(str);
	//Nodes[1].value = { 0,0, MAX_VALUE };
	Nodes[1].value = { 0, 0, 0 };
	Nodes[1].nextnodes.clear();
	Nodes[1].finish = 0;

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

// ���ۂ̔Ֆʂ̃m�[�h�Ɉړ�
void UCT::SetNode(Recieve str)
{
	playnum++;	//�^�[��������i�߂�
	
	//if (!Nodes[playnodenum].nextnodes.empty())
	//{
	//	pieces = toPieces(str);
	//	Board nowboard = toBoard(str);
	//	if (nowboard == Nodes[playnodenum].board)
	//		return;
	//	for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
	//	{
	//		if (Nodes[nextnode].board == nowboard)
	//		{
	//			//total_play = Nodes[nextnode].value.play;
	//			playnodenum = nextnode;
	//			break;
	//		}
	//	}
	//	if (Nodes[playnodenum].board == nowboard)
	//	{
	//		//total_play = Nodes[playnodenum].value.play;
	//		return;
	//	}
	//}
	// �T���؂Ƀm�[�h��������΍\�z���Ȃ���
	Nodes.resize(2);
	Nodes[1].board = toBoard(str);
	//Nodes[1].value = { 0,0, MAX_VALUE };
	//������z�@�̃��f����ʂ����l��ۑ�
	BitsStatus bsta, bsta_null;
	toBitsStatus(bsta, Nodes[1].board);
	Nodes[1].value = { 0, 0, getNNres(bsta.TwoPiece, bsta_null.TwoPiece, 0) };
	Nodes[1].nextnodes.clear();
	Nodes[1].finish = 0;

	nodenum = 2;
	total_play = 0;
	playnodenum = 1;
	//pieces = toPieces(str);

	board_index.clear();
}

//UCT�̊e�m�[�h�̉��l(?)���v�Z�E�ۑ�����
//void UCT::UpdateBoardValue(NodeValue& v)
//{
//	if (v.play == 0)
//	{
//		v.comp = MAX_VALUE;
//		return;
//	}
//	assert(total_play != 0);
//	//v.comp = ((double)v.win / v.play) + FACTOR * pow(log(total_play) / v.play, 0.5);
//	v.comp = compare(v.win, v.play);
//}

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

//UCT���̃v���C�A�E�g������֐�
int UCT::playout(bool nowPlayer, int turnnum, Board nowboard)
{
	int bluenum = (4 - nowboard.dead_enblue) - __popcnt64(nowboard.enblue);	//�킩���Ă��Ȃ��̐�
	int rednum = (4 - nowboard.dead_enred) - __popcnt64(nowboard.enred);	//�킩���Ă��Ȃ��Ԃ̐�
	int undefinenum = bluenum + rednum;

	int ennum = __popcnt64(nowboard.enemy);	//�����Ă���G�̐�
	
	BitsStatus bsta_pre;
	double res_pre;

	//�|����Ă���G���F���߂���
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

	//�Ֆʏ�̓G���F���߂���
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
					assert(onPiece(nowboard.enemy, nowboard.pieces[i]));
				}
				sum -= Red::eval[i];
				colored[i] = 1;
				break;
			}
		}
		undefinenum--;
	}
	nowboard.dead_enblue = (8 - ennum) - nowboard.dead_enred;
	nowboard.enblue = nowboard.enemy & ~nowboard.enred;
	assert(nowboard.enemy == (nowboard.enblue ^ nowboard.enred));

	//�ǂ����������_���ɂȂ��Ă��Ȃ����Ă������i�G�X�P�[�v�ȊOLOSE�ɂ��Ă���΂ɏ��Ɠǂ�ł��܂��j
	while (true)
	{
		//���������Ă��邩����
		if (turnnum > MAXPLAY)
			return DRAW_VALUE;
		if (nowboard.escape)
			return (nowPlayer == 0 ? LOSE_VALUE : WIN_VALUE);
		if (nowboard.dead_myblue == 4)
			return LOSE_VALUE;
		if (nowboard.dead_enred == 4)
			return LOSE_VALUE;
		if (nowboard.dead_myred == 4)
			return WIN_VALUE;
		if (nowboard.dead_enblue == 4)
			return WIN_VALUE;
		assert(__popcnt64(nowboard.enemy) >= 2);
		assert(nowboard.enemy == (nowboard.enblue ^ nowboard.enred));

		/////	������z�@��p����

		//Board nextboards[32];
		//int nextboards_size = 0;
		//PossibleNextBoard(nextboards, nowPlayer, nowboard, nextboards_size);
		//int nx = NextBoard_byNN(nextboards, nowPlayer, nextboards_size, bsta_pre, res_pre);
		//nowboard = nextboards[nx];

		////

		//�������x����
		//vector<Board> nextboards;
		//PossibleNextBoard(nextboards, nowPlayer, nowboard);
		//int nextmoves_size = nextboards.size();
		//assert(nextmoves_size > 0);
		//int r = rand() % nextmoves_size;
		//nowboard = nextboards[r];	//�����_���Ɏ��̎�����߂�

		//�ς��Ă݂�
		BitBoard ppos = 0, npos = 0;
		//�S�[���ł�����S�[������
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
			//assert(__popcnt64(ppos) == 1);
			ppos = getBottomBB(ppos);
			npos = getNextPosBB(ppos) & ENGOAL;
			assert(__popcnt64(npos) == 1);
		}
		//�ʏ�̓���
		else
		{
			ppos = (nowPlayer == 0 ? nowboard.my : nowboard.enemy);
			npos = Inside(getNextPosBB(ppos) & ~ppos);	//�ړ�����𒊏o
			assert(ppos != 0);
			assert(npos != 0);
			//�ړ���������_���ɑI��
			int r = rand[__popcnt64(npos)](mt);
			while (r--)
				offBottomBB(npos);
			npos = getBottomBB(npos);
			//�I�����ꂽ�ړ���ɍs�����̒��Ń����_���ɑI��
			ppos &= getNextPosBB(npos);
			assert(ppos != 0);
			r = rand[__popcnt64(ppos)](mt);
			while (r--)
				offBottomBB(ppos);
			ppos = getBottomBB(ppos);
		}
		//���߂��s����board�ɕϊ�����
		toNextBoard(nowboard, ppos, npos, nowPlayer);
		
		
		//�^�[����i�߂�
		nowPlayer = !nowPlayer;
		turnnum++;
	} // while(true)
}

void UCT::search_tree(int& turnnum, int& search_node, bool& nowPlayer)
{
	while (turnnum < MAXPLAY && !Nodes[search_node].nextnodes.empty() && !Nodes[search_node].finish)	//�؂̒[�܂Ō���
	{
		NodeNum choice_nodenum = -1;
		double maxvalue = -MAX_VALUE, minvalue = MAX_VALUE;
		assert(Nodes[search_node].board.willplayer == nowPlayer);
		double softmax_sum = 0;
		for (NodeNum nextnode : Nodes[search_node].nextnodes)
		{
			//softmax_sum += exp((nowPlayer == 0 ? 1 : -1) * Nodes[nextnode].value.NN_score);
			if (nowPlayer == 0) softmax_sum += exp(Nodes[nextnode].value.NN_score);
		}
		for (NodeNum nextnode : Nodes[search_node].nextnodes)
		{
			assert(Nodes[nextnode].board.willplayer != nowPlayer);
			if (used[nextnode])	//2�x�����Ֆʂ͌��Ȃ�
				continue;
			// �����̎�ԁˑ傫���̂�I��  ����̎�ԁˏ������̂�I��
			// ����̕]���l�͂킩���̂Ń����_��UCT�Ɠ��l�ɏ�����
			if ((nowPlayer == 0 ?
				chmax(maxvalue, compare_RF(Nodes[nextnode].value.win / 2.0, Nodes[nextnode].value.play, Nodes[search_node].value.play, exp(Nodes[nextnode].value.NN_score) / softmax_sum)) :
				//chmax(maxvalue, compare_RF(Nodes[nextnode].value.play - Nodes[nextnode].value.win / 2.0, Nodes[nextnode].value.play, Nodes[search_node].value.play, exp(-Nodes[nextnode].value.NN_score) / softmax_sum))))
				//chmax(maxvalue, compare(Nodes[nextnode].value.win / 2.0, Nodes[nextnode].value.play, total_play)) :
				chmax(maxvalue, compare(Nodes[nextnode].value.play - Nodes[nextnode].value.win / 2.0, Nodes[nextnode].value.play, total_play))))
			{
				choice_nodenum = nextnode;
			}
		}
		if (choice_nodenum == -1)	//�������Ƃ���Ֆʂ����Ȃ���΃v���C�A�E�g
		{
			break;
		}

		turnnum++;
		used[choice_nodenum] = 1;
		search_node = choice_nodenum;
		nowPlayer = !nowPlayer;
		//usenode.emplace_back(search_node);
		usenode[turnnum] = search_node;
	}//while(!empty())
}

int UCT::expansion(int search_node, bool nowPlayer)
{
	// �łĂ��̌��𒊏o����
	Board nextboards[32];
	int nextboards_size = 0;
	PossibleNextBoard(nextboards, nowPlayer, Nodes[search_node].board, nextboards_size);
	assert(nextboards_size > 0);

	//Nodes�̊g��
	//for (const Board& nextboard : nextboards)
	BitsStatus bsta, bsta_pre;
	double res_pre = Nodes[search_node].value.NN_score;
	toBitsStatus(bsta_pre, Nodes[search_node].board);
	int nx = 0;
	double res_max = -MAX_VALUE;
	for (int i = 0; i < nextboards_size; i++)
	{
		Board nextboard = nextboards[i];
		assert(nowPlayer == Nodes[search_node].board.willplayer);
		assert(nextboard.willplayer != nowPlayer);
		assert(Nodes[search_node].board.willplayer != nextboard.willplayer);
		if (board_index[nextboard] != 0)	//�n�b�V�����ݒ�ς݁i�T���ς݂̃m�[�h�j�͌q���邾��
		{
			assert(Nodes[search_node].board.willplayer != Nodes[board_index[nextboard]].board.willplayer);
			Nodes[search_node].nextnodes.push_back(board_index[nextboard]);
			continue;
		}
		board_index[nextboard] = nodenum;	//�V���Ƀn�b�V����ݒ肷��

		Node nextnode;
		nextnode.board = nextboard;
		nextnode.nextnodes.clear();

		//���������Ă��邩
		nextnode.finish = (
			__popcnt64(nextnode.board.enemy) < 2 ||
			nextnode.board.dead_enblue == 4 ||
			nextnode.board.dead_enred == 4 ||
			nextnode.board.dead_myblue == 4 ||
			nextnode.board.dead_myred == 4 ||
			nextnode.board.escape
			);

		//������z�@�̃��f����ʂ����l��ۑ�
		toBitsStatus(bsta, nextboard);
		nextnode.value.NN_score = getNNres(bsta.TwoPiece, bsta_pre.TwoPiece, res_pre);
		//�ł��l���ǂ���ɂ���
		if (chmax(res_max, (nowPlayer == 0 ? nextnode.value.NN_score : -nextnode.value.NN_score)))
		{
			nx = i;
		}

		Nodes.emplace_back(nextnode);
		assert(Nodes[search_node].board.willplayer != Nodes[nodenum].board.willplayer);
		Nodes[search_node].nextnodes.push_back(nodenum);
		assert(Nodes[nodenum] == nextnode);
		nodenum++;
	}	//for nextmoves_size

	//���̎�
	return Nodes[search_node].nextnodes[nx];
}

//UCT�T��������
void UCT::Search()
{
	//���O��SetNode(str)�����Ă���
	NodeNum search_node = playnodenum;
	bool nowPlayer = 0; // 0:����  1:����
	int turnnum = playnum;
	fill(usenode, usenode + 302, 0);
	usenode[playnum] = playnodenum;
	

	assert(Nodes[playnodenum].board.willplayer == nowPlayer);

	//���[�g�̓W�J
	expansion(search_node, nowPlayer);
	
	clock_t start_time = clock();
	while (clock() - start_time < (THINKING_TIME - 1) * CLOCKS_PER_SEC)	// UCT�̃��[�v
	{
		//�؂̒[�܂Ō���
		search_tree(turnnum, search_node, nowPlayer);

		//�W�J
		if (total_play == 0 || (!Nodes[search_node].finish && Nodes[search_node].value.play >= 1))
		{
			search_node = expansion(search_node, nowPlayer);
			//���̎�
			turnnum++;
			//assert(!Nodes[search_node].nextnodes.empty());
			nowPlayer = !nowPlayer;
			//usenode.emplace_back(search_node);
			usenode[turnnum] = search_node;
		}

		//�v���C�A�E�g�ƃo�b�N�v���p�Q�[�V����
		{
			//playout();
			int reward = playout(nowPlayer, turnnum, Nodes[search_node].board);

			//backpropagation();
			total_play++;
			while (turnnum >= playnum)
			{
				Nodes[usenode[turnnum]].value.play++;
				Nodes[usenode[turnnum]].value.win += reward;
				turnnum--;
				//UpdateBoardValue(Nodes[node].value);
			}
			//�T���������Z�b�g
			//usenode.resize(1, playnodenum);
			search_node = playnodenum;
			nowPlayer = 0;
			turnnum = playnum;
			used.clear();
		}

		
	}	// while(true)	UCT�̃��[�v

}	// Search()

//�؂�"�e�m�[�h","�q�m�[�h"������ƁA�J�ڎ���MoveCommand��Ԃ�
MoveCommand UCT::toMoveCommand(NodeNum from, NodeNum to)
{
	cout << "from:" << from << " to:" << to << endl;
	assert(from != to);
	assert(Nodes[from].board.willplayer != Nodes[to].board.willplayer);
	MoveCommand move;
	BitBoard from_bb = Nodes[from].board.my;
	BitBoard to_bb = Nodes[to].board.my;
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

//root�m�[�h����ǂ̎��I��Ŏw�����B�m�[�h�̓Y������Ԃ�
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
		// �����̎�ԁˑ傫���̂�I��
		// NN : 10%
		if (Nodes[nextnode].value.play == 0) continue;
		//if (chmax(maxvalue, (double)Nodes[nextnode].value.win / Nodes[nextnode].value.play))
		if (chmax(maxvalue, (double)Nodes[nextnode].value.win / 2.0 / Nodes[nextnode].value.play + exp(Nodes[nextnode].value.NN_score) / softmax * 0.00))
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
		cout << "Value:" << Nodes[nextnode].value.win << " Play:" << Nodes[nextnode].value.play;
		cout << " NN_score:" << exp(Nodes[nextnode].value.NN_score) / softmax << endl;
	}
	cout << "}\nChoose" << endl;
	cout << "Value:" << Nodes[choice_nodenum].value.win << " Play:" << Nodes[choice_nodenum].value.play;
	cout << " NN_score:" << exp(Nodes[choice_nodenum].value.NN_score) / softmax << endl;
	return choice_nodenum;
}

//�m�[�h��J�ڂ��āA�T�[�o�[�ɑ��镶�����Ԃ��i�����Ă͂Ȃ��j
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