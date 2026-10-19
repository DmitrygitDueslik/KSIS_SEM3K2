#include <winsock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

int main() {
	WSADATA w;
	WSAStartup(MAKEWORD(2, 2), &w);
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in peer;
	memset(&peer, 0, sizeof(peer));
	peer.sin_family = AF_INET;
	peer.sin_port = htons(1280);
	inet_pton(AF_INET, "127.0.0.1", &peer.sin_addr);

	if (connect(s, (sockaddr*)&peer, sizeof(peer))
		== SOCKET_ERROR) {
		cout << "Connection failed: " << WSAGetLastError() << endl;
		return 1;
	}

	string big(10000, 'a');

	int sent = send(s, big.c_str(), (int)big.size(), 0);
	cout << "send() vernul: " << sent << endl;

	char reply[256];
	int n = recv(s, reply, sizeof(reply) - 1, 0);
	reply[n > 0 ? n : 0] = '\0';
	cout << "Server otvetil: " << reply << endl;
	cout << "A dolzhen byl: 10000" << endl;

	closesocket(s);
	WSACleanup();
	return 0;
}