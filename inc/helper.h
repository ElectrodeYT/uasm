#pragma once
#include <vector>
#include <string>

namespace Helper {
	std::vector<std::string> splitStringByColon(std::string s);
	std::vector<std::string> splitString(std::string s, char a);
	std::vector<std::string> splitString_enforceCount(std::string s, char a, size_t len);
	std::string trimString(std::string s);
	std::vector<std::string> readIntoVector(std::ifstream* stream);
	std::string removeTrailingAndLeading(std::string s, char c);
}