#include "client.h"

namespace client
{
	// �T�[�o�ɕ�����𑗂�
	void Send(int dstSocket, string str)
	{
		if (str.length() == 0)
		{ //null�����Ȃ�u���́v���󂯕t����
			cin >> str;
		}
		if (str.length() < 2 || str[str.length() - 2] != '\r' || str[str.length() - 1] != '\n')
		{ //\r�������ɂȂ���Βǉ�
			str += '\r';
			str += '\n';
		}
		int byte = send(dstSocket, str.c_str(), str.length(), 0); //������𑗐M
		if (byte <= 0)
		{
			cout << "���M�G���[" << endl;
		}
	}

	// �T�[�o���當������󂯎��
	string Recv(int dstSocket)
	{
		char buffer[10];
		string msg;

		do
		{
			Sleep(10);
			int byte = recv(dstSocket, buffer, 1, 0); //��������M

			if (byte == 0)
				break;
			if (byte < 0)
			{
				cout << "��M�Ɏ��s���܂���" << endl;
				return msg;
			}

			msg += buffer[0];
		} while (msg.length() < 2 || msg[msg.length() - 2] != '\r' || msg[msg.length() - 1] != '\n');
		cout << "��M = " << msg << endl;
		return msg;
	}

	// �|�[�g���J��
	bool openPort(int &dstSocket, int port, string dest)
	{
		// IP �A�h���X�C�|�[�g�ԍ��C�\�P�b�g�Csockaddr_in �\����
		char destination[32];
		struct sockaddr_in dstAddr;
		int PORT;

		// Windows �̏ꍇ
		WSADATA data;
		WSAStartup(MAKEWORD(2, 0), &data);

		// �����A�h���X�̓��͂Ƒ��镶���̓���
		if (port == -1)
		{
			printf("�|�[�g�ԍ��́H�F");
			scanf("%d", &PORT);
		}
		else
		{
			PORT = port;
		}

		if (dest.length() == 0)
		{
			printf("�T�[�o�[�}�V����IP�́H:");
			scanf("%s", destination);
		}
		else
		{
			for (int i = 0; i < dest.size(); i++)
				destination[i] = dest[i];
			destination[dest.size()] = '\0';
		}

		// sockaddr_in �\���̂̃Z�b�g
		memset(&dstAddr, 0, sizeof(dstAddr));
		dstAddr.sin_port = htons(PORT);
		dstAddr.sin_family = AF_INET;
		dstAddr.sin_addr.s_addr = inet_addr(destination);

		// �\�P�b�g�̐���
		dstSocket = socket(AF_INET, SOCK_STREAM, 0);

		//�ڑ�
		if (connect(dstSocket, (struct sockaddr *)&dstAddr, sizeof(dstAddr)))
		{
			printf("%s�@�ɐڑ��ł��܂���ł���\n", destination);
			return false;
		}
		printf("%s �ɐڑ����܂���\n", destination);

		return true;
	}

	// �|�[�g�����
	void closePort(int &dstSocket)
	{
		// Windows �ł̃\�P�b�g�̏I��
		closesocket(dstSocket);
		// close(dstSocket);
		WSACleanup();
	}
}
