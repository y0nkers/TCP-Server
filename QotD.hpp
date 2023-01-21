#pragma once

#include <string>
#include <fstream>
#include <vector>

// Quote of the day class
class QotD {
public:

	QotD(std::string filename);

	std::string getRandomQuote();

private:

	std::vector<std::string> quotes;
};