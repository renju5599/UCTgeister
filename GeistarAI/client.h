#pragma once
// visualstudio�ł�tcp�ʐM�ŕK�v
#pragma comment(lib, "wsock32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>

// linux�ł�tcp�ʐM�ŕK�v
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <netdb.h>
// #include <unistd.h>

using namespace std;

// miyo/geister_server�ɑ΂���N���C�A���g
namespace client
{
    void Send(int dstSocket, string str = "");                      // �T�[�o�ɕ�����𑗂�
    string Recv(int dstSocket);                                     // �T�[�o���當������󂯎��
    bool openPort(int &dstSocket, int port = -1, string dest = ""); // �|�[�g���J��
    void closePort(int &dstSocket);                                 // �|�[�g�����
}
