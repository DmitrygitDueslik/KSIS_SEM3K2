#define _WINSOCK_DEPRECATED_NO_WARNINGS	
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <cstring>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

int main(void) {
	char buf[100], b[100];
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return 0;
	}
	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		cout << "socket failed: " << WSAGetLastError() << endl;
		WSACleanup();return 1;
	}
	sockaddr_in add;
	add.sin_family = AF_INET;
	add.sin_port = htons(1024);
	add.sin_addr.s_addr = inet_addr("127.0.0.1");
	int timeout = 3000;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,
		(const char*)&timeout, sizeof(timeout));
	int t = sizeof(add);
	cout << "Enter the string, please" << endl;
	cin.getline(buf, sizeof(buf), '\n');

	sendto(s, buf, (int)strlen(buf), 0,
		(struct sockaddr*)&add, t);

	int rv = recvfrom(s, b, sizeof(b) - 1, 0,
		(struct sockaddr*)&add, &t);
	if (rv == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err == WSAETIMEDOUT)
			cout << "Otvet ne poluchen."
			<< " Server zapushchen?" << endl;
		else
			cout << "recvfrom: " << err << endl;
		closesocket(s); WSACleanup(); return 1;
	}
	b[rv] = '\0';
	cout << b << endl;

	closesocket(s);

	WSACleanup(); return 0;
}

