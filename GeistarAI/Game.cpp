#include "Game.h"

namespace Game_
{
    //sの先頭がt ⇔ true
    bool startWith(string &s, string t)
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
        string redName = "ABCDEFGH";
        random_device seed_gen;
        mt19937 engine(seed_gen());
        shuffle(redName.begin(), redName.end(), engine);
        redName = redName.substr(4);
        sort(redName.begin(), redName.end());
        return "SET:" + redName;
    }

    // ゲームを行う
    int playgame(int port, string destination)
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
        UCT Tree(StartStr);
        Recieve recieve;
        Send send;

        //試合
        while (true)
        {
            recieve = client::Recv(dstSocket); // サーバーからの文字列を受け取る
            result = Game_::isEnd(recieve);    // ゲームの終了判定
            if (result)
                break;                     // 終了してたら繰り返し抜ける
            Tree.SetNode(recieve);         //受け取った文字列通りにセット
            Tree.Search();                 //探索
						printf("Finish Search\n");
						Tree.PrintStatus();
            NodeNum move = Tree.Choice();  //探索結果に合わせてrootからノードを選択
            send = Tree.MoveNode(move);    //選択したノードに遷移し、サーバーに送る文字列を受け取る
						cout << send << endl;
            client::Send(dstSocket, send); //サーバーに文字列を送る
            client::Recv(dstSocket);       // ACKの受信
        }

        client::closePort(dstSocket);
        return result;
    }
}
