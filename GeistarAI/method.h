#pragma once

#include <cassert>
#include <iostream>

#include "types.h"


#define MYGOAL 0x0000000000000042
#define MYGOAL_L 0x0000000000000040
#define MYGOAL_R 0x0000000000000002
#define ENGOAL 0x4200000000000000
#define ENGOAL_L 0x4000000000000000
#define ENGOAL_R 0x0200000000000000

inline Point x(Point xy) { return xy & 7; }		// 1～6が盤面内
inline Point y(Point xy) { return xy >> 3; }	// 1～6が盤面内
inline Point x(BitBoard b)
{
	if (b & 0x8080808080808080) return 7;
	if (b & 0x4040404040404040) return 6;
	if (b & 0x2020202020202020) return 5;
	if (b & 0x1010101010101010) return 4;
	if (b & 0x0808080808080808) return 3;
	if (b & 0x0404040404040404) return 2;
	if (b & 0x0202020202020202) return 1;
	if (b & 0x0101010101010101) return 0;
	return 0;
}
inline Point y(BitBoard b)
{
	if (b & 0xff00000000000000) return 7;
	if (b & 0x00ff000000000000) return 6;
	if (b & 0x0000ff0000000000) return 5;
	if (b & 0x000000ff00000000) return 4;
	if (b & 0x00000000ff000000) return 3;
	if (b & 0x0000000000ff0000) return 2;
	if (b & 0x000000000000ff00) return 1;
	if (b & 0x00000000000000ff) return 0;
	return 0;
}

//内側だけのマスク処理
inline BitBoard Inside(BitBoard bb)
{
	return (bb & 0x007E7E7E7E7E7E00);
}
inline bool Goal(Point xy, bool player)
{
	if (player == 0 && (xy == 62 || xy == 57))
		return true;
	else if (player == 1 && (xy == 1 || xy == 6))
		return true;
	else
		return false;
}
inline bool Goal(BitBoard bb, bool player)
{
	return bb & (player ? ENGOAL : MYGOAL);
	//if (player == 1 && (bb & ENGOAL))
	//	return true;
	//else if (player == 0 && (bb & MYGOAL))
	//	return true;
	//else
	//	return false;
}
inline BitBoard InsideOrGoal(BitBoard bb, bool player)
{
	return (bb & (0x007E7E7E7E7E7E00 | (player ? ENGOAL : MYGOAL)));
}

inline BitBoard toBit(Point xy)
{
	// 8x8の内側
	return (1LL << xy);
}
inline BitBoard toBit(uint8_t x, uint8_t y)
{
	// 8x8の内側
	return (1LL << ((y << 3) + x));
}
inline BitBoard toBit_0indexed(uint8_t x, uint8_t y)
{
	// 8x8の内側
	return (1LL << (((y + 1) << 3) + x + 1));
}

inline Point toPoint(uint8_t x, uint8_t y)
{
	return (y << 3) + x;
}
inline Point toPoint_0indexed(uint8_t x, uint8_t y)
{
	return ((y + 1) << 3) + x + 1;
}

//実質底が2のlog
inline Point toPoint(BitBoard bb)
{
	assert(__popcnt64(bb) == 1);
	//二分探索で高速になりそう
	Point res = 0;
	res += (bb & 0xffffffff00000000) ? 32 : 0;
	res += (bb & 0xffff0000ffff0000) ? 16 : 0;
	res += (bb & 0xff00ff00ff00ff00) ? 8 : 0;
	res += (bb & 0xf0f0f0f0f0f0f0f0) ? 4 : 0;
	res += (bb & 0xcccccccccccccccc) ? 2 : 0;
	res += (bb & 0xaaaaaaaaaaaaaaaa) ? 1 : 0;
	return res;
}

