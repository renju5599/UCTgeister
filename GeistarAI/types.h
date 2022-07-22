#pragma once

#include <string>
#include <vector>
#include <bitset>

using namespace std;


template<class T>inline bool chmax(T& a, const T& b) { if (a < b) { a = b; return 1; } return 0; }
template<class T>inline bool chmin(T& a, const T& b) { if (a > b) { a = b; return 1; } return 0; }

typedef uint64_t BitBoard;
typedef int8_t Point;
typedef int8_t Point36;
typedef int8_t PieceNum;
typedef int8_t Direct;
typedef std::string Recieve;
typedef std::string Send;
typedef int8_t Dead;



struct BitsStatus	//方策勾配法用
{
	//bool bit[127];
	//bool TwoPiece[8128];
	//uint64_t bit[2] = {};
	//uint64_t TwoPiece[127][2];
	bitset<128> bit;
	bitset<8128> TwoPiece;
};

struct Board
{
	//my_mix,en_mixは総和, my,enは不明なやつ
	BitBoard my_mix, myblue, myred, my;
	BitBoard en_mix, enblue, enred, en;
	Dead dead_my, dead_myblue, dead_myred;
	Dead dead_en, dead_enblue, dead_enred;
	bool kill;
	bool escape;
	Point pieces[16];
	bool myred_eval = false;

	Board()
	{
		my_mix = myblue = myred = my = en_mix = enblue = enred = en = 0;
		dead_my = dead_myblue = dead_myred = 0;
		dead_en = dead_enblue = dead_enred = 0;
		kill = escape = 0;
		fill(pieces, pieces + 16, 0);
	}

	bool const operator==(const Board& right) const
	{
		return (
			my != right.my ? false :
			myblue != right.myblue ? false :
			myred != right.myred ? false :
			en != right.en ? false :
			enblue != right.enblue ? false :
			enred != right.enred ? false :
			dead_myblue != right.dead_myblue ? false :
			dead_myred != right.dead_myred ? false :
			dead_enblue != right.dead_enblue ? false :
			dead_enred != right.dead_enred ? false :
			true
			);
	}

};
//unordered_mapのためのhash
struct BoardHash
{
	inline size_t operator()(const Board& data) const
	{
		return
			hash<BitBoard>()(data.myblue) ^
			hash<BitBoard>()(data.myred) ^
			hash<BitBoard>()(data.my) ^
			hash<BitBoard>()(data.enblue) ^
			hash<BitBoard>()(data.enred) ^
			hash<BitBoard>()(data.en) ^
			hash<Dead>()(data.dead_my) ^
			hash<Dead>()(data.dead_myblue) ^
			hash<Dead>()(data.dead_myred) ^
			hash<Dead>()(data.dead_en) ^
			hash<Dead>()(data.dead_enblue) ^
			hash<Dead>()(data.dead_enred);
	}
};

struct MoveCommand
{
	Point xy;
	Direct dir;	//移動する方向 (↑…0, →…1, ↓…2, ←…3)
	MoveCommand()
	{
		xy = 0;
		dir = 0;
	}
};
