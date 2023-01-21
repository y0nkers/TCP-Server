#pragma once

#include <iostream>
#include <string>
#include <sstream>

#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

#define MAX_BUFFER_SIZE (4096)

// Forward declaration
class TcpListener;

// Callback to data received
typedef void (*MessageReceivedHandler)(TcpListener* listener, SOCKET socketId, std::string msg);

// TcpListener class
class TcpListener {
public:

	TcpListener(std::string ipAddress, int port, MessageReceivedHandler handler);
	~TcpListener();

	void sendMessage(SOCKET clientSocket, std::string message);
	bool init();
	void runSingle();
	void runMultiple();
	void cleanup();

private:

	SOCKET createSocket();
	SOCKET waitForConnection(SOCKET listening);

	std::string m_ipAddress;
	int m_port;
	MessageReceivedHandler m_messageReceivedHandler;
};