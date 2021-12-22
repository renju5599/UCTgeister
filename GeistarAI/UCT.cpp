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

#include "types.h"
#include "method.h"
#include "UCT.h"

using namespace std;


//�ړ��O�̈ʒuppos�ƈړ���̈ʒunpos���� board���X�V����
void toNextBoard(Board& board, BitBoard ppos, BitBoard npos, bool nowPlayer)
{
	if (Goal(npos, nowPlayer))	//�E�o����
		board.escape = true;
	else	//�E�o���ĂȂ�
		board.escape = false;

	if (nowPlayer == 0)
	{
		if (onPiece(board.myblue, ppos))	//�𓮂�����
			change(board.myblue, ppos, npos);	//��̈ړ�
		else	//�Ԃ𓮂�����
			change(board.myred, ppos, npos);

		//��������
		if (onPiece(board.enblue, npos))
		{
			board.enemy ^= npos;
			board.enblue ^= npos;
			board.dead_enblue <<= 1;
			board.kill = true;
		}
		//�ԋ�������
		else if (onPiece(board.enred, npos))
		{
			board.enemy ^= npos;
			board.enred ^= npos;
			board.dead_enred <<= 1;
			board.kill = true;
		}
		else if(onPiece(board.enemy, npos))
		{
			board.enemy ^= npos;	//�G����������̏�Ԃ�
			board.kill = true;
		}
		else	//�G������Ȃ�����
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

		if (onPiece(board.myblue, npos))	//������ꂽ
		{
			board.myblue ^= npos;
			board.kill = true;
			board.dead_myblue <<= 1;
		}
		else if (onPiece(board.myred, npos))	//�ԋ�����ꂽ
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

// ���ׂĂ̍��@���̔Ֆʂ�vector�ɓ����
// �G��̐F����͂��Ȃ�
void PossibleNextBoard(vector<Board>& nextpositions, bool nowPlayer, Board board)
{
	BitBoard nowmy = board.myblue | board.myred;

	//getBottomBB()��offBottomBB()���g���č����ɒ��ׂ�悤�ɂ�����
	if (nowPlayer == 0)
	{
		// �ǂ̈ʒu�̋�𓮂�������for
		for (BitBoard piecebb = nowmy; piecebb != 0; offBottomBB(piecebb))
		{
			BitBoard ppos = getBottomBB(piecebb);
			assert(ppos != 0);
			// �ړ�4����������for
			for (BitBoard nextbb = getNextPosBB(ppos); nextbb != 0; offBottomBB(nextbb))
			{
				BitBoard npos = getBottomBB(nextbb);
				assert(npos != 0);

				//���@��������
				if (!Inside(npos) && !Goal(npos, nowPlayer))	//�ՖʊO
					continue;
				if (Goal(npos, nowPlayer) && onPiece(board.myred, ppos))	//�ԋ�E�o���悤�Ƃ���
					continue;
				if (onPiece(nowmy, npos))	//���g�̋�d�Ȃ�
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
				if (!Inside(npos) && !Goal(npos, nowPlayer))	//�ՖʊO
					continue;
				if (Goal(npos, nowPlayer) && onPiece(board.enred, ppos))	//�ԋ�E�o���悤�Ƃ���
					continue;
				if (onPiece(board.enemy, npos))	//���g�̋�d�Ȃ�
					continue;

				Board nextboard = board;
				toNextBoard(nextboard, ppos, npos, nowPlayer);
				nextpositions.push_back(nextboard);

			}
		}

	} //nowPlayer == 1

	return;
}


//������
UCT::UCT(Recieve str)
{
	// ���[�g�m�[�h������
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

//UCT�ŗv��Ȃ��Ȃ����m�[�h���i�����Ɓj����
//�Ӗ����Ȃ������Ȃ̂ŏ���
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

// ���ۂ̔Ֆʂ̃m�[�h�Ɉړ�
void UCT::SetNode(Recieve str)
{
	playnum++;	//�^�[��������i�߂�

	// �T���؂Ƀm�[�h��������΍\�z���Ȃ���
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
				total_play = Nodes[nextnode].value.play;
				playnodenum = nextnode;
				break;
			}
		}
		assert(Nodes[playnodenum].board == nowboard);
	}
}

//UCT�̊e�m�[�h�̉��l(?)���v�Z�E�ۑ�����
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

