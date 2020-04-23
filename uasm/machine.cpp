#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "machine.h"
#include "helper.h"

using namespace Machine;

MachineFile Machine::readMachine(std::string path) {
	MachineFile machine = MachineFile();
	// Check if path contains a extension
	if (path.find('.') == std::string::npos) {
		path = path += ".mach";
	}
	std::ifstream ifstream(path);
	if (!ifstream.is_open()) { std::cerr << "[Machine] Machine file could not be opened!\n"; return machine; }
	std::string s;
	while (std::getline(ifstream, s)) {
		std::vector<std::string> line = convertFileLineToVector(s);
		if (line.empty()) { continue; }
		for (int i = 0; i < line.size(); i++) {
			std::cout << line.at(i);
			if (i + 1 < line.size()) {
				std::cout << "--";
			}
		}
		std::cout << "\n";
	}

	return machine;
}

#pragma region Instructions
void Machine::Instruction::decodeInstLine(std::vector<std::string> line) {
	if (line.empty() || line.at(0) != "inst") {
		std::cerr << "[Instruction] [BUG] Instruction line argument name is not 'inst'!\n";
		return;
	}
	std::vector<std::string> instruction = Helper::splitStringByColon(line.at(1));

}
#pragma endregion


#pragma region Helper functions
std::vector<std::string> Machine::convertFileLineToVector(std::string s) {
	std::vector<std::string> ret;
	if (s.find('#') == std::string::npos) {
		return ret; // This line does not contain anything
	}
	// check if line contains semicolon
	s.erase(s.begin(), s.begin() + s.find('#') + 1);
	if (s.find(';') == std::string::npos) {
		std::cerr << "[Machine] Line does not contain semicolon!\n[Machine] Offending line: " << s << "\n";
		return ret;
	}
	s = s.substr(0, s.find(';'));
	// Get name
	if (s.find(':') == std::string::npos) {
		std::cerr << "[Machine] Line does not contain :!\n[Machine] Offending line: " << s << "\n";
		return ret;
	}
	/// "name" of the line
	std::string name = s.substr(0, s.find(':'));
	std::string data = s.substr(s.find(':') + 1);
	// remove all whitespaces
	data.erase(std::remove_if(data.begin(), data.end(), std::isspace), data.end());
	// seperate data by commas
	std::vector<std::string> arguments;
	std::stringstream data_sstream(data);
	while (data_sstream.good()) {
		std::string t;
		getline(data_sstream, t, ',');
		arguments.push_back(t);
	}

	// construct return vector
	ret.push_back(name);
	for (int i = 0; i < arguments.size(); i++) {
		ret.push_back(arguments.at(i));
	}

	return ret;
}
std::vector<std::string> 
#pragma endregion

