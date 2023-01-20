#include "TcpListener.hpp"

TcpListener::TcpListener(std::string ipAddress, int port, MessageReceivedHandler handler) : m_ipAddress(ipAddress), m_port(port), m_messageReceivedHandler(handler) { }

TcpListener::~TcpListener() {
	cleanup();
}

// Send a message to the specified client
void TcpListener::sendMessage(SOCKET clientSocket, std::string message) {
	send(clientSocket, message.c_str(), (int)message.size() + 1, 0);
}

// Initialize winsocket
bool TcpListener::init() {
	WSADATA data;
	WORD ver = MAKEWORD(2, 2);

	int wsInit = WSAStartup(ver, &data);
	if (wsInit != 0) {
		std::cerr << "Cant initialize winsock! Quitting" << std::endl;
		return false;
	}
	return wsInit == 0;
}

// Main processing loop
void TcpListener::run() {
	char buffer[MAX_BUFFER_SIZE];

	std::cout << "Waiting for connection..." << std::endl;

	while (true) {
		SOCKET listening = createSocket();
		if (listening == INVALID_SOCKET) {
			break;
		}

		SOCKET client = waitForConnection(listening);
		if (client != INVALID_SOCKET) {
			closesocket(listening);

			int bytesReceived = 0;
			do {
				memset(buffer, 0, MAX_BUFFER_SIZE);
				bytesReceived = recv(client, buffer, MAX_BUFFER_SIZE, 0);
				if (bytesReceived == 0 || bytesReceived == -1) {
					std::cout << "Client disconnected. Waiting for connection..." << std::endl;
					break;
				}

				std::cout << "Client: " << std::string(buffer, 0, bytesReceived) << std::endl;

				if (m_messageReceivedHandler != NULL) {
					m_messageReceivedHandler(this, client, std::string(buffer, 0, bytesReceived));
				}

			} while (bytesReceived > 0);

			closesocket(client);
		}
	}
}

// Clean up after using the service
void TcpListener::cleanup() {
	// Cleanup winsock
	WSACleanup();
}

// Create a socket
SOCKET TcpListener::createSocket() {
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);

	if (listening != INVALID_SOCKET) {
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(m_port);
		inet_pton(AF_INET, m_ipAddress.c_str(), &hint.sin_addr);

		int bindOk = bind(listening, (sockaddr*)&hint, sizeof(hint));
		if (bindOk != SOCKET_ERROR) {
			int listenOk = listen(listening, SOMAXCONN);
			if (listenOk == SOCKET_ERROR) {
				std::cerr << "ERROR: Cant listen socket!" << std::endl;
				return INVALID_SOCKET;
			}
		}
	}
	else {
		std::cerr << "ERROR: Cant create socket!" << std::endl;
		return INVALID_SOCKET;
	}

	return listening;
}

// Wait for a connection
SOCKET TcpListener::waitForConnection(SOCKET listening) {
	sockaddr_in clientInfo;
	int clientSize = sizeof(clientInfo);

	SOCKET client = accept(listening, (sockaddr*)&clientInfo, &clientSize);

	if (client == INVALID_SOCKET) {
		std::cerr << "Invalid client socket! Quitting" << std::endl;
		return INVALID_SOCKET;
	}

	char host[NI_MAXHOST]; // Client's remote name
	char service[NI_MAXSERV]; // Service (i.e. port) the client is connect on

	memset(host, 0, NI_MAXHOST);
	memset(service, 0, NI_MAXSERV);

	if (getnameinfo((sockaddr*)&clientInfo, sizeof(clientInfo), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
		std::cout << host << " connected on port " << service << std::endl;
	}
	else {
		inet_ntop(AF_INET, &clientInfo.sin_addr, host, NI_MAXHOST);
		std::cout << host << " connected on port " << ntohs(clientInfo.sin_port) << std::endl;
	}

	return client;
}