//UCT���̃v���C�A�E�g������֐�
int UCT::playout(bool nowPlayer, int playoutnum, Board nowboard)
{
	int bluenum = livenum(nowboard.dead_enblue) - __popcnt64(nowboard.enblue);	//�킩���Ă��Ȃ��̐�
	int rednum = livenum(nowboard.dead_enred) - __popcnt64(nowboard.enred);	//�킩���Ă��Ȃ��Ԃ̐�
	int undefinenum = bluenum + rednum;

	int ennum = __popcnt64(nowboard.enemy);	//�����Ă���G�̐�
	
	//�|����Ă���G���F���߂���
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

	//�Ֆʏ�̓G���F���߂���
	while (bluenum--)
	{
		int r = rand() % undefinenum;
		BitBoard bb = nowboard.enemy ^ nowboard.enblue ^ nowboard.enred;
		while (r--)
		{
			offBottomBB(bb);
		}
		bb = getBottomBB(bb);
		nowboard.enblue |= bb;
		undefinenum--;
	}
	assert(undefinenum == rednum);
	nowboard.enred = nowboard.enemy ^ nowboard.enblue;

	//���łɏ��������Ă��邩����
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
		//�������x����
		vector<Board> nextboards;
		PossibleNextBoard(nextboards, nowPlayer, nowboard);
		int nextmoves_size = nextboards.size();
		assert(nextmoves_size > 0);
		int r = rand() % nextmoves_size;
		nowboard = nextboards[r];	//�����_���Ɏ��̎�����߂�
		
		//�ς��Ă݂�
		//�S�[�����肪�ł��ĂȂ�
		//BitBoard ppos = (nowPlayer == 0 ? (nowboard.myblue | nowboard.myred) : nowboard.enemy);
		//BitBoard npos = Inside(getNextPosBB(ppos) ^ ppos);	//�ړ�����𒊏o
		//assert(ppos != 0);
		//assert(npos != 0);
		////�ړ���������_���ɑI��
		//int r = rand() % __popcnt64(npos);
		//while (r--)
		//	offBottomBB(npos);
		//npos = getBottomBB(npos);
		////�I�����ꂽ�ړ���ɍs�����̒��Ń����_���ɑI��
		//ppos &= getNextPosBB(npos);
		//assert(ppos != 0);
		//r = rand() % __popcnt64(ppos);
		//while (r--)
		//	offBottomBB(ppos);
		//ppos = getBottomBB(ppos);
		////���߂��s����board�ɕϊ�����
		//toNextBoard(nowboard, ppos, npos, nowPlayer);
		//

		//����������������
		if (playoutnum > MAXPLAY)
			return DRAW_VALUE;
		if (nowboard.escape)
			return (nowPlayer == 0 ? WIN_VALUE : LOSE_VALUE);
		if (nowboard.dead_myblue == 16 || nowboard.dead_enred == 16)
			return LOSE_VALUE;
		if (nowboard.dead_myred == 16 || nowboard.dead_enblue == 16)
			return WIN_VALUE;
		assert(__popcnt64(nowboard.enemy) >= 2);
		assert(nowboard.enemy == (nowboard.enblue ^ nowboard.enred));

		//�^�[����i�߂�
		nowPlayer = !nowPlayer;
		playoutnum++;
	} // while(true)
}

//UCT�T��������
void UCT::Search()
{
	//���O��SetNode(str)�����Ă���
	NodeNum search_node = playnodenum;
	vector<NodeNum> usenode(1, playnodenum);
	bool nowPlayer = 0; // 0:����  1:����
	int playoutnum = playnum;

	clock_t start_time = clock();
	while (clock() - start_time < 1.0 * CLOCKS_PER_SEC)	// UCT�̃��[�v
	{

		playoutnum++;

		// �łĂ��̌��𒊏o����
		vector<Board> nextboards;
		PossibleNextBoard(nextboards, nowPlayer, Nodes[search_node].board);
		int nextboards_size = nextboards.size();
		assert(nextboards_size > 0);

		if (Nodes[search_node].nextnodes.empty())	//�t�m�[�h
		{

			if (!Nodes[search_node].finish && Nodes[search_node].value.play >= 3)	//�W�J
			{
				//Nodes�̊g��
				for (int i = 0; i < nextboards_size; i++)
				{
					Node nextnode;
					nextnode.board = nextboards[i];
					nextnode.nextnodes.resize(0);

					//���������Ă��邩
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
			}	// if �W�J

			else	//�v���C�A�E�g�ƃo�b�N�v���p�Q�[�V����
			{
				//playout();
				int reward = playout(nowPlayer, playoutnum, Nodes[search_node].board);

				//backpropagation();
				total_play++;
				for (NodeNum node : usenode)
				{
					Nodes[node].value.play++;
					Nodes[node].value.win += reward;
					UpdateBoardValue(Nodes[node].value);
				}
				//�T���������Z�b�g
				usenode.resize(1, playnodenum);
				search_node = playnodenum;
				nowPlayer = 0;
				playoutnum = playnum;
			}	// else
		} // if �t�m�[�h

		else	//�؂̒T��
		{
			NodeNum choice_nodenum = Nodes[search_node].nextnodes[0];
			NodeValue maxvalue = Nodes[choice_nodenum].value;
			for (NodeNum nextnode : Nodes[search_node].nextnodes)
			{
				// �����̎�ԁˑ傫���̂�I��  ����̎�ԁˏ������̂�I��
				if (Nodes[nextnode].value.comp == MAX_VALUE)
				{
					choice_nodenum = nextnode;
					break;
				}
				if ((nowPlayer == 0 ? chmax(maxvalue, Nodes[nextnode].value) : chmin(maxvalue, Nodes[nextnode].value)))
				{
					choice_nodenum = nextnode;
				}
			}

			search_node = choice_nodenum;
			nowPlayer = !nowPlayer;
			usenode.push_back(search_node);
		}
	}	// while(true)	UCT�̃��[�v


}	// Search()

//�؂�"�e�m�[�h","�q�m�[�h"������ƁA�J�ڎ���MoveCommand��Ԃ�
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

//root�m�[�h����ǂ̎��I��Ŏw�����B�m�[�h�̓Y������Ԃ�
NodeNum UCT::Choice()
{
	assert(playnodenum < Nodes.size());
	assert(!Nodes[playnodenum].nextnodes.empty());
	NodeNum choice_nodenum = Nodes[playnodenum].nextnodes[0];
	double maxvalue = (double)Nodes[choice_nodenum].value.win / Nodes[choice_nodenum].value.play;
	for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
	{
		// �����̎�ԁˑ傫���̂�I��
		if (chmax(maxvalue, (double)Nodes[nextnode].value.win / Nodes[nextnode].value.play))
		{
			choice_nodenum = nextnode;
		}
	}
	assert(playnodenum != choice_nodenum);
	cout << Nodes[choice_nodenum].value.win << " " << Nodes[choice_nodenum].value.play << endl;
	return choice_nodenum;
}

//�m�[�h��J�ڂ��āA�T�[�o�[�ɑ��镶�����Ԃ��i�����Ă͂Ȃ��j
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
