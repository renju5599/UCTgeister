#include "Game.h"

namespace Game_
{
    //s�̐擪��t �� true
    bool startWith(string &s, string t)
    {
        for (int i = 0; i < t.length(); i++)
        {
            if (i >= s.length() || s[i] != t[i])
                return false;
        }
        return true;
    }

    //�Q�[���̏I������
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

    // �I���̌���
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

    // �ԋ�̔z�u(�����_��)
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

    // �Q�[�����s��
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

        const Recieve StartStr = "MOV?14R24R34R44R15B25B35B45B41u31u21u11u40u30u20u10u"; //�����Ֆ�
        UCT Tree(StartStr);
        Recieve recieve;
        Send send;

        //����
        while (true)
        {
            recieve = client::Recv(dstSocket); // �T�[�o�[����̕�������󂯎��
            result = Game_::isEnd(recieve);    // �Q�[���̏I������
            if (result)
                break;                     // �I�����Ă���J��Ԃ�������
            Tree.SetNode(recieve);         //�󂯎����������ʂ�ɃZ�b�g
            Tree.Search();                 //�T��
						printf("Finish Search\n");
						Tree.PrintStatus();
            NodeNum move = Tree.Choice();  //�T�����ʂɍ��킹��root����m�[�h��I��
            send = Tree.MoveNode(move);    //�I�������m�[�h�ɑJ�ڂ��A�T�[�o�[�ɑ��镶������󂯎��
						cout << send << endl;
            client::Send(dstSocket, send); //�T�[�o�[�ɕ�����𑗂�
            client::Recv(dstSocket);       // ACK�̎�M
        }

        client::closePort(dstSocket);
        return result;
    }
}
