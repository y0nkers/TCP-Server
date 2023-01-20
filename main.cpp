#include "TcpListener.hpp"

void listenerMessageReceived(TcpListener* listener, int client, std::string message) {
	listener->sendMessage(client, message);
}

int main(int argc, char* argv[]) {

	TcpListener server("127.0.0.1", 56789, listenerMessageReceived);

	if (server.init()) {
		server.run();
	}

	return 0;
}