inline void toPieces(Point pieces[16], Recieve rec)
{
	int i = 4;
	for (int j = 0; j < 16; i += 3, j++)
	{
		int x = rec[i] - '0';
		int y = rec[i + 1] - '0';
		char c = rec[i + 2];
		if (x >= 6) x = 6;
		if (y >= 6) y = 6;

		pieces[j] = toPoint_0indexed(x, y);
	}
}
inline void toPieceNum(PieceNum num[64], Recieve rec)
{
	fill(num, num + 64, -1);
	int i = 4;
	for (int j = 0; j < 16; i += 3, j++)
	{
		int x = rec[i] - '0';
		int y = rec[i + 1] - '0';
		char c = rec[i + 2];
		if (x >= 6) x = 6;
		if (y >= 6) y = 6;

		num[toPoint_0indexed(x, y)] = j;
	}
}

inline Board toBoard(Recieve rec)
{
	Board res;
	int N = rec.size();
	for (int i = 4, j = 0; j < 16; i += 3, j++)
	{
		int x = rec[i] - '0';
		int y = rec[i + 1] - '0';
		char c = rec[i + 2];
		if (j < 8)	//自分の
		{
			if (x == 9 && y == 9)
			{
				if (c == 'b')
				{
					res.dead_myblue++;
				}
				else if (c == 'r')
				{
					res.dead_myred++;
				}
				else
					assert(false);
			}
			else
			{
				if (c == 'B')
				{
					res.myblue |= toBit_0indexed(x, y);
				}
				else if (c == 'R')
				{
					res.myred |= toBit_0indexed(x, y);
				}
				else
				{
					cout << "c = " << c << endl;
					assert(false);
				}
			}
		}
		else	//相手の
		{
			if (x == 9 && y == 9)
			{
				if (c == 'b')
				{
					res.dead_enblue++;
				}
				else if (c == 'r')
				{
					res.dead_enred++;
				}
				else
				{
					cout << "c = " << c << endl;
					assert(false);
				}
			}
			else
			{
				res.en |= toBit_0indexed(x, y);	// if (c == 'u')
			}
		}
	}
	assert(res.enblue == 0 || res.enred == 0);
	res.en_mix = res.en;
	res.my_mix = res.myblue | res.myred;
	toPieces(res.pieces, rec);
	return res;
}

//inline PieceNum getPieceNum(Pieces pieces, Point point)
//{
//	for (PieceNum i = 0; i < 16; i++)
//	{
//		if (pieces.pos[i] == point)
//			return i;
//	}
//	assert(false);
//	return 255;
//}

//(↑…0, →…1, ↓…2, ←…3)
inline Direct toDir(Point p, Point newpoint)
{
	int8_t dir[4] = { -8, 1, 8, -1 };
	for (int i = 0; i < 4; i++)
	{
		if (p + dir[i] == newpoint)
			return i;
	}
	cout << (int)p << " " << (int)newpoint << endl;
	assert(false);
	return 255;
}

inline Point nextpos(MoveCommand move)
{
	if (move.dir == 0) return move.xy - 8;
	if (move.dir == 1) return move.xy + 1;
	if (move.dir == 2) return move.xy + 8;
	if (move.dir == 3) return move.xy - 1;
	assert(false);
}
inline BitBoard nextpos(BitBoard pos, uint8_t dir)
{
	if (dir == 0) return (pos >> 8);
	if (dir == 1) return (pos << 1);
	if (dir == 2) return (pos << 8);
	if (dir == 3) return (pos >> 1);
	assert(false);
}

inline bool onPiece(BitBoard bb, Point p)
{
	assert(p <= 63);
	return (bb >> p & 1);
}
inline bool onPiece(BitBoard bb1, BitBoard bb2)
{
	return (bb1 & bb2);
}

inline void change(BitBoard& bb, BitBoard ppos, BitBoard npos)
{
	bb ^= (ppos ^ npos);
}

inline Send toSend_piece(MoveCommand move, Point pieces[16])
{
	Send sen("MOV:");
	for (int i = 0; i < 8; i++)
	{
		if (onPiece(toBit(move.xy), pieces[i]))
		{
			sen += (char)(i + 'A');
			break;
		}
	}
	sen += ',';
	char dirchar[4] = { 'N', 'E', 'S', 'W' };
	sen += dirchar[move.dir];

	sen += "\r\n";

	return sen;
}
inline Send toSend(MoveCommand move, PieceNum piecenum[64])
{
	Send sen("MOV:");
	sen += (char)(piecenum[move.xy] + 'A');
	sen += ',';
	char dirchar[4] = { 'N', 'E', 'S', 'W' };
	sen += dirchar[move.dir];

	sen += "\r\n";

	return sen;
}

