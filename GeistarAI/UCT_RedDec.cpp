#include<string>
#include<algorithm>
#include<time.h>
#include<cassert>

#include "types.h"
#include "method.h"
#include "UCT_RedDec.h"
#include "ColorGuess.h"
#include "Game.h"

using namespace std;

using namespace UCT_RedDec;


//������
UCT::UCT(Recieve str)
{
	mt.seed(rnd());
	for (int i = 1; i <= 64; i++)
	{
		uniform_int_distribution<>::param_type param(0, i - 1);
		rand[i].param(param);
	}

	// ���[�g�m�[�h������
	Nodes.resize(2);	//map�̊֌W��0�Ԗڂ͋�ɂ���
	Nodes[1].board = toBoard(str);
	Nodes[1].value = { 0, 0 };
	Nodes[1].nextnodes.clear();
	Nodes[1].finish = 0;

	nodenum = 2;
	total_play = 0;
	playnodenum = 1;
	playnum = 0;
	//red_rate.resize(8, 0);

	//�ԋ�����߂�
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
}

// ���ۂ̔Ֆʂ̃m�[�h�Ɉړ�
void UCT::SetNode(Recieve str)
{
	playnum = Game_::turn;	//�^�[������i�߂�

	// �T���؂��\�z���Ȃ���
	Nodes.resize(2);
	Nodes[1].board = toBoard(str);
	Nodes[1].value = { 0, 0 };
	Nodes[1].nextnodes.clear();
	Nodes[1].finish = 0;

	//�ԋ��ݒ肷��
	if (onPiece(Nodes[1].board.en, red_pos))
		blue_pos = Nodes[1].board.en ^ red_pos;
	else
		red_pos = Nodes[1].board.en ^ blue_pos;
	Nodes[1].board.enblue = blue_pos;
	Nodes[1].board.enred = red_pos;
	Nodes[1].board.en = 0;
	assert(Nodes[1].board.enblue ^ Nodes[1].board.enred == Nodes[1].board.en_mix);
	cout << "�ԓǂ�" << endl;
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
}


//UCT���̃v���C�A�E�g������֐�
//double UCT::playout(bool nowPlayer, int turnnum, Board nowboard)
int UCT::playout(bool nowPlayer, int turnnum, Board nowboard)
{

	while (true)
	{
		if (clock() - start_time > (THINKING_TIME - 1) * CLOCKS_PER_SEC)
		{
			return DRAW_VALUE;
		}
		//���������Ă��邩����
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
			ppos = getBottomBB(ppos);
			npos = getNextPosBB(ppos) & ENGOAL;
			assert(__popcnt64(npos) == 1);
		}
		//�ʏ�̓���
		else
		{
			ppos = (nowPlayer == 0 ? nowboard.my_mix : nowboard.en_mix);
			//npos = Inside(getNextPosBB(ppos) & ~ppos);	//�ړ�����𒊏o
			npos = Inside(getNextPosBB(ppos) & ~ppos & ~nowboard.enred);	//�ړ�����𒊏o
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

//��
int UCT::playout_P(bool nowPlayer, int turnnum, Board nowboard)
{
	if (nowboard.enblue == 0)
		return WIN_VALUE;
	if (nowboard.enred == 0)
		return LOSE_VALUE;

	nowboard.en |= nowboard.enblue | nowboard.enred;
	nowboard.enblue = nowboard.enred = 0;
	while (true)
	{
		if (clock() - start_time > (THINKING_TIME - 1) * CLOCKS_PER_SEC)
		{
			return DRAW_VALUE;
		}
		//���������Ă��邩����
		if (turnnum > MAXPLAY)
			return DRAW_VALUE;
		if (nowboard.escape)
			return (nowPlayer == 0 ? LOSE_VALUE : WIN_VALUE);
		if (nowboard.dead_myblue == 4)
			return LOSE_VALUE;
		if (nowboard.dead_en + nowboard.dead_enred >= 4)
			return LOSE_VALUE;
		if (nowboard.dead_myred == 4)
			return WIN_VALUE;
		assert(__popcnt64(nowboard.en_mix) >= 2);
		assert(nowboard.en_mix == nowboard.en);


		//�ς��Ă݂�
		BitBoard ppos = 0, npos = 0;
		//�S�[���ł�����S�[������
		if (nowPlayer == 0 && Goal(getNextPosBB(nowboard.myblue), 0))
		{
			/*
			ppos = getNextPosBB(MYGOAL) & nowboard.myblue;
			ppos = getBottomBB(ppos);
			npos = getNextPosBB(ppos) & MYGOAL;
			assert(__popcnt64(npos) == 1);
			*/
			return WIN_VALUE;
		}
		else if (nowPlayer == 1 && Goal(getNextPosBB(nowboard.en), 1))
		{
			/*
			ppos = getNextPosBB(ENGOAL) & nowboard.en;
			ppos = getBottomBB(ppos);
			npos = getNextPosBB(ppos) & ENGOAL;
			assert(__popcnt64(npos) == 1);
			*/
			return LOSE_VALUE;
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
		toNextBoard(nowboard, ppos, npos, nowPlayer);


		//�^�[����i�߂�
		nowPlayer = !nowPlayer;
		turnnum++;
	} // while(true)
}

void UCT::search_tree(int& turnnum, int& search_node, bool& nowPlayer)
{
	while (turnnum < MAXPLAY && !Nodes[search_node].nextnodes.empty())	//�؂̒[�܂Ō���
	{
		if (clock() - start_time > (THINKING_TIME - 1) * CLOCKS_PER_SEC)
		{
			break;
		}
		NodeNum choice_nodenum = -1;
		double maxvalue = -MAX_VALUE, minvalue = MAX_VALUE;
		//int maxvalue = -MAX_VALUE, minvalue = MAX_VALUE;
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
				chmax(maxvalue, compare(Nodes[nextnode].value.win, Nodes[nextnode].value.play, total_play)) :
				chmax(maxvalue, compare(-Nodes[nextnode].value.win, Nodes[nextnode].value.play, total_play))))
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
			nextnode.board.enblue == 0 ||
			nextnode.board.enred == 0 ||
			nextnode.board.dead_myblue == 4 ||
			nextnode.board.dead_myred == 4 ||
			nextnode.board.escape
			);

		Nodes.emplace_back(nextnode);
		Nodes[search_node].nextnodes.push_back(nodenum);
		assert(Nodes[nodenum] == nextnode);
		nodenum++;
	}	//for nextmoves_size

	//���̎�
	return Nodes[search_node].nextnodes[0];
}

