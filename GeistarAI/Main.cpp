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
		cout << "�g�p����AI{ [0]:UCT, [1]:UCT_RF, [2]:UCT_onlyRootRF } ��" << endl;
		cin >> AI_kind;

    // �ΐ�
    for (int i = 0; i < n; i++)
    {
        res = Game_::playgame(port, destination, AI_kind); // �ΐ�
        if (res == Game_::WON)
            win++; // ����
        if (res == Game_::DRW)
            draw++; // ��������
        if (res == Game_::LST)
            lose++; // ����
        Sleep(1);
    }

    // ���ʏo��
    cout << "(����, ��������, ����) = (" << win << ", " << draw << ", " << lose << ")" << endl;

    return 0;
}
