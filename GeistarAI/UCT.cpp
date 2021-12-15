//////////////////////
//todo
//
// �R�[�h�����G
// �������m��Ȃ�
// 
// 
// �������x���I�I�I
// �˃r�b�g�{�[�h���p���G�I���W�ۑ��ˑS���r�b�g�{�[�h�ɕς��Ă��ǂ������i���o�͈ȊO�ŕs�v�j
// �V�t�g���Z�̉񐔂����炵����
// 
// (���E�G)��̗L����8bit�ŕ\��
// 01234567
// 10010111		...5��c���Ă邱�Ƃ��ȒP�ɂ킩��
// __popcnt()���g����
// 
// �L����Move��32bit�ŕ\��	(vector�ɓ���Ă��̂͒x��)
// 0    1    2    3...
// NEWS NEWS NEWS...
// 0111 0110 1111...		...�v���C�A�E�g�Ɏg����(ON�̐��ŗ�������N�Ԗڂ�ON�̓���������)
// 
// �o�͂����W�ōς�...�Ƃ��v�������ǈ�����B���W�Ǘ��͏o�͂ŕK�v�B
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


void PossibleMoves(vector<MoveCommand>& nextmoves, bool nowPlayer, Board board)
{
	BitBoard nowmyblue = board.myblue;
	BitBoard nowmyred = board.myred;
	BitBoard nowmy = nowmyblue | nowmyred;
	BitBoard nowen = board.enemy;
	for (Point i = 9; i < 55; i++)	//����1�}�X�͒T���s�v�Ȃ��ߎG�ɃJ�b�g
	{

		for (uint8_t dir = 0; dir < 4; dir++)
		{

			MoveCommand move = { i, dir };
			if (nowPlayer == 0 && onPiece(nowmy, i))
			{
				Point pos = nextpos(move);
				if (Outside(pos) && !Goal(pos, nowPlayer))
					continue;
				if (Goal(pos, nowPlayer) && onPiece(nowmyred, i))
					continue;
				if (onPiece(nowmy, pos))	//���g�̋�d�Ȃ�
					continue;

				nextmoves.push_back(move);
			}
			if (nowPlayer == 1 && onPiece(nowen, i))
			{
				for (uint8_t dir = 0; dir < 4; dir++)
				{
					Point pos = nextpos(move);
					if (Outside(pos) && !Goal(pos, nowPlayer))
						continue;
					if (onPiece(nowen, pos))	//���g�̋�d�Ȃ�
						continue;

					nextmoves.push_back(move);
				}
			}
		}
	}
	return;
}

