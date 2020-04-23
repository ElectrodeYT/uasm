#include "helper.h"
#include <sstream>

std::vector<std::string> Helper::splitStringByColon(std::string s) {
	std::vector<std::string> ret;
	// Convert the input string into a string stream, so that getline can be used on it
	std::stringstream ss(s);
	while (ss.good()) {
		std::string t;
		getline(ss, t, ':');
		ret.push_back(t);
	}
	return ret;
}
