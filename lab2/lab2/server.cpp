#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <cstring>
#pragma comment(lib,"ws2_32.lib")
using namespace std;

int main(void) {
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);

	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		cout << "socket() failed: " << WSAGetLastError() << endl;
		WSACleanup();return 1;

	}
		struct sockaddr_in ad;
		ad.sin_port = htons(1024);
		ad.sin_family = AF_INET;
		ad.sin_addr.s_addr = htonl(INADDR_ANY);

		BOOL reuse = TRUE;
		setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
			(const char*)&reuse, sizeof(reuse));
		if (bind(s, (struct sockaddr*)&ad, sizeof(ad))
			== SOCKET_ERROR) {
			cout<<"bind() failed: "<<WSAGetLastError()<<endl;
			closesocket(s);WSACleanup();return 1;
		}

		char b[200];
		sockaddr_in from;
		int fromlen = sizeof(from);

		while (true) {   
			int rv = recvfrom(s, b, sizeof(b) - 1, 0,
				(struct sockaddr*)&from, &fromlen);
			if (rv == SOCKET_ERROR) {
				cout << "recvfrom failed:"
					<< WSAGetLastError() << endl;
				continue;
			}
			b[rv] = '\0';
			cout << b << endl;
			int count = 0;
			for (int i = 0; i < rv; i++) {
				if (b[i] == '@') {
					count++;
					if (count % 2 == 0) b[i] = '#';
				}
			}

			sendto(s, b, rv, 0, (struct sockaddr*)&from,
				fromlen);
		}
		closesocket(s);
		WSACleanup();
		return 0;
	}