//これ頭よいので使おう
//ONになっているビットの一番下を取り出す（そこだけONのビット列にする）
inline BitBoard getBottomBB(BitBoard bb)
{
	return (bb & (-bb));
}
//ONになっているビットの一番下のビットをoffする
inline void offBottomBB(BitBoard& bb)
{
	bb &= (bb - 1);
}
//

inline BitBoard getNextPosBB(BitBoard bb)
{
	//00000000 00001000 00000000
	//↓
	//00001000 00010100 00001000
	return ((bb << 8) | (bb << 1) | (bb >> 1) | (bb >> 8));
}

inline BitBoard to36bit(BitBoard bb)
{
	BitBoard res = 0;
	//00000000 01111110 01111110 01111110
	res |= bb >> 9 & 0x3f;
	res |= bb >> 11 & 0xfc0;
	res |= bb >> 13 & 0x3f000;
	res |= bb >> 15 & 0xfc0000;
	res |= bb >> 17 & 0x3f000000;
	res |= bb >> 19 & 0xfc0000000;
	return res;
}

inline Point _abs(Point a)
{
	return ((a & 0x80) ? -a : a);
	//return ((a & 0x80) ? (~a) + 1 : a);	//これが-aでできるのか
}
inline int distance(BitBoard a, BitBoard b)
{
	return _abs(x(a) - x(b)) + _abs(y(a) - y(b));
}

//移動前の位置pposと移動先の位置nposから boardを更新する
inline void toNextBoard(Board& board, BitBoard ppos, BitBoard npos, bool nowPlayer)
{
	if (Goal(npos, nowPlayer))	//脱出した
		board.escape = true;
	else	//脱出してない
		board.escape = false;

	if (nowPlayer == 0)
	{
		//激遅ポイント
		//pposにあった駒をnposに動かす
		//このときnposに相手駒があれば消す
		//(unorderd_map使えば速くなる??)
		for (int i = 0; i < 8; i++)
		{
			if (toBit(board.pieces[i]) == ppos)
			{
				board.pieces[i] = toPoint(npos);
				for (int j = 8; j < 16; j++)
				{
					if (toBit(board.pieces[j]) == npos)
					{
						board.pieces[j] = 63;
						break;
					}
				}
				break;
			}
		}
		//
		if (onPiece(board.myblue, ppos))	//青を動かした
			change(board.myblue, ppos, npos);	//駒の移動
		else	//赤を動かした
			change(board.myred, ppos, npos);
		board.my_mix = board.myblue | board.myred | board.my;

		if (onPiece(board.en_mix, npos))
		{
			//青駒を取った
			if (onPiece(board.enblue, npos))
			{
				board.enblue ^= npos;
				board.dead_enblue++;
			}
			//赤駒を取った
			else if (onPiece(board.enred, npos))
			{
				board.enred ^= npos;
				board.dead_enred++;
			}
			else if (onPiece(board.en, npos))
			{
				board.en ^= npos;
				board.dead_en++;
			}
			else
			{
				assert(false);
			}
			board.en_mix ^= npos;	//敵駒を取った後の状態に
			board.kill = true;
		}
		else	//敵駒を取らなかった
		{
			board.kill = false;
		}
		assert(board.en_mix != 0);
	}
	else
	{
		//激遅ポイント
		for (int i = 8; i < 16; i++)
		{
			if (toBit(board.pieces[i]) == ppos)
			{
				board.pieces[i] = toPoint(npos);
				for (int j = 0; j < 8; j++)
				{
					if (toBit(board.pieces[j]) == npos)
					{
						board.pieces[j] = 63;
					}
				}
				break;
			}
		}
		//
		change(board.en_mix, ppos, npos);
		if (onPiece(board.enblue, ppos))
			change(board.enblue, ppos, npos);
		if (onPiece(board.enred, ppos))
			change(board.enred, ppos, npos);
		if (onPiece(board.en, ppos))
			change(board.en, ppos, npos);

		if (onPiece(board.my_mix, npos))
		{
			if (onPiece(board.myblue, npos))	//青駒を取られた
			{
				board.myblue ^= npos;
				board.dead_myblue++;
			}
			else if (onPiece(board.myred, npos))	//赤駒を取られた
			{
				board.myred ^= npos;
				board.dead_myred++;
			}
			else if (onPiece(board.my, npos))
			{
				board.my ^= npos;
				board.dead_my++;
			}
			else
			{
				assert(false);
			}
			board.my_mix ^= npos;
			board.kill = true;
		}
		else
		{
			board.kill = false;
		}
	}
}

