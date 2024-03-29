#include "Game.h"
#include "weight.h"
#include "ColorGuess.h"

namespace Game_
{
	PieceNum piecenum[64] = {};
	Point pieces[16] = {};
	int turn = 0;
	int ver_num = 0;
	bool purple = false;
	bool ABcheck = false;
	Board gameboard;

	//sの先頭がt ⇔ true
	bool startWith(string& s, string t)
	{
		for (int i = 0; i < t.length(); i++)
		{
			if (i >= s.length() || s[i] != t[i])
				return false;
		}
		return true;
	}

	//ゲームの終了判定
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

	// 終了の原因
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

	// 赤駒の配置(ランダム)
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

	// ゲームを行う
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

		const Recieve StartStr = "MOV?14R24R34R44R15B25B35B45B41u31u21u11u40u30u20u10u"; //初期盤面

		UCT_simple::UCT Tree(StartStr);
		UCT_RF::UCT TreeRF(StartStr);
		UCT_RFall::UCT TreeRFall(StartStr);
		UCT_RootRF::UCT TreeRootRF(StartStr);
		UCT_Afterstates::UCT TreeAfterstates(StartStr);
		UCT_RedDec::UCT TreeRedDec(StartStr);
		UCTandAlphaBeta_RedDec::UCTandAlphaBeta TreeUCT_AB(StartStr);
		UCTwithRFandABRed::UCTandAlphaBeta TreeUCTRF_AB(StartStr);
		UCTrootRFandABRed::UCTandAlphaBeta TreeUCTrootRF_AB(StartStr);

		Recieve recieve;
		Send send;

		//NNの重みを読み込む
		if (AI_kind != 0)
			NN::setweight();

		Red::init();

		ABcheck = false;