//UCT�T��������
void UCT::Search(int playout_type)
{
	//���O��SetNode(str)�����Ă���
	NodeNum search_node = playnodenum;
	bool nowPlayer = 0; // 0:����  1:����
	int turnnum = playnum;
	fill(usenode, usenode + 302, 0);
	usenode[playnum] = playnodenum;


	//���[�g�̓W�J
	expansion(search_node, nowPlayer);

	start_time = clock();
	while (clock() - start_time < (THINKING_TIME - 1) * CLOCKS_PER_SEC)	// UCT�̃��[�v
	{
		//�؂̒[�܂Ō���
		search_tree(turnnum, search_node, nowPlayer);
		if (clock() - start_time > (THINKING_TIME - 1) * CLOCKS_PER_SEC)
		{
			break;
		}

		//�W�J
		if (!Nodes[search_node].finish && Nodes[search_node].value.play >= 1)
		{
			search_node = expansion(search_node, nowPlayer);
			//���̎�
			turnnum++;
			//assert(!Nodes[search_node].nextnodes.empty());
			nowPlayer = !nowPlayer;
			usenode[turnnum] = search_node;
		}

		//�v���C�A�E�g�ƃo�b�N�v���p�Q�[�V����
		{
			//double reward = playout(nowPlayer, turnnum, Nodes[search_node].board);
			int reward = 0;
			if(playout_type == 0)
				reward = playout(nowPlayer, turnnum, Nodes[search_node].board);
			else if(playout_type == 1)
				reward = playout_P(nowPlayer, turnnum, Nodes[search_node].board);

			//backpropagation();
			total_play++;
			while (turnnum >= playnum)
			{
				Nodes[usenode[turnnum]].value.play++;
				Nodes[usenode[turnnum]].value.win += reward;
				turnnum--;
			}
			//�T���������Z�b�g
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

//MoveNode()�̑O��
void UCT::changeRed_pos(NodeNum to)
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

//root�m�[�h����ǂ̎��I��Ŏw�����B�m�[�h�̓Y������Ԃ�
NodeNum UCT::Choice()
{
	assert(playnodenum < Nodes.size());
	assert(!Nodes[playnodenum].nextnodes.empty());
	NodeNum choice_nodenum = -1;
	//double maxvalue = -MAX_VALUE;
	int maxvalue = 0;
	for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
	{
		// �����̎�ԁˑ傫���̂�I��
		if (Nodes[nextnode].value.play == 0) continue;
		//if (chmax(maxvalue, (double)Nodes[nextnode].value.win / Nodes[nextnode].value.play))
		if (chmax(maxvalue, Nodes[nextnode].value.play))
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
		cout << "Value:" << Nodes[nextnode].value.win << " Play:" << Nodes[nextnode].value.play << " Comp:" << intcompare(Nodes[nextnode].value.win, Nodes[nextnode].value.play, total_play) << endl;
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
	Send send = toSend(move, Game_::piecenum);
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
