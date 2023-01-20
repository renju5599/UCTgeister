#pragma once

#include <cassert>

#include "method.h"
#include "types.h"
#include "weight.h"

inline uint16_t getHeadIndex(bitset<8128> bs)
{
	assert(bs.any());
	uint16_t res = 0;
	for (int shif = 4096; shif >= 1; shif >>= 1)
	{
		if ((bs >> shif).any())
		{
			res += shif;
			bs >>= shif;
		}
	}
	return res;
}

inline double getNNres_manybit(const bitset<8128>& bs)
{
	double res = 0;
	for (int i = 0; i < 8128; i++)
	{
		if (bs.test(i))
			res += NN::weight[i];
	}
	return res;
}
inline double getNNres_fewbit(bitset<8128>& bs)
{
	double res = 0;
	uint16_t pos = 0;
	while (bs.any())
	{
		pos = getHeadIndex(bs);
		bs.reset(pos);
		res += NN::weight[pos];
	}
	return res;
}

//一個前の盤面との差分だけ見れば速そう
inline double getNNres(const bitset<8128>& bsta_TP, const bitset<8128>& bsta_pre_TP, double res_pre)
{
	if (bsta_pre_TP.any())
	{
		bitset<8128> bs = bsta_TP & ~bsta_pre_TP;
		res_pre += getNNres_fewbit(bs);
		bs = ~bsta_TP & bsta_pre_TP;
		res_pre -= getNNres_fewbit(bs);
		return res_pre;
	}
	else
	{
		double res = 0;
		//ONのビットは高々190個
		//res = getNNres_manybit(bsta_TP);
		bitset<8128> bs = bsta_TP;
		res = getNNres_fewbit(bs);
		return res;
	}
}

//差分でできたら(ry
inline void toTwoPiece(BitsStatus& bsta)
{
	bsta.TwoPiece.reset();
	int shif = 0;
	for (int i = 0; i < 127; i++)
	{
		shif += i;
		if (!bsta.bit.test(i)) continue;
		for (int j = 0; j <= i; j++)
		{
			if (!bsta.bit.test(j)) continue;
			bsta.TwoPiece.set(shif + j);
		}
	}
}
inline void dead_tobit(BitsStatus& bsta, Dead d, uint8_t a)
{
	if (d == 1)
		bsta.bit.set(36 + a);
	else if (d == 2)
		bsta.bit.set(78 + a);
	else if (d == 3)
		bsta.bit.set(120 + a);
}
//差分でできたら(ry
inline void toBitsStatus(BitsStatus& bsta, const Board& board)
{
	bsta.bit.reset();

	bsta.bit |= to36bit(board.myblue);
	//use:36
	bsta.bit |= (bitset<128>)to36bit(board.myred) << 42;
	//use:78
	bsta.bit |= (bitset<128>)to36bit(board.en_mix) << 84;
	//use:120

	dead_tobit(bsta, board.dead_enblue, 0);
	dead_tobit(bsta, board.dead_enred + board.dead_en, 1);
	dead_tobit(bsta, board.dead_myblue, 2);
	dead_tobit(bsta, board.dead_myred, 3);

	if (board.kill)
		bsta.bit.set(40);
	if (board.escape)
		bsta.bit.set(41);

	bsta.bit.set(126);

	toTwoPiece(bsta);
}