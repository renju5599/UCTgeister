#pragma once

#include <cassert>
#include <iostream>

#include "types.h"

inline Point x(Point xy) { return xy & 7; }		// 1Å`6Ç™î’ñ ì‡
inline Point y(Point xy) { return xy >> 3; }	// 1Å`6Ç™î’ñ ì‡

inline bool Outside(Point xy)
{
	if (y(xy) == 0) return true;
	if (y(xy) == 7) return true;
	if (x(xy) == 0) return true;
	if (x(xy) == 7) return true;
	return false;
}
inline bool Outside(BitBoard bb)
{
	return (bool)(bb & 0xFF818181818181FF);
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
	if (player == 0 && (bb & 0x4200000000000000))
		return true;
	else if (player == 1 && (bb & 0x0000000000000042))
		return true;
	else
		return false;
}

inline BitBoard toBit(Point xy)
{
	// 8x8ÇÃì‡ë§
	return (1LL << xy);
}
inline BitBoard toBit(uint8_t x, uint8_t y)
{
	// 8x8ÇÃì‡ë§
	return (1LL << ((y << 3) + x));
}
inline BitBoard toBit_0indexed(uint8_t x, uint8_t y)
{
	// 8x8ÇÃì‡ë§
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
inline Point toPoint(BitBoard bb)
{
	assert(bb != 0);
	//ìÒï™íTçıÇ≈çÇë¨Ç…Ç»ÇËÇªÇ§
	BitBoard flag[6] = {
		0xffffffff00000000,
		0xffff0000,
		0xff00,
		0xf0,
		0xc,
		0x2
	};
	Point p[6] = {
		32,
		16,
		8,
		4,
		2,
		1
	};
	Point res = 0;
	for (int i = 0; i < 6; i++)
	{
		if (bb & flag[i])
		{
			bb >>= p[i];
			res += p[i];
		}
	}
	return res;
}

inline Recieve toRecieve(Pieces pieces, Board board)
{
	Recieve rec = "***?";
	for (int i = 0; i < 8; i++)
	{
		if (Outside(pieces.pos[i]))
		{
			rec += '9';
			rec += '9';
			rec += (board.dead_myblue & toBit(pieces.pos[i]) ? 'b' : 'r');
		}
		else
		{
			rec += x(pieces.pos[i]) + '0';
			rec += y(pieces.pos[i]) + '0';
			rec += (board.myblue & toBit(pieces.pos[i]) ? 'B' : 'R');
		}
	}
	for (int i = 8; i < 16; i++)
	{
		if (Outside(pieces.pos[i]))
		{
			rec += '9';
			rec += '9';
			rec += (board.dead_enblue & toBit(pieces.pos[i]) ? 'b' : 'r');
		}
		else
		{
			rec += x(pieces.pos[i]) + '0';
			rec += y(pieces.pos[i]) + '0';
			rec += 'u';
		}
	}
	return rec;
}

inline Pieces toPieces(Recieve rec)
{
	Pieces pieces = {};
	int N = rec.size();
	int i = 4;
	for (int j = 0; j < 16; i += 3, j++)
	{
		int x = rec[i] - '0';
		int y = rec[i + 1] - '0';
		char c = rec[i + 2];
		if (x == 9) x = 6;
		if (y == 9) y = 6;

		pieces.pos[j] = toPoint_0indexed(x, y);
	}
	return pieces;
}

inline Board toBoard(Recieve rec)
{
	Board res;
	int N = rec.size();
	for (int i = 4; i < N - 2; i += 3)
	{
		int x = rec[i] - '0';
		int y = rec[i + 1] - '0';
		char c = rec[i + 2];
		if (i < 3 * 8 + 4)	//é©ï™ÇÃ
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
		else	//ëäéËÇÃ
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
				res.enemy |= toBit_0indexed(x, y);	// if (c == 'u')
			}
		}
	}
	return res;
}

inline PieceNum getPieceNum(Pieces pieces, Point point)
{
	for (PieceNum i = 0; i < 16; i++)
	{
		if (pieces.pos[i] == point)
			return i;
	}
	assert(false);
	return 255;
}

//(Å™Åc0, Å®Åc1, Å´Åc2, Å©Åc3)
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

inline BitBoard change(BitBoard bb, BitBoard ppos, BitBoard npos)
{
	return (bb & ~ppos | npos);
}

inline Send toSend(MoveCommand move, Pieces pieces)
{
	Send sen("MOV:");
	for (int i = 0; i < 8; i++)
	{
		if (onPiece(toBit(move.xy), pieces.pos[i]))
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

//Ç±ÇÍì™ÇÊÇ¢ÇÃÇ≈égÇ®Ç§
inline BitBoard getBottomBB(BitBoard bb)
{
	return (bb & (-bb));
}
inline BitBoard offBottomBB(BitBoard bb)
{
	return (bb & (bb - 1));
}
//
inline BitBoard getNextPosBB(BitBoard bb)
{
	//00000000 00001000 00000000
	//Å´
	//00001000 00010100 00001000
	return ((bb << 8) | (bb << 1) | (bb >> 1) | (bb >> 8));
}

void PossibleMoves(vector<MoveCommand>& nextmoves, bool nowPlayer, Board board);