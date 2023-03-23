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
#include<iomanip>

#include "types.h"
#include "method.h"
#include "UCT_Afterstates.h"
#include "NN_method.h"
#include "weight.h"
#include "Game.h"
#include "ColorGuess.h"

using namespace std;

using namespace UCT_Afterstates;



//������
UCT::UCT(Recieve str)
{
	// ���[�g�m�[�h������
	Nodes.resize(2);	//map�̊֌W��0�Ԗڂ͋�ɂ���
	Nodes[1].board = toBoard(str);
	Nodes[1].value = { 0, 0, 0 };
	Nodes[1].nextnodes.clear();
	Nodes[1].finish = 0;

	nodenum = 2;
	total_play = 0;
	playnodenum = 1;
	playnum = 0;

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
	
	// �T���؂��\�z���Ȃ���
	Nodes.resize(2);
	Nodes[1].board = toBoard(str);
	BitsStatus bsta, bsta_null;
	toBitsStatus(bsta, Nodes[1].board);
	Nodes[1].value = { 0, 0, getNNres(bsta.TwoPiece, bsta_null.TwoPiece, 0) };
	Nodes[1].nextnodes.clear();
	Nodes[1].finish = 0;

	nodenum = 2;
	total_play = 0;
	playnodenum = 1;

	board_index.clear();
	
	//�����̐Ԃ��o����
	for (int i = 0; i < 8; i++)
	{
		if (onPiece(Nodes[1].board.myred, Game_::pieces[i]) && Red::eval[i] == RED_CNTSTOP)
		{
			Nodes[1].board.myred_eval = true;
			break;
		}
	}
}



//UCT���̃v���C�A�E�g������֐�
double UCT::playout(bool nowPlayer, int turnnum, Board nowboard)
{
	//��̐F����
	int bluenum = 4 - nowboard.dead_enblue - __popcnt64(nowboard.enblue);	//�킩���Ă��Ȃ��̐�
	int rednum = 4 - nowboard.dead_enred - __popcnt64(nowboard.enred);	//�킩���Ă��Ȃ��Ԃ̐�
	int undefinenum = __popcnt64(nowboard.en) + nowboard.dead_en;
	int ennum = __popcnt64(nowboard.en_mix);	//�����Ă���G�̐�
	assert(nowboard.en_mix == (nowboard.en ^ nowboard.enblue ^ nowboard.enred));
	assert(undefinenum == bluenum + rednum);

	//�|����Ă���G���F���߂���
	while (ennum < undefinenum)
	{
		int r = rand[undefinenum](mt);
		if (r < bluenum)
		{
			nowboard.dead_enblue++;
			bluenum--;
		}
		else
		{
			nowboard.dead_enred++;
			rednum--;
		}
		undefinenum--;
	}

	//�Ֆʏ�̓G���F���߂���
	while (bluenum--)
	{
		int r = rand[undefinenum](mt);
		BitBoard bb = nowboard.en;
		while (r--)
		{
			offBottomBB(bb);
		}
		bb = getBottomBB(bb);
		nowboard.enblue |= bb;
		nowboard.en ^= bb;
		undefinenum--;
	}
	assert(undefinenum == rednum);
	nowboard.enred |= nowboard.en;
	nowboard.en = 0;
	assert(nowboard.en_mix == (nowboard.enblue ^ nowboard.enred ^ nowboard.en));

	while (true)
	{

		//���������Ă��邩����
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
		assert(nowboard.en_mix == (nowboard.enblue ^ nowboard.enred ^ nowboard.en));

		/////	������z�@��p����

		//Board nextboards[32];
		//int nextboards_size = 0;
		//PossibleNextBoard(nextboards, nowPlayer, nowboard, nextboards_size);
		//int nx = NextBoard_byNN(nextboards, nowPlayer, nextboards_size, bsta_pre, res_pre);
		//nowboard = nextboards[nx];

		////

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
			ppos = getBottomBB(ppos);
			npos = getNextPosBB(ppos) & ENGOAL;
			assert(__popcnt64(npos) == 1);
		}
		//�ʏ�̓���
		else
		{
			ppos = (nowPlayer == 0 ? nowboard.my_mix : nowboard.en_mix);
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
		toNextBoard_quick(nowboard, ppos, npos, nowPlayer);
		
		
		//�^�[����i�߂�
		nowPlayer = !nowPlayer;
		turnnum++;
	} // while(true)
}

