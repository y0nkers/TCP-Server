#include <iostream>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

int main(int argc, char* argv[]) {

	// Init winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0) {
		std::cerr << "Cant initialize winsock! Quitting" << std::endl;
		return -1;
	}

	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) {
		std::cerr << "Cant create socket! Quitting" << std::endl;
		return -1;
	}

	// Bind the ip address and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(56789); // host to network short
	hint.sin_addr.S_un.S_addr = INADDR_ANY; // could also use inet_pton

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// Tell winsock the socket if sor listening
	listen(listening, SOMAXCONN);

	// Wait for a connection
	sockaddr_in client;
	int clientSize = sizeof(client);

	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
	if (clientSocket == INVALID_SOCKET) {
		std::cerr << "Invalid client socket! Quitting" << std::endl;
		return -1;
	}

	char host[NI_MAXHOST]; // Client's remote name
	char service[NI_MAXSERV]; // Service (i.e. port) the client is connect on

	memset(host, 0, NI_MAXHOST);
	memset(service, 0, NI_MAXSERV);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
		std::cout << host << " connected on port " << service << std::endl;
	}
	else {
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		std::cout << host << " connected on port " << ntohs(client.sin_port) << std::endl;
	}

	// Close listening socket
	closesocket(listening);

	// While loop: accept and echo message back to client
	char buf[4096];

	while (true) {
		memset(buf, 0, 4096);

		// Wait for client to send data
		int bytesReceived = recv(clientSocket, buf, 4096, 0);
		if (bytesReceived == SOCKET_ERROR) {
			std::cerr << "Error in recv(). Quitting" << std::endl;
			break;
		}

		if (bytesReceived == 0) {
			std::cout << "Client disconnected." << std::endl;
			break;
		}

		std::cout << "Client message: " << buf << std::endl;

		// Echo message back to client
		send(clientSocket, buf, bytesReceived + 1, 0); // + '\0'
	}


	// Close the socket
	closesocket(clientSocket);

	// Cleanup winsock
	WSACleanup();

	return 0;
}
