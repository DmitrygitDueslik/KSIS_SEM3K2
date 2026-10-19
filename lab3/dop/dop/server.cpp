#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

struct Detail
{
	string zavod;
	double cost;
	string date;
};

vector<Detail> parts = {
	{ "Minsk Tractor Works", 1250.00, "12.09.2026" },
	{ "BelAZ",                980.50, "12.09.2026" },
	{ "MAZ",                  450.00, "12.09.2026" },
	{ "Gomselmash",          1730.00, "15.09.2026" },
	{ "Integral",             620.75, "15.09.2026" },
	{ "Atlant",              2100.00, "20.09.2026" },
	{ "Horizont",             860.00, "20.09.2026" },
	{ "BelAZ",               1500.00, "01.10.2026" },
};
CRITICAL_SECTION cs;

DWORD WINAPI ThreadFunc ( LPVOID param )
{
	SOCKET s2 = *(SOCKET*)param;
	delete (SOCKET*)param;

	char buf[100];
	int n;
	while ( (n = recv ( s2, buf, sizeof ( buf ) - 1, 0 )) > 0 ) {
		buf[n] = '\0';
		cout << "Zapros: " << buf << endl;
		string answer = "";
		char line[200];

		if ( strncmp ( buf, "add ", 4 ) == 0 ) {
			char z[50], d[20];
			double c;
			if ( sscanf ( buf + 4, "%49[^;];%lf;%19s", z, &c, d ) == 3 ) {
				EnterCriticalSection ( &cs );
				parts.push_back ( { z, c, d } );
				int count = (int)parts.size ( );
				LeaveCriticalSection ( &cs );
				snprintf ( line, sizeof ( line ), "Dobavleno. Zapisey v spiske: %d\n", count );
				answer = line;
			}
			else answer = "Oshibka. Format: add Zavod;stoimost;dd.mm.gggg\n";
		}
		else if ( strncmp ( buf, "del ", 4 ) == 0 ) {
			int num = atoi ( buf + 4 );
			bool ok = false;
			EnterCriticalSection ( &cs );
			if ( num >= 1 && num <= (int)parts.size ( ) ) {
				parts.erase ( parts.begin ( ) + (num - 1) );
				ok = true;
			}
			LeaveCriticalSection ( &cs );
			answer = ok ? "Udaleno\n" : "Net zapisi s takim nomerom\n";
		}
		else {
			EnterCriticalSection ( &cs );
			vector<Detail> copy = parts;
			LeaveCriticalSection ( &cs );

			if ( copy.empty ( ) ) answer = "Spisok pust\n";
			else if ( strcmp ( buf, "list" ) == 0 ) {
				// весь список с номерами
				for ( int i = 0; i < (int)copy.size ( ); i++ ) {
					snprintf ( line, sizeof ( line ), "%d) %s  %.2f  %s\n", i + 1,
						copy[i].zavod.c_str ( ), copy[i].cost, copy[i].date.c_str ( ) );
					answer += line;
				}
			}
			else {
				// поиск по дате (основное задание варианта 5)
				double min = copy[0].cost;
				for ( int i = 1; i < (int)copy.size ( ); i++ )
					if ( copy[i].cost < min ) min = copy[i].cost;

				for ( int i = 0; i < (int)copy.size ( ); i++ ) {
					if ( copy[i].date == buf && copy[i].cost > min ) {
						snprintf ( line, sizeof ( line ), "%s  %.2f  %s\n",
							copy[i].zavod.c_str ( ), copy[i].cost, copy[i].date.c_str ( ) );
						answer += line;
					}
				}
				if ( answer == "" ) answer = "Net takih komplektuyushchih\n";
			}
		}

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
	InitializeCriticalSection ( &cs );

	cout << "Server receive ready" << endl << endl;
	printf ( "%2s  %-20s %9s  %s\n", "N", "Zavod", "Stoimost", "Data" );
	for ( int i = 0; i < (int)parts.size ( ); i++ )
		printf ( "%2d  %-20s %9.2f  %s\n", i + 1, parts[i].zavod.c_str ( ), parts[i].cost, parts[i].date.c_str ( ) );
	cout << endl;

	while ( true ) {
		sockaddr_in client_addr;
		int client_addr_size = sizeof ( client_addr );
		SOCKET client_socket = accept ( s, (sockaddr*)&client_addr, &client_addr_size );
		if ( client_socket == INVALID_SOCKET ) {
			cout << "accept: " << WSAGetLastError ( ) << endl;
			break;
		}
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
	DeleteCriticalSection ( &cs );       // один раз при завершении
	closesocket ( s );
	WSACleanup ( );
	return 0;
}