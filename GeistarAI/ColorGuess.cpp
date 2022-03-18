
#include "ColorGuess.h"
#include "method.h"
#include "Game.h"

namespace Red
{
	int eval[16];	//赤度
	Board after_me, after_enemy;
}

void Red::init()
{
	after_me.my = 0;
	for (int i = 0; i < 16; i++)
	{
		eval[i] = 2;
	}
	
}

void Red::setAfterMe(string str)
{
	int8_t dir[4] = { -8, 1, 8, -1 };
	char dirchar[4] = { 'N', 'E', 'S', 'W' };
	int i;
	for (i = 0; i < 4; i++)
	{
		if (dirchar[i] == str[6])
			break;
	}
	after_me = after_enemy;
	toNextBoard(after_me, toBit(Game_::pieces[str[4] - 'A'] - dir[i]), toBit(Game_::pieces[str[4] - 'A']), 0);
	assert(after_me.my != after_enemy.my);
}
void Red::setAfterEnemy(string str)	//MOV?14B24B34R44B15R25B35R45R41u31u21u11u40u30u20u10u
{
	after_enemy = toBoard(str);
}

void Red::setEval()
{
	if (Red::after_me.my == 0) return;
	for (int i = 8; i < 16; i++)
	{
		if (Red::after_me.pieces[i] == 63 && eval[i] != 0)
			eval[i] = 0;
	}
	//敵に接していると赤
	//ゴールできない奴が戻ろうとすると青
	//ゴールできる奴が動かなかった。もしくは遠ざかった。
	BitBoard prev = Red::after_me.enemy & ~Red::after_enemy.enemy;
	BitBoard next = Red::after_enemy.enemy & ~Red::after_me.enemy;
	BitBoard BigRed = 0, BigBlue = 0, SmallRed = 0, SmallBlue = 0;
	if (onPiece(Red::after_me.my, getNextPosBB(prev)))
	{
		if (!onPiece(Red::after_enemy.my, getNextPosBB(next)))
		{
			SmallBlue |= next;
		}
	}
	for (BitBoard bb = Red::after_enemy.enemy; bb != 0; offBottomBB(bb))
	{
		BitBoard b = getBottomBB(bb);
		if (onPiece(Red::after_enemy.my, getNextPosBB(b)))
		{
			SmallRed |= b;
		}
	}
	int me_dist_L = 100, me_dist_R = 100;
	int me_goal_dist = 100;
	for (BitBoard bb = Red::after_me.my; bb != 0; offBottomBB(bb))
	{
		BitBoard b = getBottomBB(bb);
		chmin(me_dist_L, distance(ENGOAL_L, b));
		chmin(me_dist_R, distance(ENGOAL_R, b));
		chmin(me_goal_dist, distance(MYGOAL_L, b));
		chmin(me_goal_dist, distance(MYGOAL_R, b));
	}
	for (BitBoard bb = Red::after_me.enemy; bb != 0; offBottomBB(bb))
	{
		BitBoard b = getBottomBB(bb);
		int dist_l = distance(ENGOAL_L, b), dist_r = distance(ENGOAL_R, b);
		assert(dist_l != dist_r);
		//if (min(dist_l,dist_r) > me_goal_dist)
		//	continue;
		if (dist_l < me_dist_L)
		{
			if (b == prev && distance(ENGOAL_L, next) < dist_l)
				SmallBlue |= next;
			else if (b == prev)
				BigRed |= next;
			else
				BigRed |= b;
		}
		if (dist_r < me_dist_R)
		{
			if (b == prev && distance(ENGOAL_R, next) < dist_r)
				SmallBlue |= next;
			else if (b == prev)
				BigRed |= next;
			else
				BigRed |= b;
		}
	}
	if (y(prev) < y(next))
	{
		SmallBlue |= next;
	}
	//BlueとRed変数からplayoutに使うランダムのウェイトに変換したい
	bool countstop = false;	//カンストしたら赤を確定させて、他を赤じゃなくさせる
	for(BitBoard b = Red::after_enemy.enemy; b != 0; offBottomBB(b))
	{
		BitBoard bb = getBottomBB(b);
		assert(Game_::piecenum[toPoint(bb)] >= 8 && Game_::piecenum[toPoint(bb)] < 16);
		if (onPiece(BigRed, bb))
		{
			eval[Game_::piecenum[toPoint(bb)]] *= 16;
			if (eval[Game_::piecenum[toPoint(bb)]] >= 65536)
			{
				eval[Game_::piecenum[toPoint(bb)]] = 65536;
				//countstop = true;
			}
		}
		else if (onPiece(SmallRed, bb))
		{
			eval[Game_::piecenum[toPoint(bb)]] *= 2;
			if (eval[Game_::piecenum[toPoint(bb)]] >= 65536)
			{
				eval[Game_::piecenum[toPoint(bb)]] = 65536;
				//countstop = true;
			}
		}
		else if (onPiece(SmallBlue, bb))
		{
			eval[Game_::piecenum[toPoint(bb)]] /= 2;
			if (eval[Game_::piecenum[toPoint(bb)]] < 1)
				eval[Game_::piecenum[toPoint(bb)]] = 1;
		}
		else if (onPiece(BigBlue, bb))
		{
			eval[Game_::piecenum[toPoint(bb)]] /= 16;
			if (eval[Game_::piecenum[toPoint(bb)]] < 1)
				eval[Game_::piecenum[toPoint(bb)]] = 1;
		}
	}
	if (countstop)
	{
		for (BitBoard b = Red::after_enemy.enemy; b != 0; offBottomBB(b))
		{
			BitBoard bb = getBottomBB(b);
			if (eval[Game_::piecenum[toPoint(bb)]] >= 65536)
				continue;
			eval[Game_::piecenum[toPoint(bb)]] = 1;
		}
	}
}

