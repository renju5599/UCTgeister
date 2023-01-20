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
	Nodes[1].value = { 0, 0 };
	Nodes[1].nextnodes.clear();
	//Nodes[1].player = 0;
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
	
	// ���肪�łO�̃m�[�h���T���؂ɖ�����΍\�z���Ȃ���
	//if (Nodes[playnodenum].nextnodes.empty())
	if(true)
	{
		Nodes.resize(2);
		Nodes[1].board = toBoard(str);
		Nodes[1].value = { 0, 0 };
		Nodes[1].nextnodes.clear();
		//Nodes[1].player = 0;
		Nodes[1].finish = 0;

		nodenum = 2;
		total_play = 0;
		playnodenum = 1;

		board_index.clear();
	}
	else
	{
		Board nowboard = toBoard(str);
		if (nowboard == Nodes[playnodenum].board)	// ????
			return;
		for (const NodeNum& nextnode : Nodes[playnodenum].nextnodes)
		{
			if (Nodes[nextnode].board == nowboard)	//������z�u
			{
				playnodenum = nextnode;
				total_play = Nodes[nextnode].value.play;
				return;
			}
		}
		for (const NodeNum& nx : Nodes[playnodenum].nextnodes)
		{
			for (const NodeNum& nnx : Nodes[nx].nextnodes)
			{
				if (Nodes[nx].player != Nodes[nnx].player)
				{

				}
				else
				{
					for (const NodeNum& nnnx : Nodes[nnx].nextnodes)
					{

					}
				}
			}
		}

	}

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
	int undefinenum = bluenum + rednum;
	int ennum = __popcnt64(nowboard.en_mix);	//�����Ă���G�̐�

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
	nowboard.enred = nowboard.en;
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

void UCT::search_tree(int& turnnum, NodeNum& search_node, bool& nowPlayer)
{
	while (turnnum < MAXPLAY && !Nodes[search_node].nextnodes.empty() && !Nodes[search_node].finish)	//�؂̒[�܂Ō���
	{
		NodeNum choice_nodenum = -1;
		//double maxvalue = -MAX_VALUE;
		int maxvalue = -MAX_VALUE;

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
				chmax(maxvalue, intcompare(Nodes[nextnode].value.win / 2.0, Nodes[nextnode].value.play, total_play)) :
				chmax(maxvalue, intcompare(Nodes[nextnode].value.play - Nodes[nextnode].value.win / 2.0, Nodes[nextnode].value.play, total_play))))
			{
				choice_nodenum = nextnode;
			}
		}
		if (choice_nodenum == -1)	//�������Ƃ���Ֆʂ����Ȃ���΃v���C�A�E�g
		{
			assert(Nodes[search_node].board.dead_en == 0);
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

		nowPlayer = !nowPlayer;
		//nowPlayer = Nodes[search_node].player;
		
	}//while(!empty())
}

int UCT::expansion(NodeNum search_node, bool nowPlayer)
{
	// �łĂ��̌��𒊏o����
	Board nextboards[32];
	int nextboards_size = 0;
	PossibleNextBoard_quick(nextboards, nowPlayer, Nodes[search_node].board, nextboards_size);
	assert(nextboards_size > 0);
	assert(Nodes[search_node].board.dead_en == 0);

	//Nodes�̊g��
	for (int i = 0; i < nextboards_size; i++)
	{
		Board nextboard = nextboards[i];
		//nextboard.player = !nowPlayer;
		if (board_index[nextboard] != 0)	//�n�b�V�����ݒ�ς݁i�T���ς݂̃m�[�h�j�͌q���邾��
		{
			assert(nextboard == Nodes[board_index[nextboard]].board);
			Nodes[search_node].nextnodes.push_back(board_index[nextboard]);
			continue;
		}
		board_index[nextboard] = nodenum;	//�V���Ƀn�b�V����ݒ肷��

		Node nextnode;
		nextnode.board = nextboard;
		nextnode.isAS = false;
		nextnode.nextnodes.clear();
		//nextnode.player = !nowPlayer;

		//���������Ă��邩
		nextnode.finish = (
			__popcnt64(nextnode.board.en_mix) < 2 ||
			(Red::decided && (__popcnt64(Nodes[search_node].board.enblue) == 0 || __popcnt64(Nodes[search_node].board.enred) == 0)) ||
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

	//���̎�
	return Nodes[search_node].nextnodes[0];
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

		nextboards[0].en ^= near_goal;
		nextboards[0].enblue ^= near_goal;
		nextboards[1].en ^= near_goal;
		nextboards[1].enred ^= near_goal;
	}
	else if (Nodes[search_node].board.dead_en == 1)
	{
		nextboards[0].dead_en = 0;
		nextboards[0].dead_enblue++;
		nextboards[1].dead_en = 0;
		nextboards[1].dead_enred++;
	}
	else
	{
		//return search_node;

		//�����Ȃ���Βʏ��expansion
		return expansion(search_node, nowPlayer);
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

	assert(Nodes[search_node].nextnodes.size() == 2);
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
	search_node = expansion(search_node, nowPlayer);

	clock_t start_time = clock();
	//while (clock() - start_time < (THINKING_TIME - 1) * CLOCKS_PER_SEC)	// UCT�̃��[�v
	while(total_play < 100000)
	{
		//�؂̒[�܂Ō���
		search_tree(turnnum, search_node, nowPlayer);

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
	for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
	{
		// �����̎�ԁˑ傫���̂�I��
		// NN : 1%
		if (Nodes[nextnode].value.play == 0) continue;
		//if (chmax(maxvalue, (double)Nodes[nextnode].value.win / Nodes[nextnode].value.play))
		if (chmax(maxvalue, (double)Nodes[nextnode].value.win / 2.0 / Nodes[nextnode].value.play))
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
