#include "QotD.hpp"

QotD::QotD(std::string filename) {
	std::ifstream file;
	file.open(filename);
	if (file.is_open()) {
		std::string line;
		std::string running = "";

		while (std::getline(file, line)) {
			if (line != "%") {
				running = running + line + "\n";
			}
			else {
				quotes.push_back(running);
				running = "";
			}
		}
	}

	srand(time(NULL));
}

std::string QotD::getRandomQuote() { return quotes[rand() % quotes.size()]; }