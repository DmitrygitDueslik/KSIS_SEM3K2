#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstring>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

int main ( )
{
	WSADATA wsaData;
	if ( WSAStartup ( MAKEWORD ( 2, 2 ), &wsaData ) != 0 ) return 1;
	SOCKET s = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( s == INVALID_SOCKET ) { WSACleanup ( ); return 1; }

	sockaddr_in dest_addr;
	memset ( &dest_addr, 0, sizeof ( dest_addr ) );
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons ( 1280 );
	inet_pton ( AF_INET, "127.0.0.1", &dest_addr.sin_addr );

	if ( connect ( s, (sockaddr*)&dest_addr, sizeof ( dest_addr ) ) == SOCKET_ERROR ) {
		cout << "connect: " << WSAGetLastError ( ) << endl;
		closesocket ( s ); WSACleanup ( ); return 1;
	}

	char buf[100];
	while ( true ) {
		cout << "Vvedite datu postavki (dd.mm.gggg):" << endl;
		cin.getline ( buf, sizeof ( buf ) );
		if ( !cin.good ( ) || buf[0] == '\0' ) break;
		send ( s, buf, (int)strlen ( buf ), 0 );

		//  Сначала получаем длину  (4 байта)
		int netlen;
		if ( recv ( s, (char*)&netlen, sizeof ( netlen ), 0 ) <= 0 ) break;
		int len = ntohl ( netlen );

		//  читаем отве пока не придёт весь
		char answer[1000];
		int got = 0;
		while ( got < len ) {
			int n = recv ( s, answer + got, len - got, 0 );
			if ( n <= 0 ) break;
			got += n;
		}
		answer[got] = '\0';
		cout << "Otvet servera:" << endl << answer << endl;
	}
	shutdown ( s, SD_SEND );
	closesocket ( s );
	WSACleanup ( );
	return 0;
}