//board.piece[]を使わない
inline void toNextBoard_quick(Board& board, BitBoard ppos, BitBoard npos, bool nowPlayer)
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
		board.my_mix = board.myblue | board.myred | board.my;

		if (onPiece(board.en_mix, npos))
		{
			//青駒を取った
			if (onPiece(board.enblue, npos))
			{
				board.enblue ^= npos;
				board.dead_enblue++;
			}
			//赤駒を取った
			else if (onPiece(board.enred, npos))
			{
				board.enred ^= npos;
				board.dead_enred++;
			}
			else if (onPiece(board.en, npos))
			{
				board.en ^= npos;
				board.dead_en++;
			}
			else
			{
				assert(false);
			}
			board.en_mix ^= npos;	//敵駒を取った後の状態に
			board.kill = true;
		}
		else	//敵駒を取らなかった
		{
			board.kill = false;
		}
		assert(board.en_mix != 0);
	}
	else
	{
		change(board.en_mix, ppos, npos);
		if (onPiece(board.enblue, ppos))
			change(board.enblue, ppos, npos);
		if (onPiece(board.enred, ppos))
			change(board.enred, ppos, npos);
		if (onPiece(board.en, ppos))
			change(board.en, ppos, npos);

		if (onPiece(board.my_mix, npos))
		{
			if (onPiece(board.myblue, npos))	//青駒を取られた
			{
				board.myblue ^= npos;
				board.dead_myblue++;
			}
			else if (onPiece(board.myred, npos))	//赤駒を取られた
			{
				board.myred ^= npos;
				board.dead_myred++;
			}
			else if (onPiece(board.my, npos))
			{
				board.my ^= npos;
				board.dead_my++;
			}
			else
			{
				assert(false);
			}
			board.my_mix ^= npos;
			board.kill = true;
		}
		else
		{
			board.kill = false;
		}
	}
}


// すべての合法手後の盤面を配列に入れる
// 敵駒の色判定はしない
inline void PossibleNextBoard(Board nextpositions[32], bool nowPlayer, Board board, int& nextboards_size)
{

	//getBottomBB()とoffBottomBB()を使って高速に調べるようにしたい
	if (nowPlayer == 0)
	{
		// どの位置の駒を動かすかのfor
		for (BitBoard piecebb = board.my_mix; piecebb != 0; offBottomBB(piecebb))
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
				if (onPiece(board.my_mix, npos))	//自身の駒が重なる
					continue;


				Board nextboard = board;
				toNextBoard(nextboard, ppos, npos, nowPlayer);
				//nextpositions.emplace_back(nextboard);
				nextpositions[nextboards_size] = nextboard;
				nextboards_size++;

			} // for nextbb
		} // for piecebb
	} // if nowPlayer == 0
	else
	{
		for (BitBoard piecebb = board.en_mix; piecebb != 0; offBottomBB(piecebb))
		{
			BitBoard ppos = getBottomBB(piecebb);
			for (BitBoard nextbb = getNextPosBB(ppos); nextbb != 0; offBottomBB(nextbb))
			{
				BitBoard npos = getBottomBB(nextbb);
				if (!Inside(npos) && !Goal(npos, nowPlayer))	//盤面外
					continue;
				if (Goal(npos, nowPlayer) && onPiece(board.enred, ppos))	//赤駒が脱出しようとした
					continue;
				if (onPiece(board.en_mix, npos))	//自身の駒が重なる
					continue;


				Board nextboard = board;
				toNextBoard(nextboard, ppos, npos, nowPlayer);
				//nextpositions.emplace_back(nextboard);
				nextpositions[nextboards_size] = nextboard;
				nextboards_size++;
			}
		}

	} //nowPlayer == 1

	return;
}

