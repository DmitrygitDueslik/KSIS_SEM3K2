#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <string>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

struct Detail
{
	const char* zavod;
	double cost;
	const char* date;
};

Detail parts[] = {
	{ "Minsk Tractor Works", 1250.00, "12.09.2026" },
	{ "BelAZ",                980.50, "12.09.2026" },
	{ "MAZ",                  450.00, "12.09.2026" },
	{ "Gomselmash",          1730.00, "15.09.2026" },
	{ "Integral",             620.75, "15.09.2026" },
	{ "Atlant",              2100.00, "20.09.2026" },
	{ "Horizont",             860.00, "20.09.2026" },
	{ "BelAZ",               1500.00, "01.10.2026" },
};
const int N = 8;

DWORD WINAPI ThreadFunc ( LPVOID param )
{
	SOCKET s2 = *(SOCKET*)param;
	delete (SOCKET*)param;

	char buf[100];
	int n;
	while ( (n = recv ( s2, buf, sizeof ( buf ) - 1, 0 )) > 0 ) {
		buf[n] = '\0';
		cout << "Zapros: " << buf << endl;

		double min = parts[0].cost;
		for ( int i = 1; i < N; i++ )
			if ( parts[i].cost < min ) min = parts[i].cost;

		string answer = "";
		for ( int i = 0; i < N; i++ ) {
			if ( strcmp ( parts[i].date, buf ) == 0 && parts[i].cost > min ) {
				char line[100];
				snprintf ( line, sizeof ( line ), "%s  %.2f  %s\n",
					parts[i].zavod, parts[i].cost, parts[i].date );
				answer += line;
			}
		}
		if ( answer == "" ) answer = "Net takih komplektuyushchih\n";

		//  Отправка сначала длины  (4 байта) потом сам ответ
		int len = (int)answer.size ( );
		int netlen = htonl ( len );
		send ( s2, (char*)&netlen, sizeof ( netlen ), 0 );
		send ( s2, answer.c_str ( ), len, 0 );
	}
	closesocket ( s2 );
	return 0;
}

int main ( )
{
	WSADATA wsaData;
	if ( WSAStartup ( MAKEWORD ( 2, 2 ), &wsaData ) != 0 ) return 1;
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
	printf ( "%2s  %-20s %9s  %s\n", "N", "Zavod", "Stoimost", "Data" );
	for ( int i = 0; i < N; i++ )
		printf ( "%2d  %-20s %9.2f  %s\n", i + 1, parts[i].zavod, parts[i].cost, parts[i].date );
	cout << endl;
	while ( true ) {
		sockaddr_in client_addr;
		int client_addr_size = sizeof ( client_addr );
		SOCKET client_socket = accept ( s, (sockaddr*)&client_addr, &client_addr_size );
		if ( client_socket == INVALID_SOCKET ) {
			cout << "accept: " << WSAGetLastError ( ) << endl;
			break;
		}
		// Каждому потоку
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