#include "helper.h"
#include "log.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <exception>

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
		while (t.size() > 1 && t[0] == ' ') { t.erase(t.begin()); }
		while (t.size() > 1 && t[t.size() - 1] == ' ') { t.erase(t.end()); }
		ret.push_back(t);
	}
	return ret;
}

std::vector<std::string> Helper::splitString_enforceCount(std::string s, char a, size_t len) {
	std::vector<std::string> ret = splitString(s, a);
	if (ret.size() != len) {
		// Incorrect size
		ret.clear();
		return ret;
	}
	return ret;
}

std::string Helper::trimString(std::string s) {
	while (s.size() > 1 && s[0] == ' ') { s.erase(s.begin()); }
	while (s.size() > 1 && s[s.size() - 1] == ' ') { s = s.substr(0, s.length() - 1); }
	return s;
}

std::vector<std::string> Helper::readIntoVector(std::ifstream* stream) {
	std::vector<std::string> ret;
	std::string s;
	while (std::getline(stream[0], s)) { ret.push_back(s); }
	return ret;
}
std::string Helper::removeTrailingAndLeading(std::string s, char c = ' ') {
	std::string ret = s;
	while (ret.size() > 1 && ret[0] == c) { ret.erase(ret.begin()); }
	while (ret.size() > 1 && ret[ret.size() - 1] == c) { ret.erase(std::prev(ret.end())); }
	return ret;
}