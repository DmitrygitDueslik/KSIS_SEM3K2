#include <winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <cstring>
#pragma comment(lib,"ws2_32.lib")
using namespace std;

DWORD WINAPI ThreadFunc ( LPVOID param )
{
	SOCKET s2 = *(SOCKET*)param;
	delete (SOCKET*)param;

	char buf[100], buf1[100];
	int n;

	while ( (n = recv ( s2, buf, sizeof ( buf ) - 1, 0 )) > 0 ) {
		buf[n] = '\0';

		int k = n;
		if ( k % 4 == 0 && k > 0 ) {
			int j = 0;

			for ( int i = k / 2; i < k; i++ ) buf1[i] = buf[j++];
			for ( int i = 0; i < k / 2; i++ ) buf1[i] = buf[j++];
			buf1[k] = '\0';
			memcpy ( buf, buf1, k + 1 );
		}
		cout << buf << endl;
		Sleep ( 5000 );
		send ( s2, buf, k, 0 );
	}
	closesocket ( s2 );
	return 0;
}

int main ( )
{
	WSADATA wsaData;
	if ( WSAStartup ( MAKEWORD ( 2, 2 ), &wsaData ) != 0 )return 1;
	SOCKET s = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( s == INVALID_SOCKET ) { WSACleanup ( ); return 1; }

	sockaddr_in local_addr;
	memset ( &local_addr, 0, sizeof ( local_addr ) );
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons ( 1280 );
	local_addr.sin_addr.s_addr = htonl ( INADDR_ANY );

	if ( bind ( s, (sockaddr*)&local_addr, sizeof ( local_addr ) ) == SOCKET_ERROR ) {
		cout << "bind failed: " << WSAGetLastError ( ) << endl;
		closesocket ( s ); WSACleanup ( ); return 1;
	}
	if ( listen ( s, SOMAXCONN ) == SOCKET_ERROR ) {
		cout << "listen: " << WSAGetLastError ( ) << endl;
		closesocket ( s ); WSACleanup ( ); return 1;
	}

	cout << "Server receive ready" << endl << endl;

	while ( true ) {
		sockaddr_in client_addr;
		int client_addr_size = sizeof ( client_addr );

		SOCKET client_socket = accept ( s, (sockaddr*)&client_addr, &client_addr_size );
		if ( client_socket == INVALID_SOCKET ) {
			cout << "accept: " << WSAGetLastError ( ) << endl;
			break;
		}
		//Каждому потоку
		SOCKET* arg = new SOCKET ( client_socket );
		HANDLE h = CreateThread ( NULL, 0, ThreadFunc, arg, 0, NULL );
		if ( h ) {
			CloseHandle ( h );
		}
		else {
			closesocket ( client_socket );
			delete arg;
		}
	}
	closesocket ( s );
	WSACleanup ( );
	return 0;
}