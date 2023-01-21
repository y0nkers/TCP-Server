#include "TcpListener.hpp"

TcpListener::TcpListener(std::string ipAddress, int port, MessageReceivedHandler handler) : m_ipAddress(ipAddress), m_port(port), m_messageReceivedHandler(handler) {

}

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

// Run single client
void TcpListener::runSingle() {
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
				if (bytesReceived <= 0) {
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

// Run multiple clients
void TcpListener::runMultiple() {
	SOCKET listening = createSocket();
	if (listening == INVALID_SOCKET) {
		return;
	}

	fd_set master;
	FD_ZERO(&master);
	FD_SET(listening, &master);

	while (true) {
		// select() is destructive so we make a copy
		fd_set copy = master;
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < socketCount; ++i) {
			SOCKET sock = copy.fd_array[i];
			if (sock == listening) {
				// Accept new connection
				SOCKET client = waitForConnection(listening);
				
				FD_SET(client, &master);

				std::string welcomeMessage = "Welcome to the chat!\r\n";
				send(client, welcomeMessage.c_str(), welcomeMessage.size() + 1, 0);

				// Broadcast: we have a new connection
				std::ostringstream ss;
				ss << "SOCKET #" << sock << " just connected!" << "\r\n";
				std::string strOut = ss.str();

				for (int i = 0; i < master.fd_count; ++i) {
					SOCKET outSock = master.fd_array[i];
					if (outSock != client) {
						send(outSock, strOut.c_str(), strOut.size() + 1, 0);
					}
				}
			}
			else {
				char buffer[MAX_BUFFER_SIZE];
				memset(buffer, 0, MAX_BUFFER_SIZE);

				// Receive new message
				int bytesReceived = recv(sock, buffer, MAX_BUFFER_SIZE, 0);
				if (bytesReceived <= 0) {
					closesocket(sock);
					FD_CLR(sock, &master);
					std::cout << "Socket #" << sock << " disconnected." << std::endl;
					//break;
				}
				else {
					std::cout << "Socket #" << sock << ": " << std::string(buffer, 0, bytesReceived) << std::endl;

					// Send message to other clients
					std::ostringstream ss;
					ss << "SOCKET #" << sock << ": " << buffer << "\r\n";
					std::string strOut = ss.str();

					for (int i = 0; i < master.fd_count; ++i) {
						SOCKET outSock = master.fd_array[i];
						if (outSock != listening && outSock != sock) {
							send(outSock, strOut.c_str(), strOut.size() + 1, 0);
						}
					}
				}
			}
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

	/*if (getnameinfo((sockaddr*)&clientInfo, clientSize, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
		std::cout << host << " connected on port " << service << std::endl;
	}
	else {
		inet_ntop(AF_INET, &clientInfo.sin_addr, host, NI_MAXHOST);
		std::cout << host << " connected on port " << ntohs(clientInfo.sin_port) << std::endl;
	}*/

	inet_ntop(AF_INET, &clientInfo.sin_addr, host, NI_MAXHOST);
	std::cout << host << " connected on port " << ntohs(clientInfo.sin_port) << std::endl;

	return client;
}