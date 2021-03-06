#include "Game.h"
#include "weight.h"
#include "ColorGuess.h"

namespace Game_
{
	PieceNum piecenum[64] = {};
	Point pieces[16] = {};

	//sĖæŠŠt Ė true
	bool startWith(string& s, string t)
	{
		for (int i = 0; i < t.length(); i++)
		{
			if (i >= s.length() || s[i] != t[i])
				return false;
		}
		return true;
	}

	//Q[ĖIđŧč
	int isEnd(string s)
	{
		if (startWith(s, "WON"))
			return WON;
		if (startWith(s, "LST"))
			return LST;
		if (startWith(s, "DRW"))
			return DRW;
		return 0;
	}

	// IđĖīö
	string getEndInfo(string recv_msg)
	{
		if (startWith(recv_msg, "DRW"))
			return "draw";

		int i, Rnum = 0, Bnum = 0, rnum = 0, bnum = 0;
		const int baius = 4;

		for (i = 0; i < 16; i++)
		{
			int x = recv_msg[baius + 3 * i] - '0';
			int y = recv_msg[baius + 3 * i + 1] - '0';
			char type = recv_msg[baius + 3 * i + 2];

			if (0 <= x && x < 6 && 0 <= y && y < 6)
			{
				if (type == 'R')
					Rnum++;
				if (type == 'B')
					Bnum++;
				if (type == 'r')
					rnum++;
				if (type == 'b')
					bnum++;
			}
		}

		if (startWith(recv_msg, "WON"))
		{
			if (Rnum == 0)
				return "won taken R";
			if (bnum == 0)
				return "won taked b";
			return "won escaped B";
		}

		if (rnum == 0)
			return "lost taked r";
		if (Bnum == 0)
			return "lost taken B";
		return "lost escaped b";
	}

	// ÔîĖzu(_)
	string setPosition()
	{
		string redName_ = "ABCDEFGH";
		//string redName_ = "ABCD";
		random_device seed_gen;
		mt19937 engine(seed_gen());
		shuffle(redName_.begin(), redName_.end(), engine);
		string redName = redName_.substr(4);
		//string redName = redName_.substr(0, 3);
		//redName_ = redName_.substr(3, 1) + "EFGH";
		//shuffle(redName_.begin(), redName_.end(), engine);
		//redName += redName_.substr(0, 1);
		sort(redName.begin(), redName.end());
		cout << redName << endl;
		return "SET:" + redName;
	}

	// Q[ðsĪ
	int playgame(int port, string destination, int AI_kind)
	{
		int dstSocket;
		string initPosition;
		int result;

		if (!client::openPort(dstSocket, port, destination))
			return 0;
		initPosition = setPosition();
		client::Recv(dstSocket);
		client::Send(dstSocket, initPosition);
		client::Recv(dstSocket);

		const Recieve StartStr = "MOV?14R24R34R44R15B25B35B45B41u31u21u11u40u30u20u10u"; //úÕĘ

		UCT_simple::UCT Tree(StartStr);
		UCT_RF::UCT TreeRF(StartStr);
		UCT_RFall::UCT TreeRFall(StartStr);
		UCT_RootRF::UCT TreeRootRF(StartStr);
		UCT_Afterstates::UCT TreeAfterstates(StartStr);

		Recieve recieve;
		Send send;

		//NNĖdÝðĮÝÞ
		if (AI_kind != 0)
			NN::setweight();

		Red::init();

		//
		while (true)
		{
			recieve = client::Recv(dstSocket); // T[o[ĐįĖķņðóŊæé
			result = Game_::isEnd(recieve);    // Q[ĖIđŧč
			if (result)
				break;                     // IđĩÄ―įJčÔĩēŊé

			toPieceNum(piecenum, recieve);
			toPieces(pieces, recieve);
			////ÔîŠÖW
			Red::setAfterEnemy(recieve);
			Red::setEval();
			cout << "ÔÁÛģ" << endl;
			for (int i = 0; i < 64; i++)
			{
				if (Game_::piecenum[i] == -1)
					cout << 0 << "\t";
				else if(Game_::piecenum[i] < 8)
					cout << Red::eval[Game_::piecenum[i]] << "*\t";
				else
					cout << Red::eval[Game_::piecenum[i]] << "\t";
				if (i % 8 == 7) cout << endl;
			}
			////

			if (AI_kind == 0)
			{
				Tree.SetNode(recieve);         //óŊæÁ―ķņĘčÉZbg
				Tree.Search();                 //Tõ
				printf("Finish Search\n");
				Tree.PrintStatus();
				UCT_simple::NodeNum move = Tree.Choice();  //TõĘÉíđÄrootĐįm[hðIð
				send = Tree.MoveNode(move);    //Iðĩ―m[hÉJÚĩAT[o[ÉéķņðóŊæé
			}
			else if (AI_kind == 1)
			{
				TreeRF.SetNode(recieve);         //óŊæÁ―ķņĘčÉZbg
				TreeRF.Search();                 //Tõ
				printf("Finish Search\n");
				TreeRF.PrintStatus();
				UCT_RF::NodeNum move = TreeRF.Choice();  //TõĘÉíđÄrootĐįm[hðIð
				send = TreeRF.MoveNode(move);    //Iðĩ―m[hÉJÚĩAT[o[ÉéķņðóŊæé
			}
			else if (AI_kind == 2)
			{
				TreeRootRF.SetNode(recieve);         //óŊæÁ―ķņĘčÉZbg
				TreeRootRF.Search();                 //Tõ
				printf("Finish Search\n");
				TreeRootRF.PrintStatus();
				UCT_RootRF::NodeNum move = TreeRootRF.Choice();  //TõĘÉíđÄrootĐįm[hðIð
				send = TreeRootRF.MoveNode(move);    //Iðĩ―m[hÉJÚĩAT[o[ÉéķņðóŊæé
			}
			else if (AI_kind == 3)
			{
				TreeAfterstates.SetNode(recieve);         //óŊæÁ―ķņĘčÉZbg
				TreeAfterstates.Search();                 //Tõ
				printf("Finish Search\n");
				TreeAfterstates.PrintStatus();
				UCT_Afterstates::NodeNum move = TreeAfterstates.Choice();  //TõĘÉíđÄrootĐįm[hðIð
				send = TreeAfterstates.MoveNode(move);    //Iðĩ―m[hÉJÚĩAT[o[ÉéķņðóŊæé
			}
			else if (AI_kind == 4)
			{
				TreeRFall.SetNode(recieve);         //óŊæÁ―ķņĘčÉZbg
				TreeRFall.Search();                 //Tõ
				printf("Finish Search\n");
				TreeRFall.PrintStatus();
				UCT_RFall::NodeNum move = TreeRFall.Choice();  //TõĘÉíđÄrootĐįm[hðIð
				send = TreeRFall.MoveNode(move);    //Iðĩ―m[hÉJÚĩAT[o[ÉéķņðóŊæé
			}

			cout << send << endl;
			client::Send(dstSocket, send); //T[o[Éķņðé
			client::Recv(dstSocket);       // ACKĖóM

			//ÔîŠÖW
			Red::setAfterMe(send);
			Red::setEvalafterMe();
			//
		}

		client::closePort(dstSocket);
		return result;
	}
}