void Red::setEvalafterMe()
{
	if (Red::after_enemy.my == 0) return;
	for (int i = 0; i < 8; i++)
	{
		if (Red::after_me.pieces[i] == 63 && eval[i] != 0)
			eval[i] = 0;
	}
	//敵に接していると赤
	//ゴールできない奴が戻ろうとすると青
	//ゴールできる奴が動かなかった。もしくは遠ざかった。
	BitBoard prev = Red::after_enemy.my & ~Red::after_me.my;
	BitBoard next = Red::after_me.my & ~Red::after_enemy.my;
	BitBoard BigRed = 0, BigBlue = 0, SmallRed = 0, SmallBlue = 0;
	//if (onPiece(Red::after_enemy.enemy, getNextPosBB(prev)))
	//{
	//	if (!onPiece(Red::after_me.enemy, getNextPosBB(next)))
	//	{
	//		SmallBlue |= next;
	//	}
	//}
	//for (BitBoard bb = Red::after_me.my; bb != 0; offBottomBB(bb))
	//{
	//	BitBoard b = getBottomBB(bb);
	//	if (onPiece(Red::after_me.enemy, getNextPosBB(b)))
	//	{
	//		SmallRed |= b;
	//	}
	//}
	int en_dist_L = 100, en_dist_R = 100;
	int en_goal_dist = 100;
	for (BitBoard bb = Red::after_me.enemy; bb != 0; offBottomBB(bb))
	{
		BitBoard b = getBottomBB(bb);
		chmin(en_dist_L, distance(MYGOAL_L, b));
		chmin(en_dist_R, distance(MYGOAL_R, b));
		chmin(en_goal_dist, distance(ENGOAL_L, b));
		chmin(en_goal_dist, distance(ENGOAL_R, b));
	}
	for (BitBoard bb = Red::after_me.my; bb != 0; offBottomBB(bb))
	{
		BitBoard b = getBottomBB(bb);
		int dist_l = distance(MYGOAL_L, b), dist_r = distance(MYGOAL_R, b);
		assert(dist_l != dist_r);
		//if (min(dist_l,dist_r) > me_goal_dist)
		//	continue;
		if (dist_l < en_dist_L)
		{
			if (b == next && distance(MYGOAL_L, prev) > dist_l)
				;// SmallBlue |= next;
			else
				BigRed |= b;
		}
		if (dist_r < en_dist_R)
		{
			if (b == next && distance(MYGOAL_R, prev) > dist_r)
				;// SmallBlue |= next;
			else
				BigRed |= b;
		}
	}
	//if (y(prev) > y(next))
	//{
	//	BigBlue |= next;
	//}
	//BlueとRed変数からplayoutに使うランダムのウェイトに変換したい
	bool countstop = false;	//カンストしたら赤を確定させて、他を赤じゃなくさせる
	for (BitBoard b = Red::after_me.my; b != 0; offBottomBB(b))
	{
		BitBoard bb = getBottomBB(b);
		assert(Game_::piecenum[toPoint(bb)] >= 0 && Game_::piecenum[toPoint(bb)] < 8);
		if (onPiece(BigRed, bb))
		{
			eval[Game_::piecenum[toPoint(bb)]] *= 8;
			if (eval[Game_::piecenum[toPoint(bb)]] >= 65536)
			{
				eval[Game_::piecenum[toPoint(bb)]] = 65536;
				//countstop = true;
			}
		}
		else if (onPiece(SmallRed, bb))
		{
			eval[Game_::piecenum[toPoint(bb)]] *= 2;
			if (eval[Game_::piecenum[toPoint(bb)]] >= 65536)
			{
				eval[Game_::piecenum[toPoint(bb)]] = 65536;
				//countstop = true;
			}
		}
		else if (onPiece(SmallBlue, bb))
		{
			eval[Game_::piecenum[toPoint(bb)]] /= 2;
			if (eval[Game_::piecenum[toPoint(bb)]] < 1)
				eval[Game_::piecenum[toPoint(bb)]] = 1;
		}
		else if (onPiece(BigBlue, bb))
		{
			eval[Game_::piecenum[toPoint(bb)]] /= 8;
			if (eval[Game_::piecenum[toPoint(bb)]] < 1)
				eval[Game_::piecenum[toPoint(bb)]] = 1;
		}
	}
	if (countstop)
	{
		for (BitBoard b = Red::after_me.my; b != 0; offBottomBB(b))
		{
			BitBoard bb = getBottomBB(b);
			if (eval[Game_::piecenum[toPoint(bb)]] >= 65536)
				continue;
			eval[Game_::piecenum[toPoint(bb)]] = 1;
		}
	}
}