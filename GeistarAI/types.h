#pragma once

#include <string>
#include <vector>

using namespace std;


template<class T>inline bool chmax(T& a, const T& b) { if (a < b) { a = b; return 1; } return 0; }
template<class T>inline bool chmin(T& a, const T& b) { if (a > b) { a = b; return 1; } return 0; }

typedef uint64_t BitBoard;
typedef uint8_t Point;
typedef uint8_t PieceNum;
typedef uint8_t Direct;
typedef std::string Recieve;
typedef std::string Send;
typedef uint8_t Dead;
typedef uint64_t Hash;

struct Status	//������z�@�p
{
	bool bit[122];
};

struct Board
{
	BitBoard myblue, myred;
	BitBoard enemy, enblue, enred;
	Dead dead_myblue, dead_myred;	//one-hot
	Dead dead_enblue, dead_enred;	//one-hot
	bool kill;
	bool escape;
	bool willplayer;

	Board()
	{
		myblue = myred = enemy = enblue = enred = 0;
		dead_myblue = dead_myred = 1;
		dead_enblue = dead_enred = 1;
		kill = escape = willplayer = 0;
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
	////�召�͂ǂ��ł��悢�̂Ń\�[�g�ł���悤�ɂ���
	////map�Ŏg������
	// map�͎g��Ȃ��Ȃ�܂����B�����unordered_map���g���i�����̂Łj
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
//unordered_map�̂��߂�hash
struct BoardHash
{
	inline size_t operator()(const Board& data) const
	{
		return
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
	Direct dir;	//�ړ�������� (���c0, ���c1, ���c2, ���c3)
	MoveCommand()
	{
		xy = 0;
		dir = 0;
	}
};


// ��0�`7���ǂ���������
// ���ꂽ���(9,9)�ł͂Ȃ�(7,7)�Ƃ���
struct Pieces
{
	Point pos[16];
};



