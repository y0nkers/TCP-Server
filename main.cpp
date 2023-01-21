#include "TcpListener.hpp"
#include "QotD.hpp"
#include <algorithm>

QotD quotes("quotes.txt");

void listenerMessageReceived(TcpListener* listener, SOCKET client, std::string message) {
	std::transform(message.begin(), message.end(), message.begin(), ::tolower);
	if (message == "quote") listener->sendMessage(client, quotes.getRandomQuote());
	else listener->sendMessage(client, message);
}

int main(int argc, char* argv[]) {

	TcpListener server("127.0.0.1", 56789, listenerMessageReceived);

	if (server.init()) {
		server.runMultiple();
	}

	system("pause");
	return 0;
}