		//試合
		while (true)
		{
			recieve = client::Recv(dstSocket); // サーバーからの文字列を受け取る
			if (recieve.back() != '\n')
			{
				client::closePort(dstSocket);
				client::openPort(dstSocket, port, destination);
				continue;
			}
			result = Game_::isEnd(recieve);    // ゲームの終了判定
			if (result)
				break;                     // 終了してたら繰り返し抜ける

			toPieceNum(piecenum, recieve);
			toPieces(pieces, recieve);
			gameboard = toBoard(recieve);
			////赤駒推測関係
			if (AI_kind != 5 && AI_kind != 6 && AI_kind != 7)
			{
				Red::setAfterEnemy(recieve);
				Red::setEval();
				cout << "赤っぽさ" << endl;
				for (int i = 0; i < 64; i++)
				{
					if (Game_::piecenum[i] == -1)
						cout << 0 << "\t";
					else if (Game_::piecenum[i] < 8)
						cout << Red::eval[Game_::piecenum[i]] << "*\t";
					else
						cout << Red::eval[Game_::piecenum[i]] << "\t";
					if (i % 8 == 7) cout << endl;
				}
			}
			////

			if (AI_kind == 0)
			{
				Tree.SetNode(recieve);         //受け取った文字列通りにセット
				Tree.Search();                 //探索
				printf("Finish Search\n");
				Tree.PrintStatus();
				UCT_simple::NodeNum move = Tree.Choice();  //探索結果に合わせてrootからノードを選択
				send = Tree.MoveNode(move);    //選択したノードに遷移し、サーバーに送る文字列を受け取る
			}
			else if (AI_kind == 1)
			{
				TreeRF.SetNode(recieve);         //受け取った文字列通りにセット
				TreeRF.Search();                 //探索
				printf("Finish Search\n");
				TreeRF.PrintStatus();
				UCT_RF::NodeNum move = TreeRF.Choice();  //探索結果に合わせてrootからノードを選択
				send = TreeRF.MoveNode(move);    //選択したノードに遷移し、サーバーに送る文字列を受け取る
			}
			else if (AI_kind == 2)
			{
				TreeRootRF.SetNode(recieve);         //受け取った文字列通りにセット
				TreeRootRF.Search();                 //探索
				printf("Finish Search\n");
				TreeRootRF.PrintStatus();
				UCT_RootRF::NodeNum move = TreeRootRF.Choice();  //探索結果に合わせてrootからノードを選択
				send = TreeRootRF.MoveNode(move);    //選択したノードに遷移し、サーバーに送る文字列を受け取る
			}
			else if (AI_kind == 3)
			{
				TreeAfterstates.SetNode(recieve);         //受け取った文字列通りにセット
				TreeAfterstates.Search();                 //探索
				printf("Finish Search\n");
				TreeAfterstates.PrintStatus();
				UCT_Afterstates::NodeNum move = TreeAfterstates.Choice();  //探索結果に合わせてrootからノードを選択
				send = TreeAfterstates.MoveNode(move);    //選択したノードに遷移し、サーバーに送る文字列を受け取る
			}
			else if (AI_kind == 4)
			{
				TreeRFall.SetNode(recieve);         //受け取った文字列通りにセット
				TreeRFall.Search();                 //探索
				printf("Finish Search\n");
				TreeRFall.PrintStatus();
				UCT_RFall::NodeNum move = TreeRFall.Choice();  //探索結果に合わせてrootからノードを選択
				send = TreeRFall.MoveNode(move);    //選択したノードに遷移し、サーバーに送る文字列を受け取る
			}
			else if (AI_kind == 5)
			{
				TreeRedDec.SetNode(recieve);         //受け取った文字列通りにセット
				TreeRedDec.Search(0);                 //探索
				printf("Finish Search\n");
				TreeRedDec.PrintStatus();
				UCT_RedDec::NodeNum move = TreeRedDec.Choice();  //探索結果に合わせてrootからノードを選択
				TreeRedDec.changeRed_pos(move);
				send = TreeRedDec.MoveNode(move);    //選択したノードに遷移し、サーバーに送る文字列を受け取る
			}
			else if (AI_kind == 6)
			{
				TreeRedDec.SetNode(recieve);         //受け取った文字列通りにセット
				TreeRedDec.Search(1);                 //探索
				printf("Finish Search\n");
				TreeRedDec.PrintStatus();
				UCT_RedDec::NodeNum move = TreeRedDec.Choice();  //探索結果に合わせてrootからノードを選択
				TreeRedDec.changeRed_pos(move);
				send = TreeRedDec.MoveNode(move);    //選択したノードに遷移し、サーバーに送る文字列を受け取る
			}
			else if (AI_kind == 7)
			{
				TreeUCT_AB.SetNode(recieve);
				if (turn < 20)
				{
					printf("UCT\n");
					TreeUCT_AB.Search();
					printf("Finish Search\n");
					TreeUCT_AB.PrintStatus();
					UCTandAlphaBeta_RedDec::NodeNum move = TreeUCT_AB.Choice();  //探索結果に合わせてrootからノードを選択
					TreeUCT_AB.changeRed_pos(move);
					send = TreeUCT_AB.MoveNode(move);    //選択したノードに遷移し、サーバーに送る文字列を受け取る
				}
				else
				{
					printf("AlphaBeta\n");
					send = TreeUCT_AB.Search_AB();
					printf("Finish Search\n");
				}
			}
			else if (AI_kind == 8)
			{
				TreeUCTRF_AB.SetNode(recieve);
				if (turn < ver_num)
				{
					printf("UCTwithRF\n");
					TreeUCTRF_AB.Search();
					printf("Finish Search\n");
					TreeUCTRF_AB.PrintStatus();
					UCTandAlphaBeta_RedDec::NodeNum move = TreeUCTRF_AB.Choice();  //探索結果に合わせてrootからノードを選択
					TreeUCTRF_AB.changeRed_pos(move);
					send = TreeUCTRF_AB.MoveNode(move);    //選択したノードに遷移し、サーバーに送る文字列を受け取る
				}
				else
				{
					printf("AlphaBeta\n");
					send = TreeUCTRF_AB.Search_AB();
					printf("Finish Search\n");
				}
			}
			else if (AI_kind == 9)
			{
				TreeUCTrootRF_AB.SetNode(recieve);
				if (turn < ver_num)
				{
					printf("UCTrootRF\n");
					TreeUCTrootRF_AB.Search();
					printf("Finish Search\n");
					TreeUCTrootRF_AB.PrintStatus();
					UCTandAlphaBeta_RedDec::NodeNum move = TreeUCTrootRF_AB.Choice();  //探索結果に合わせてrootからノードを選択
					TreeUCTrootRF_AB.changeRed_pos(move);
					send = TreeUCTrootRF_AB.MoveNode(move);    //選択したノードに遷移し、サーバーに送る文字列を受け取る
				}
				else
				{
				AlphaBeta9:
					printf("AlphaBeta\n");
					send = TreeUCTrootRF_AB.Search_AB();
					printf("Finish Search\n");
				}
			}
			else if (AI_kind == 10)
			{
				TreeUCTrootRF_AB.SetNode(recieve);
				if (!ABcheck && turn < ver_num)
				{
					printf("AlphaBeta Check...\n");
					int check = TreeUCTrootRF_AB.Check_AB();
					if (check > 50000)
					{
						ABcheck = true;
						goto AlphaBeta10;
					}
					printf("Finish Check\n");

					printf("UCTrootRF\n");
					TreeUCTrootRF_AB.Search();
					printf("Finish Search\n");
					TreeUCTrootRF_AB.PrintStatus();
					UCTandAlphaBeta_RedDec::NodeNum move = TreeUCTrootRF_AB.Choice();  //探索結果に合わせてrootからノードを選択
					TreeUCTrootRF_AB.changeRed_pos(move);
					send = TreeUCTrootRF_AB.MoveNode(move);    //選択したノードに遷移し、サーバーに送る文字列を受け取る
				}
				else
				{
				AlphaBeta10:
					printf("AlphaBeta\n");
					send = TreeUCTrootRF_AB.Search_AB();
					printf("Finish Search\n");
				}
			}
			else if (AI_kind == 11)
			{
				purple = true;
				TreeUCTrootRF_AB.SetNode(recieve);
				if (!ABcheck && turn < ver_num)
				{
					printf("AlphaBeta Check...\n");
					int check = TreeUCTrootRF_AB.Check_AB();
					if (check > 50000)
					{
						ABcheck = true;
						goto AlphaBeta11;
					}
					printf("Finish Check\n");

					printf("UCTrootRF\n");
					TreeUCTrootRF_AB.Search();
					printf("Finish Search\n");
					TreeUCTrootRF_AB.PrintStatus();
					UCTandAlphaBeta_RedDec::NodeNum move = TreeUCTrootRF_AB.Choice();  //探索結果に合わせてrootからノードを選択
					TreeUCTrootRF_AB.changeRed_pos(move);
					send = TreeUCTrootRF_AB.MoveNode(move);    //選択したノードに遷移し、サーバーに送る文字列を受け取る
				}
				else
				{
				AlphaBeta11:
					printf("AlphaBeta\n");
					send = TreeUCTrootRF_AB.Search_AB();
					printf("Finish Search\n");
				}
			}

			cout << send << endl;
			client::Send(dstSocket, send); //サーバーに文字列を送る
			client::Recv(dstSocket);       // ACKの受信

			//赤駒推測関係
			if (AI_kind != 5 && AI_kind != 6 && AI_kind != 7)
			{
				Red::setAfterMe(send);
				Red::setEvalafterMe();
			}
			//

			turn += 2;
		}

		client::closePort(dstSocket);
		return result;
	}
}