///////////////////
//
// UCT�������Γ���?
// 
// Game.cpp ��playgame()��UCT���g���Ă܂�
//
///////////////////

#include <iostream>
#include <string>
// #include <map> //getEndInfo��g�ݍ������Ǝv�������ǂ܂����

#include "Game.h"

int main()
{
    // �T�[�o�ڑ��p
    string destination; // IP�A�h���X
    int port;           // �|�[�g�ԍ�
    // �ΐ�Ǘ��p
    int n;        // �ΐ��
    int res;      // �ΐ팋��
    int win = 0;  // ��������
    int draw = 0; // ����������
    int lose = 0; // ��������
	int AI_kind = 0;

    // �ΐ�̐ݒ�
    cout << "�ΐ�� �|�[�g�ԍ� IP�A�h���X ��" << endl;
    cin >> n >> port >> destination;
	cout << "�g�p����AI{ [0]:UCT, [1]:UCT_RF, [2]:UCT_onlyRootRF, [3]:UCT_Afterstates, [4]:UCT_RFall, [5]:UCT_RedDic_nomal, [6]:UCT_RedDic_murasaki, [7]:UCTandAB, [8]:UCTRFandAB,\n [9]:UCTAokiBeta, [10]:UCTAokiBeta_ABcheck, **[11]:UCTAokiBeta_3Purple** } ��" << endl;
    cin >> AI_kind;
    if (AI_kind == 8 || AI_kind == 9 || AI_kind == 10 || AI_kind == 11)
    {
        cout << "�؂�ւ��^�[�� (0 - 300)��best:30" << endl;
        cin >> Game_::ver_num;
    }

    // �ΐ�
    for (int i = 0; i < n; i++)
    {
        Game_::turn = (port == 10000 ? 0 : 1);
        res = Game_::playgame(port, destination, AI_kind); // �ΐ�
        if (res == Game_::WON)
            win++; // ����
        if (res == Game_::DRW)
            draw++; // ��������
        if (res == Game_::LST)
            lose++; // ����

        cout << "�r���o�߁@(���� - ���� - ��������) = (" << win << " - " << lose << " - " << draw << ")" << endl;
        Sleep(1);
    }

    // ���ʏo��
    cout << "(���� - ���� - ��������) = (" << win << " - " << lose << " - " << draw << ")" << endl;

    return 0;
}
