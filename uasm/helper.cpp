#include "helper.h"
#include <sstream>
std::vector<std::string> Helper::splitStringByColon(std::string s) {
	return splitString(s, ':');
}
std::vector<std::string> Helper::splitString(std::string s, char a) {
	std::vector<std::string> ret;
	// Convert the input string into a string stream, so that getline can be used on it
	std::stringstream ss(s);
	while (ss.good()) {
		std::string t;
		getline(ss, t, a);
		// remove trailing and leading whitespaces
		while (t.size() > 1 && t.at(0) == ' ') { t.erase(t.begin()); }
		while (t.size() > 1 && t.at(t.size() - 1) == ' ') { t.erase(t.end()); }
		ret.push_back(t);
	}
	return ret;
}
