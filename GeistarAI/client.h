#pragma once
// visualstudioでのtcp通信で必要
#pragma comment(lib, "wsock32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>

// linuxでのtcp通信で必要
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <netdb.h>
// #include <unistd.h>

using namespace std;

// miyo/geister_serverに対するクライアント
namespace client
{
    void Send(int dstSocket, string str = "");                      // サーバに文字列を送る
    string Recv(int dstSocket);                                     // サーバから文字列を受け取る
    bool openPort(int &dstSocket, int port = -1, string dest = ""); // ポートを開く
    void closePort(int &dstSocket);                                 // ポートを閉じる
}
