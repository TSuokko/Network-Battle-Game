// UDP_Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>

#pragma comment (lib, "ws2_32.lib")

#define IP_ADDRESS "127.0.0.1"
#define PORT "2315"
#define DEFAULT_BUFLEN 512

struct PlayerClient {
	int id;
	SOCKET socket;
	char receivedMessage[DEFAULT_BUFLEN];

};

int process_PlayerClient(PlayerClient &new_player) {

	while (true)
	{
		memset(new_player.receivedMessage, 0, DEFAULT_BUFLEN);

		if (new_player.socket != 0)
		{
			int iResult = recv(new_player.socket, new_player.receivedMessage, DEFAULT_BUFLEN, 0);

			if (iResult != SOCKET_ERROR)
				std::cout << new_player.receivedMessage << std::endl;
			else
			{
				std::cout << "recv() failed with error: " << WSAGetLastError() << std::endl;
				break;
			}
		}
	}
	if (WSAGetLastError() == WSAECONNRESET)
		std::cout << "The server has disconnected" << std::endl;

	return 0;
}

int main()
{
	
	//Initialization
	WSAData wsaData;
	addrinfo *result = NULL, *ptr = NULL, hints;
	std::string sent_message = "";
	PlayerClient player = { INVALID_SOCKET, -1, ""};
	int iResult = 0;
	std::string serverMessage;

	std::cout << "Initializing player client.\n";
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup() failed with error: " << iResult << std::endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	std::cout << "Connecting to server...\n";

	// Resolve the server address and port
	iResult = getaddrinfo(IP_ADDRESS, PORT, &hints, &result);
	if (iResult != 0) {
		std::cout << "getaddrinfo() failed with error: " << iResult << std::endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	//connect to address
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		player.socket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (player.socket == INVALID_SOCKET) {
			std::cout << "socket() failed with error: " << WSAGetLastError() << std::endl;
			WSACleanup();
			system("pause");
			return 1;
		}
		// Connect to server.
		iResult = connect(player.socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(player.socket);
			player.socket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (player.socket == INVALID_SOCKET) {
		std::cout << "Unable to connect to server!" << std::endl;
		WSACleanup();
		system("pause");
		return 1;
	}

	std::cout << "A successful connection has been made\n\nPlease type 'commands' for the list of available commands!\n";

	//Obtain id from server for this client;
	recv(player.socket, player.receivedMessage, DEFAULT_BUFLEN, 0);
	serverMessage = player.receivedMessage;

	if (serverMessage != "Server is full")
	{
		player.id = atoi(player.receivedMessage);

		std::thread m_thread(process_PlayerClient, player);

		while (true)
		{
			std::getline(std::cin, sent_message);
			iResult = send(player.socket, sent_message.c_str(), strlen(sent_message.c_str()), 0);

			if (iResult <= 0)
			{
				std::cout << "send() failed: " << WSAGetLastError() << std::endl;
				break;
			}
		}
		//Shutdown the connection since no more data will be sent
		m_thread.detach();
	}
	else {
		std::cout << player.receivedMessage << std::endl;
	}
	std::cout << "Shutting down socket." << std::endl;
	iResult = shutdown(player.socket, SD_SEND);

	if (iResult == SOCKET_ERROR) {
		std::cout << "shutdown() failed with error: " << WSAGetLastError() << std::endl;
		closesocket(player.socket);
		WSACleanup();
		system("pause");
		return 1;
	}

	closesocket(player.socket);
	WSACleanup();
	system("pause");
	return 0;
}