void UCT::search_RF(int& turnnum, NodeNum& search_node, bool& nowPlayer)
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
		if (used[nextnode])	//2�x�����Ֆʂ͌��Ȃ�
			continue;
		// �����̎�ԁˑ傫���̂�I��  ����̎�ԁˏ������̂�I��
		// (����̕]���l�͂킩���̂Ŏ������_�̕����t)
		// PUCT�͑��葤�̓����_��(�񐔍ŏ�)���ۂ�
		if ((nowPlayer == 0 ?
			chmax(maxvalue, compare_RF(Nodes[nextnode].value.win / 2.0, Nodes[nextnode].value.play, Nodes[search_node].value.play, exp(Nodes[nextnode].value.NN_score) / softmax_sum * 0.10)) :
			chmax(maxvalue, -(double)Nodes[nextnode].value.play)))
		{
			choice_nodenum = nextnode;
		}
	}
	if (choice_nodenum == -1)	//�������Ƃ���Ֆʂ����Ȃ���΃v���C�A�E�g
	{
		return;
	}

	turnnum++;
	used[choice_nodenum] = 1;
	search_node = choice_nodenum;
	nowPlayer = !nowPlayer;
	usenode[turnnum] = search_node;
}
void UCT::search_tree(int& turnnum, NodeNum& search_node, bool& nowPlayer)
{
	while (turnnum < MAXPLAY && !Nodes[search_node].nextnodes.empty() && !Nodes[search_node].finish)	//�؂̒[�܂Ō���
	{
		NodeNum choice_nodenum = -1;
		//double maxvalue = -MAX_VALUE;
		double maxvalue = -MAX_VALUE;

		if (Nodes[search_node].nextnodes.size() == 2)
		{
			NodeNum nx0 = Nodes[search_node].nextnodes[0], nx1 = Nodes[search_node].nextnodes[1];
			if (Nodes[nx0].value.play <= Nodes[nx1].value.play)
			{
				search_node = nx0;
			}
			else
			{
				search_node = nx1;
			}
			useAS[asnum] = search_node;
			asnum++;
		}
		else if (Nodes[search_node].nextnodes.size() == 1)
		{
			search_node = Nodes[search_node].nextnodes[0];
			useAS[asnum] = search_node;
			asnum++;
		}

		for (NodeNum nextnode : Nodes[search_node].nextnodes)
		{
			if (used[nextnode])	//2�x�����Ֆʂ͌��Ȃ�
				continue;
			// �����̎�ԁˑ傫���̂�I��  ����̎�ԁˏ������̂�I��
			if (Nodes[nextnode].value.play == 0)
			{
				choice_nodenum = nextnode;
				break;
			}
			if ((nowPlayer == 0 ?
				chmax(maxvalue, compare(Nodes[nextnode].value.win / 2.0, Nodes[nextnode].value.play, total_play)) :
				chmax(maxvalue, compare(Nodes[nextnode].value.play - Nodes[nextnode].value.win / 2.0, Nodes[nextnode].value.play, total_play))))
			{
				choice_nodenum = nextnode;
			}
		}
		if (choice_nodenum == -1)	//�������Ƃ���Ֆʂ����Ȃ���΃v���C�A�E�g
		{
			break;
		}

		used[choice_nodenum] = 1;
		search_node = choice_nodenum;
		turnnum++;
		usenode[turnnum] = search_node;

		if (Nodes[search_node].nextnodes.size() == 2)
		{
			NodeNum nx0 = Nodes[search_node].nextnodes[0], nx1 = Nodes[search_node].nextnodes[1];
			if (Nodes[nx0].value.play <= Nodes[nx1].value.play)
			{
				search_node = nx0;
			}
			else
			{
				search_node = nx1;
			}
			useAS[asnum] = search_node;
			asnum++;
		}
		else if (Nodes[search_node].nextnodes.size() == 1)
		{
			search_node = Nodes[search_node].nextnodes[0];
			useAS[asnum] = search_node;
			asnum++;
			nowPlayer = !nowPlayer;
		}

		nowPlayer = !nowPlayer;
		//nowPlayer = Nodes[search_node].player;
		
	}//while(!empty())
}

