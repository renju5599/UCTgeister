///////////////////
//
// UCTが動けば動く?
// 
// Game.cpp のplaygame()でUCTを使ってます
//
///////////////////

#include <iostream>
#include <string>
// #include <map> //getEndInfoを組み込もうと思ったけどまた後で

#include "Game.h"

int main()
{
    // サーバ接続用
    string destination; // IPアドレス
    int port;           // ポート番号
    // 対戦管理用
    int n;        // 対戦回数
    int res;      // 対戦結果
    int win = 0;  // 勝った回数
    int draw = 0; // 引き分け回数
    int lose = 0; // 負けた回数
	int AI_kind = 0;

    // 対戦の設定
    cout << "対戦回数 ポート番号 IPアドレス ↓" << endl;
    cin >> n >> port >> destination;
	cout << "使用するAI{ [0]:UCT, [1]:UCT_RF, [2]:UCT_onlyRootRF, [3]:UCT_Afterstates, [4]:UCT_RFall, [5]:UCT_RedDic_nomal, [6]:UCT_RedDic_murasaki, [7]:UCTandAB, [8]:UCTRFandAB,\n [9]:UCTAokiBeta, [10]:UCTAokiBeta_ABcheck, **[11]:UCTAokiBeta_3Purple** } ↓" << endl;
    cin >> AI_kind;
    if (AI_kind == 8 || AI_kind == 9 || AI_kind == 10 || AI_kind == 11)
    {
        cout << "切り替えターン (0 - 300)↓best:30" << endl;
        cin >> Game_::ver_num;
    }

    // 対戦
    for (int i = 0; i < n; i++)
    {
        Game_::turn = (port == 10000 ? 0 : 1);
        res = Game_::playgame(port, destination, AI_kind); // 対戦
        if (res == Game_::WON)
            win++; // 勝ち
        if (res == Game_::DRW)
            draw++; // 引き分け
        if (res == Game_::LST)
            lose++; // 負け

        cout << "途中経過　(勝ち - 負け - 引き分け) = (" << win << " - " << lose << " - " << draw << ")" << endl;
        Sleep(1);
    }

    // 結果出力
    cout << "(勝ち - 負け - 引き分け) = (" << win << " - " << lose << " - " << draw << ")" << endl;

    return 0;
}
