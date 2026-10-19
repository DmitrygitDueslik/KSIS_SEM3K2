#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;
#pragma comment(lib, "ws2_32.lib")

int main() {
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);


	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

	if (s == INVALID_SOCKET) {
		cout << "socket() failed: " << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}


	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(1280);
	local.sin_addr.s_addr = htonl(INADDR_ANY);

	BOOL reuse = TRUE;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
		(const char*)&reuse, sizeof(reuse));

	if (bind(s, (struct sockaddr*)&local, sizeof(local))
		== SOCKET_ERROR) {
		cout << "bind() failed: " << WSAGetLastError() << endl;
		closesocket(s);WSACleanup();
		return 1;
	}

	int r = listen(s, SOMAXCONN);

	while (true) {
		char buff[255], res[100], b[255], * Res;
		int n;
		//структура определяет удаленный адрес,
		//с которым соединяется сокет
		sockaddr_in remote_addr;
		int size = sizeof(remote_addr);

		SOCKET s2 = accept(s, (struct sockaddr*)&remote_addr, &size);

		while ((n = recv(s2, b, sizeof(b) - 1, 0)) > 0) {
			b[n] = '\0';
			int i = 0;

			for (int i = 0;i < n - 1;i++) {
				int min_idx = i;
				for (int j = i + 1;j < n;j++) {
					if (b[j] < b[min_idx]) {
					min_idx = j;
				}
				}
				char tmp = b[i];
				b[i] = b[min_idx];
				b[min_idx] = tmp;
			}
			send(s2,b,n, 0);
	
		}
		closesocket(s2);
	}
}