#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
using namespace std;
#pragma comment(lib, "ws2_32.lib")


int main() {
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);

	struct sockaddr_in peer;
	peer.sin_family = AF_INET;
	peer.sin_port = htons(1280);
	//т.к.клиент и сервер на одном компьютере
	peer.sin_addr.s_addr = inet_addr("127.0.0.1");


	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

	connect(s, (struct sockaddr*)&peer, sizeof(peer));

	char buf[255], b[255];
	cout << "Enter the string,please: ";
	cin.getline(buf, 100, '\n');

	send(s, buf, (int)strlen(buf), 0);
	int n = recv(s, b, sizeof(b) - 1, 0);
	if (n > 0) {
		b[n] = '\0';
		cout << b << endl;
	}
	closesocket(s);
	WSACleanup();
	return 0;
}