int UCT::expansion(NodeNum search_node, bool nowPlayer, bool useRF)
{
	// �łĂ��̌��𒊏o����
	Board nextboards[32];
	int nextboards_size = 0;
	PossibleNextBoard(nextboards, nowPlayer, Nodes[search_node].board, nextboards_size);
	assert(nextboards_size > 0);

	//Nodes�̊g��
	BitsStatus bsta, bsta_pre;
	double res_pre = Nodes[search_node].value.NN_score;
	toBitsStatus(bsta_pre, Nodes[search_node].board);
	int nx = 0;
	double res_max = -MAX_VALUE;
	for (int i = 0; i < nextboards_size; i++)
	{
		Board nextboard = nextboards[i];
		if (board_index[nextboard] != 0)	//�n�b�V�����ݒ�ς݁i�T���ς݂̃m�[�h�j�͌q���邾��
		{
			Nodes[search_node].nextnodes.push_back(board_index[nextboard]);
			continue;
		}
		board_index[nextboard] = nodenum;	//�V���Ƀn�b�V����ݒ肷��

		Node nextnode;
		nextnode.board = nextboard;
		nextnode.nextnodes.clear();

		//���������Ă��邩
		nextnode.finish = (
			__popcnt64(nextnode.board.en_mix) < 2 ||
			(Red::decided && (__popcnt64(Nodes[search_node].board.enblue) == 0 || __popcnt64(Nodes[search_node].board.enred) == 0)) ||
			nextnode.board.dead_enred == 4 ||
			nextnode.board.dead_myblue == 4 ||
			nextnode.board.dead_myred == 4 ||
			nextnode.board.escape
			);

		if (nowPlayer == 0 && useRF)
		{
			//������z�@�̃��f����ʂ����l��ۑ�
			toBitsStatus(bsta, nextboard);
			nextnode.value.NN_score = getNNres(bsta.TwoPiece, bsta_pre.TwoPiece, res_pre);
			//�ł��l���ǂ���ɂ���
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

	//���̎�
	return Nodes[search_node].nextnodes[nx];
}

//�ł���nowPlayer���ς������
//�G��S�[���O�ɂ���Ƃ��Ɉ��A������������Ƃ��Ɉ��g��
int UCT::expansion_afterstates(NodeNum search_node, bool nowPlayer)
{
	//��������S�[���������ȋ����Ƃ��A���򂵂ĐF���߂���
	BitBoard near_goal = Nodes[search_node].board.en & getNextPosBB(ENGOAL);
	Board nextboards[2] = { Nodes[search_node].board, Nodes[search_node].board };
	int nextboards_size = 2;
	if (near_goal)
	{
		assert(__popcnt64(near_goal) == 1);

		if (Nodes[search_node].board.dead_enblue + __popcnt64(Nodes[search_node].board.enblue) == 4)
		{
			nextboards_size = 1;
			nextboards[0].en ^= near_goal;
			nextboards[0].enred ^= near_goal;
		}
		else if (Nodes[search_node].board.dead_enred + __popcnt64(Nodes[search_node].board.enred) == 4)
		{
			nextboards_size = 1;
			nextboards[0].en ^= near_goal;
			nextboards[0].enblue ^= near_goal;
		}
		else
		{
			nextboards[0].en ^= near_goal;
			nextboards[0].enblue ^= near_goal;
			nextboards[1].en ^= near_goal;
			nextboards[1].enred ^= near_goal;
		}
	}
	else if (Nodes[search_node].board.dead_en == 1)
	{
		if (Nodes[search_node].board.dead_enblue + __popcnt64(Nodes[search_node].board.enblue) == 4)
		{
			nextboards_size = 1;
			nextboards[0].dead_en--;
			nextboards[0].dead_enred++;
		}
		else if (Nodes[search_node].board.dead_enred + __popcnt64(Nodes[search_node].board.enred) == 4)
		{
			nextboards_size = 1;
			nextboards[0].dead_en--;
			nextboards[0].dead_enblue++;
		}
		else
		{
			nextboards[0].dead_en--;
			nextboards[0].dead_enblue++;
			nextboards[1].dead_en--;
			nextboards[1].dead_enred++;
		}
	}
	else
	{
		//return search_node;

		//�����Ȃ���Βʏ��expansion
		return expansion(search_node, nowPlayer, false);
	}

	//Nodes�̊g��
	for (int i = 0; i < nextboards_size; i++)
	{
		Board nextboard = nextboards[i];
		if (board_index[nextboard] != 0)	//�n�b�V�����ݒ�ς݁i�T���ς݂̃m�[�h�j�͌q���邾��
		{
			Nodes[search_node].nextnodes.push_back(board_index[nextboard]);
			continue;
		}
		//else
		board_index[nextboard] = nodenum;	//�V���Ƀn�b�V����ݒ肷��

		Node nextnode;
		nextnode.board = nextboard;
		nextnode.isAS = true;
		nextnode.nextnodes.clear();
		//nextnode.player = nowPlayer;

		Nodes.emplace_back(nextnode);
		Nodes[search_node].nextnodes.push_back(nodenum);
		assert(Nodes[nodenum] == nextnode);
		nodenum++;
	}	//for nextmoves_size

	assert(Nodes[search_node].nextnodes.size() <= 2);
	assert(Nodes[search_node].nextnodes.size() > 0);
	//���̎�
	//�����ς������i�Ԑ̗��Ȃ̂Łj
	return Nodes[search_node].nextnodes[0];
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
	fill(useAS, useAS + 8, 0);
	asnum = 0;

	
	//���[�g�̓W�J
	expansion(search_node, nowPlayer, true);

	clock_t start_time = clock();
	//while (clock() - start_time < (THINKING_TIME - 1) * CLOCKS_PER_SEC)	// UCT�̃��[�v
	while(clock() - start_time < (THINKING_TIME - 1) * CLOCKS_PER_SEC)
	{
		//�؂̒[�܂Ō���
		search_RF(turnnum, search_node, nowPlayer);
		search_tree(turnnum, search_node, nowPlayer);
		if (clock() - start_time > (THINKING_TIME - 1) * CLOCKS_PER_SEC)
		{
			break;
		}

		//�W�J
		if (!Nodes[search_node].finish && Nodes[search_node].value.play >= 1)
		{
			search_node = expansion_afterstates(search_node, nowPlayer);

			if (Nodes[search_node].isAS)
			{
				useAS[asnum] = search_node;
				asnum++;
			}
			else
			{
				//���̎�
				turnnum++;
				usenode[turnnum] = search_node;
				nowPlayer = !nowPlayer;
			}	
		}

		//�v���C�A�E�g�ƃo�b�N�v���p�Q�[�V����
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
			while (asnum > 0)
			{
				asnum--;
				Nodes[useAS[asnum]].value.play++;
				Nodes[useAS[asnum]].value.win += reward;
			}
			//�T���������Z�b�g
			//usenode.resize(1, playnodenum);
			search_node = playnodenum;
			nowPlayer = 0;
			turnnum = playnum;
			used.clear();
			asnum = 0;
		}

		
	}	// while(true)	UCT�̃��[�v

}	// Search()

//�؂�"�e�m�[�h","�q�m�[�h"������ƁA�J�ڎ���MoveCommand��Ԃ�
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
		// NN : 1%
		if (Nodes[nextnode].value.play == 0) continue;
		//if (chmax(maxvalue, (double)Nodes[nextnode].value.win / Nodes[nextnode].value.play))
		//if (chmax(maxvalue, (double)Nodes[nextnode].value.win / 2.0 / Nodes[nextnode].value.play))
		if (chmax(maxvalue, (double)Nodes[nextnode].value.win / Nodes[nextnode].value.play + exp(Nodes[nextnode].value.NN_score) / softmax * 0.1))
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
		printf("Value:%8d", Nodes[nextnode].value.win);
		printf("\tPlay:%8d", Nodes[nextnode].value.play);
		printf("\tComp:%.6f", (Nodes[nextnode].value.play == 0 ? 0 : (double)Nodes[nextnode].value.win / Nodes[nextnode].value.play + exp(Nodes[nextnode].value.NN_score) / softmax * 0.1));
		printf("\tNNScore:%.6f", exp(Nodes[nextnode].value.NN_score) / softmax);
		printf("\n");
	}
	cout << "} Next {" << endl;
	if (Nodes[choice_nodenum].nextnodes.empty())
	{
		cout << "Empty!!" << endl;
	}
	for (NodeNum nextnode : Nodes[choice_nodenum].nextnodes)
	{
		cout << "\t";
		cout << "Value:" << Nodes[nextnode].value.win << " Play:" << Nodes[nextnode].value.play << endl;
	}
	cout << "}\nChoose" << endl;
	cout << "Value:" << Nodes[choice_nodenum].value.win << " Play:" << Nodes[choice_nodenum].value.play;
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