void PossibleNextBoard(vector<Board>& nextpositions, bool nowPlayer, Board board)
{
	BitBoard nowmyblue = board.myblue;
	BitBoard nowmyred = board.myred;
	BitBoard nowmy = nowmyblue | nowmyred;
	BitBoard nowen = board.enemy;

	//getBottomBB()��offBottomBB()���g���č����ɒ��ׂ�悤�ɂ�����
	if (nowPlayer == 0)
	{
		for (BitBoard piecebb = nowmy; piecebb != 0; piecebb = offBottomBB(piecebb))
		{
			BitBoard ppos = getBottomBB(piecebb);
			for (BitBoard nextbb = getNextPosBB(ppos); nextbb != 0; nextbb = offBottomBB(nextbb))
			{
				BitBoard npos = getBottomBB(nextbb);
				if (Outside(npos) && !Goal(npos, nowPlayer))
					continue;
				if (Goal(npos, nowPlayer) && onPiece(nowmyred, npos))
					continue;
				if (onPiece(nowmy, npos))	//���g�̋�d�Ȃ�
					continue;

				if (onPiece(nowmyblue, ppos))
				{
					board.myblue = change(nowmyblue, ppos, npos);
					nextpositions.push_back(board);
					board.myblue = nowmyblue;
				}
				else
				{
					board.myred = change(nowmyred, ppos, npos);
					nextpositions.push_back(board);
					board.myred = nowmyred;
				}
			}
		}
	}
	else
	{
		for (BitBoard piecebb = nowen; piecebb != 0; piecebb = offBottomBB(piecebb))
		{
			BitBoard ppos = getBottomBB(piecebb);
			for (BitBoard nextbb = getNextPosBB(ppos); nextbb != 0; nextbb = offBottomBB(nextbb))
			{
				BitBoard npos = getBottomBB(nextbb);
				if (Outside(npos) && !Goal(npos, nowPlayer))
					continue;
				if (onPiece(nowen, npos))	//���g�̋�d�Ȃ�
					continue;

				board.enemy = change(nowen, ppos, npos);
				nextpositions.push_back(board);
				board.enemy = nowen;
			}
		}
	}

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
		nodenum = 1;
		total_play = 0;
		playnodenum = 0;
		pieces = toPieces(str);
	}
	else
	{
		Board nowboard = toBoard(str);
		if (nowboard == Nodes[playnodenum].board)
			return;
		for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
		{
			if (Nodes[nextnode].board == nowboard)
			{
				//�s�v�ȃm�[�h������
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

//UCT���̃v���C�A�E�g������֐�
int UCT::playout(bool nowPlayer, int playoutnum, Board nowboard)
{

	while (true)
	{
		if (playoutnum > MAXPLAY)
		{
			return 1;
		}
		if (nowboard.escape)
		{
			return (nowPlayer == 0 ? 0 : 3);	//�������̃^�[���̘b�Ȃ̂ŉ��l���t
		}
		if (nowboard.dead_myblue == 4 || nowboard.dead_enred == 4)
		{
			return 0;
		}
		if (nowboard.dead_myred == 4 || nowboard.dead_enblue == 4)
		{
			return 3;
		}

		vector<Board> nextboards;
		PossibleNextBoard(nextboards, nowPlayer, nowboard);
		int nextmoves_size = nextboards.size();
		assert(nextmoves_size > 0);
		int r = rand() % nextmoves_size;
		Board nextboard = nextboards[r];
		if (nowPlayer == 0)
		{
			if (onPiece(nextboard.enemy, (nextboard.myblue | nextboard.myred)))
			{
				nextboard.enemy &= ~(nextboard.myblue | nextboard.myred);
				nextboard.kill = true;

				int r2 = rand() % 2;
				if (r2 == 0)
					nextboard.dead_enblue++;
				else
					nextboard.dead_enred++;
			}
			if (Goal(nextboard.myblue, nowPlayer))
			{
				nextboard.escape = true;
			}
		}
		else
		{
			if (onPiece(nextboard.myblue, nextboard.enemy))
			{
				nextboard.myblue &= ~nextboard.enemy;
				nextboard.kill = true;
				nextboard.dead_myblue++;
			}
			if (onPiece(nextboard.myred, nextboard.enemy))
			{
				nextboard.myred &= ~nextboard.enemy;
				nextboard.kill = true;
				nextboard.dead_myred++;
			}

			if (Goal(nextboard.enemy, nowPlayer))
			{
				nextboard.escape = true;
			}
		}

		nowboard = nextboard;
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

	clock_t start_time = clock();
	while (clock() - start_time < 3.0 * CLOCKS_PER_SEC)	// UCT�̃��[�v
	{

		// �łĂ��̌��𒊏o����
		vector<Board> nextboards;
		PossibleNextBoard(nextboards, nowPlayer, Nodes[search_node].board);
		int nextboards_size = nextboards.size();
		assert(nextboards_size > 0);

		if (Nodes[search_node].nextnodes.empty())	//�t�m�[�h
		{
			if (Nodes[search_node].value.play >= 3)	//�W�J
			{
				//Nodes�̊g��
				for (int i = 0; i < nextboards_size; i++)
				{
					Board nextboard = nextboards[i];
					if (nowPlayer == 0)
					{
						if (onPiece(nextboard.enemy, (nextboard.myblue | nextboard.myred)))
						{
							nextboard.enemy &= ~(nextboard.myblue | nextboard.myred);
							nextboard.kill = true;

							int r2 = rand() % 2;
							if (r2 == 0)
								nextboard.dead_enblue++;
							else
								nextboard.dead_enred++;
						}
						if (Goal(nextboard.myblue, nowPlayer))
						{
							nextboard.escape = true;
						}
					}
					else
					{
						if (onPiece(nextboard.myblue, nextboard.enemy))
						{
							nextboard.myblue &= ~nextboard.enemy;
							nextboard.kill = true;
							nextboard.dead_myblue++;
						}
						if (onPiece(nextboard.myred, nextboard.enemy))
						{
							nextboard.myred &= ~nextboard.enemy;
							nextboard.kill = true;
							nextboard.dead_myred++;
						}

						if (Goal(nextboard.enemy, nowPlayer))
						{
							nextboard.escape = true;
						}
					}
					Node nextnode;
					nextnode.board = nextboard;
					nextnode.nextnodes.resize(0);

					Nodes.push_back(nextnode);
					Nodes[search_node].nextnodes.push_back(nodenum);
					assert(nodenum == Nodes[search_node].nextnodes.back());
					assert(Nodes[nodenum] == nextnode);
					nodenum++;
				}	//for nextmoves_size


				assert(!Nodes[search_node].nextnodes.empty());
				NodeNum r = rand() % Nodes[search_node].nextnodes.size();
				search_node = Nodes[search_node].nextnodes[r];
				nowPlayer = !nowPlayer;
				usenode.push_back(search_node);

			}	// if �W�J

			else	//�v���C�A�E�g�ƃo�b�N�v���p�Q�[�V����
			{
				//playout();
				int reward = playout(nowPlayer, playnum, Nodes[search_node].board);	//����:3  ����:1  ����:0�@(������)

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

			}	// else
		} // if �t�m�[�h

		else	//�؂̒T��
		{
			NodeNum choice_nodenum = Nodes[search_node].nextnodes[0];
			NodeValue maxvalue = Nodes[choice_nodenum].value;
			for (NodeNum nextnode : Nodes[search_node].nextnodes)
			{
				// �����̎�ԁˑ傫���̂�I��  ����̎�ԁˏ������̂�I��
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
	assert(npos != 0);
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
	NodeValue maxvalue = Nodes[choice_nodenum].value;
	for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
	{
		// �����̎�ԁˑ傫���̂�I��  ����̎�ԁˏ������̂�I��
		if (chmax(maxvalue, Nodes[nextnode].value))
		{
			choice_nodenum = nextnode;
		}
	}
	assert(playnodenum != choice_nodenum);
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
	printf("Playout Times : %d\n", total_play);
	printf("Node Num : %d\n", nodenum);
}
