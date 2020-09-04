#include "helper.h"
#include "log.h"
#include <sstream>
#include <iostream>

std::vector<std::string> Helper::splitStringByColon(std::string s) {
	return splitString(s, ':');
}

std::vector<std::string> Helper::splitString(std::string s, char a) {
	std::vector<std::string> ret;
	// Convert the input string into a string stream, so that getline can be used on it
	std::stringstream ss(s);
	while (ss.good()) {
		std::string t;
		// Use getline to stop reading at a character
		getline(ss, t, a);
		// remove trailing and leading whitespaces
		while (t.size() > 1 && t.at(0) == ' ') { t.erase(t.begin()); }
		while (t.size() > 1 && t.at(t.size() - 1) == ' ') { t.erase(t.end()); }
		ret.push_back(t);
	}
	return ret;
}

std::vector<std::string> Helper::splitString_enforceCount(std::string s, char a, int len, std::string loc, std::string err) {
	std::vector<std::string> ret = splitString(s, a);
	if (ret.size() != len) {
		QUIT_ERR(loc, err);
	}
	return ret;
}