inline void PossibleNextBoard_quick(Board nextpositions[32], bool nowPlayer, Board board, int& nextboards_size)
{

	//getBottomBB()とoffBottomBB()を使って高速に調べるようにしたい
	if (nowPlayer == 0)
	{
		// どの位置の駒を動かすかのfor
		for (BitBoard piecebb = board.my_mix; piecebb != 0; offBottomBB(piecebb))
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
				if (onPiece(board.my_mix, npos))	//自身の駒が重なる
					continue;


				Board nextboard = board;
				toNextBoard_quick(nextboard, ppos, npos, nowPlayer);
				//nextpositions.emplace_back(nextboard);
				nextpositions[nextboards_size] = nextboard;
				nextboards_size++;

			} // for nextbb
		} // for piecebb
	} // if nowPlayer == 0
	else
	{
		for (BitBoard piecebb = board.en_mix; piecebb != 0; offBottomBB(piecebb))
		{
			BitBoard ppos = getBottomBB(piecebb);
			for (BitBoard nextbb = getNextPosBB(ppos); nextbb != 0; offBottomBB(nextbb))
			{
				BitBoard npos = getBottomBB(nextbb);
				if (!Inside(npos) && !Goal(npos, nowPlayer))	//盤面外
					continue;
				if (Goal(npos, nowPlayer) && onPiece(board.enred, ppos))	//赤駒が脱出しようとした
					continue;
				if (onPiece(board.en_mix, npos))	//自身の駒が重なる
					continue;


				Board nextboard = board;
				toNextBoard_quick(nextboard, ppos, npos, nowPlayer);
				//nextpositions.emplace_back(nextboard);
				nextpositions[nextboards_size] = nextboard;
				nextboards_size++;
			}
		}

	} //nowPlayer == 1

	return;
}

inline int intsqrt(int a)
{
	if (a < 1) return 0;
	int l = 0, r = a;
	int c = 0;
	while (r - l > 1)
	{
		c = (l + r) / 2;
		if (c * c <= a)
			l = c;
		else
			r = c;
	}
	return l;
}
inline int intlog(int a)
{
	int res = 0;
	res += (a & 0xffff0000) ? 16 : 0;
	res += (a & 0xff00ff00) ? 8 : 0;
	res += (a & 0xf0f0f0f0) ? 4 : 0;
	res += (a & 0xcccccccc) ? 2 : 0;
	res += (a & 0xaaaaaaaa) ? 1 : 0;
	return res;
}
#define FACTOR 1.41421356 //(pow(2, 0.5)) のままでは毎回演算するはず(?)
#define MAX_VALUE (1LL << 30);
inline int intcompare(int win, int play, int total_play)
{
	//return (play == 0 ? 0.5 : win / play) + FACTOR * pow(log(total_play) / (1.0+play), 0.5);
	if (play == 0) return MAX_VALUE;
	return (win * 100 / play) + 141 * intsqrt(intlog(total_play) * 100 / play);
}
inline double compare(double win, int play, int total_play)
{
	//return (play == 0 ? 0.5 : win / play) + FACTOR * pow(log(total_play) / (1.0+play), 0.5);
	if (play == 0) return MAX_VALUE;
	return (win / play) + FACTOR * pow(log(total_play) / play, 0.5);
}
inline double compare_RF(double win, int play, int parent_play, double NN_softmax)
{
	return (play == 0 ? 0.5 : win / play) + (log((1.0 + parent_play + 19652) / 19652.0) + 1.25) * NN_softmax * pow(parent_play, 0.5) / (1.0 + play);
	//return (win == 0 ? 0 : win / play) + (log((1.0 + parent_play + 1965) / 1965.0) + 1.25) * NN_softmax * pow(parent_play, 0.5) / (1.0 + play);
	//play==0なら当然win==0だよねという工夫
	//でもplay==0なら引き分けが良いのでは?
}