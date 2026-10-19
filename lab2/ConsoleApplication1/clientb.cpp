#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

int main() {
    WSADATA w;
    WSAStartup(MAKEWORD(2, 2), &w);
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    int timeout = 3000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,
        (const char*)&timeout, sizeof(timeout));

    sockaddr_in add;
    memset(&add, 0, sizeof(add));
    add.sin_family = AF_INET;
    add.sin_port = htons(1024);
    add.sin_addr.s_addr = inet_addr("127.0.0.1");

    string big(10000, 'a'); //10 000 букв a
    int sent = sendto(s, big.c_str(),
        (int)big.size(), 0,
        (sockaddr*)&add, sizeof(add));
    cout << "sendto() vernul: " << sent << endl;

    char b[512];
    sockaddr_in from;
    int fromlen = sizeof(from);
    int rv = recvfrom(s, b, sizeof(b) - 1, 0,
        (sockaddr*)&from, &fromlen);
    if (rv == SOCKET_ERROR)
        cout << "recvfrom oshibka: " << WSAGetLastError() << endl;
    else {
        b[rv] = '\0';
        cout << "Otvet: " << b << endl;
    }

    closesocket(s);
    WSACleanup();
    return 0;
}