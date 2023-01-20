#pragma once

#include <iostream>
#include <string>

#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

#define MAX_BUFFER_SIZE (4096)

// Forward declaration
class TcpListener;

// Callback to data received
typedef void (*MessageReceivedHandler)(TcpListener* listener, int socketId, std::string msg);

class TcpListener {
public:

	TcpListener(std::string ipAddress, int port, MessageReceivedHandler handler);
	~TcpListener();

	void sendMessage(int clientSocket, std::string message);
	bool init();
	void run();
	void cleanup();

private:

	SOCKET createSocket();
	SOCKET waitForConnection(SOCKET listening);

	std::string m_ipAddress;
	int m_port;
	MessageReceivedHandler m_messageReceivedHandler;
};