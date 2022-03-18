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
	BitBoard my, myblue, myred;
	BitBoard enemy, enblue, enred;
	Dead dead_myblue, dead_myred;
	Dead dead_enblue, dead_enred;
	bool kill;
	bool escape;
	bool willplayer;
	Point pieces[16];
	bool myred_eval = false;

	Board()
	{
		my = myblue = myred = enemy = enblue = enred = 0;
		dead_myblue = dead_myred = 0;
		dead_enblue = dead_enred = 0;
		kill = escape = willplayer = 0;
		fill(pieces, pieces + 16, 0);
	}

	bool const operator==(const Board& right) const
	{
		return (
			myblue != right.myblue ? false :
			myred != right.myred ? false :
			enemy != right.enemy ? false :
			enblue != right.enblue ? false :
			enred != right.enred ? false :
			dead_myblue != right.dead_myblue ? false :
			dead_myred != right.dead_myred ? false :
			dead_enblue != right.dead_enblue ? false :
			dead_enred != right.dead_enred ? false :
			willplayer != right.willplayer ? false :
			true
			);
	}
	////大小はどうでもよいのでソートできるようにする
	////mapで使うため
	// mapは使わなくなりました。代わりにunordered_mapを使う（速いので）
	//bool operator<(const Board& right) const
	//{
	//	return (
	//		myblue < right.myblue ? true :
	//		myblue > right.myblue ? false :
	//		myred < right.myred ? true :
	//		myred > right.myred ? false :
	//		enemy < right.enemy ? true :
	//		enemy > right.enemy ? false :
	//		enblue < right.enblue ? true :
	//		enblue > right.enblue ? false :
	//		enred < right.enred ? true :
	//		enred > right.enred ? false :
	//		dead_myblue < right.dead_myblue ? true :
	//		dead_myblue > right.dead_myblue ? false :
	//		dead_myred < right.dead_myred ? true :
	//		dead_myred > right.dead_myred ? false :
	//		dead_enblue < right.dead_enblue ? true :
	//		dead_enblue > right.dead_enblue ? false :
	//		dead_enred < right.dead_enred ? true :
	//		dead_enred > right.dead_enred ? false :
	//		willplayer < right.willplayer ? false :
	//		willplayer > right.willplayer ? true :
	//		//kill < right.kill ? true :
	//		//escape < right.escape ? true :
	//		false
	//		);
	//}
};
//unordered_mapのためのhash
struct BoardHash
{
	inline size_t operator()(const Board& data) const
	{
		return
			hash<BitBoard>()(data.my) ^
			hash<BitBoard>()(data.myblue) ^
			hash<BitBoard>()(data.myred) ^
			hash<BitBoard>()(data.enemy) ^
			hash<BitBoard>()(data.enblue) ^
			hash<BitBoard>()(data.enred) ^
			hash<Dead>()(data.dead_myblue) ^
			hash<Dead>()(data.dead_myred) ^
			hash<Dead>()(data.dead_enblue) ^
			hash<Dead>()(data.dead_enred) ^
			hash<bool>()(data.willplayer);
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
