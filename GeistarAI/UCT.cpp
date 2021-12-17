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

#define WIN_VALUE 1
#define LOSE_VALUE 0
#define DRAW_VALUE 0

using namespace std;


// ���ׂĂ̍��@���̔Ֆʂ�vector�ɓ����
// �G�����������̐F�͐Ԃ̂Ƃ����̂Ƃ��������i�m���Ƃ��͍l�����Ă��Ȃ��j
void PossibleNextBoard(vector<Board>& nextpositions, bool nowPlayer, Board board)
{
	BitBoard nowmyblue = board.myblue;
	BitBoard nowmyred = board.myred;
	BitBoard nowmy = nowmyblue | nowmyred;
	BitBoard nowen = board.enemy;

	//getBottomBB()��offBottomBB()���g���č����ɒ��ׂ�悤�ɂ�����
	if (nowPlayer == 0)
	{
		// �ǂ̈ʒu�̋�𓮂�������for
		for (BitBoard piecebb = nowmy; piecebb != 0; piecebb = offBottomBB(piecebb))
		{
			BitBoard ppos = getBottomBB(piecebb);
			// �ړ�4����������for
			for (BitBoard nextbb = getNextPosBB(ppos); nextbb != 0; nextbb = offBottomBB(nextbb))
			{
				BitBoard npos = getBottomBB(nextbb);
				//���@��������
				if (Outside(npos) && !Goal(npos, nowPlayer))	//�ՖʊO
					continue;
				if (Goal(npos, nowPlayer) && onPiece(nowmyred, ppos))	//�ԋ�E�o���悤�Ƃ���
					continue;
				if (onPiece(nowmy, npos))	//���g�̋�d�Ȃ�
					continue;

				if (Goal(npos, nowPlayer))	//�E�o����
				{
					board.escape = true;
				}
				else	//�E�o���ĂȂ�
				{
					board.escape = false;
				}
				if (onPiece(nowmyblue, ppos))	//�𓮂�����
				{
					board.myblue = change(nowmyblue, ppos, npos);	//��̈ړ�
					if (onPiece(nowen, npos))	//�G��������
					{
						board.enemy ^= npos;	//�G����������̏�Ԃ�
						if (board.enemy == 0)
						{
							cout << (int)board.dead_enblue << " " << (int)board.dead_enred << endl;
						}
						assert(board.enemy != 0);
						board.kill = true;
						//��������
						board.dead_enblue <<= 1;
						nextpositions.push_back(board);
						board.dead_enblue >>= 1;
						//�ԋ�������
						board.dead_enred <<= 1;
						nextpositions.push_back(board);
						board.dead_enred >>= 1;
						//���ɖ߂�
						board.enemy ^= npos;
					}
					else	//�G������Ȃ�����
					{
						board.kill = false;
						nextpositions.push_back(board);
					}
					board.myblue = nowmyblue;	//�������O�ɖ߂�
				}
				else	//�Ԃ𓮂�����
				{
					board.myred = change(nowmyred, ppos, npos);
					if (onPiece(nowen, npos))
					{
						board.enemy ^= npos;
						if (board.enemy == 0)
						{
							cout << (int)board.dead_enblue << " " << (int)board.dead_enred << endl;
						}
						assert(board.enemy != 0);
						board.kill = true;

						board.dead_enblue <<= 1;
						nextpositions.push_back(board);
						board.dead_enblue >>= 1;

						board.dead_enred <<= 1;
						nextpositions.push_back(board);
						board.dead_enred >>= 1;

						board.enemy ^= npos;
					}
					else
					{
						board.kill = false;
						nextpositions.push_back(board);
					}
					board.myred = nowmyred;

				} // ���������F������if-else
			} // for nextbb
		} // for piecebb
	} // if nowPlayer == 0
	else
	{
		if (nowen == 0)
		{
			cout << board.dead_enblue << " " << board.dead_enred << endl;
		}
		assert(nowen != 0);
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

				if (Goal(npos, nowPlayer))
				{
					board.escape = true;
				}
				else
				{
					board.escape = false;
				}

				board.enemy = change(nowen, ppos, npos);
				if (onPiece(nowmyblue, npos))	//������ꂽ
				{
					board.myblue ^= npos;
					board.kill = true;

					board.dead_myblue <<= 1;
					nextpositions.push_back(board);
					board.dead_myblue >>= 1;

					board.myblue ^= npos;
				}
				else if(onPiece(nowmyred, npos))	//�ԋ�����ꂽ
				{
					board.myred ^= npos;
					board.kill = true;

					board.dead_myred <<= 1;
					nextpositions.push_back(board);
					board.dead_myred >>= 1;

					board.myred ^= npos;
				}
				else
				{
					board.kill = false;
					nextpositions.push_back(board);
				}
				board.enemy = nowen;

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

//UCT���̃v���C�A�E�g������֐�
int UCT::playout(bool nowPlayer, int playoutnum, Board nowboard)
{

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
		vector<Board> nextboards;
		PossibleNextBoard(nextboards, nowPlayer, nowboard);
		int nextmoves_size = nextboards.size();
		if (nextmoves_size <= 0)
		{
			cout << "size:" << nextmoves_size << endl;
			cout << "nowplayer:" << nowPlayer << endl;
			cout << "playoutnum:" << playoutnum << endl;
		}
		assert(nextmoves_size > 0);
		int r = rand() % nextmoves_size;
		nowboard = nextboards[r];

		if (playoutnum > MAXPLAY)
			return DRAW_VALUE;
		if (nowboard.escape)
			return (nowPlayer == 0 ? WIN_VALUE : LOSE_VALUE);
		if (nowboard.dead_myblue == 16 || nowboard.dead_enred == 16)
			return LOSE_VALUE;
		if (nowboard.dead_myred == 16 || nowboard.dead_enblue == 16)
			return WIN_VALUE;

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
					nextnode.finish = (
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
				int reward = playout(nowPlayer, playoutnum, Nodes[search_node].board);	//����:3  ����:1  ����:0�@(������)

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
	NodeValue maxvalue = Nodes[choice_nodenum].value;
	for (NodeNum nextnode : Nodes[playnodenum].nextnodes)
	{
		// �����̎�ԁˑ傫���̂�I��
		if (chmax(maxvalue, Nodes[nextnode].value))
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
