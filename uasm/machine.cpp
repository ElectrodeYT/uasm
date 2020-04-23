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
		if (line.at(0) == "inst") {
			// instruction definition
			Instruction new_inst;
			new_inst.decodeInstLine(line);

			std::cout << "Instruction name: " << new_inst.mnemonic << "\nInstruction bits: ";
			for (int i = 0; i < new_inst.instruction.size(); i++) {
				std::cout << new_inst.instruction.at(i);
			}
			std::cout << "\n";
			// add the instruction to the machine
			machine.instructions.push_back(new_inst);
		}
	}

	return machine;
}

#pragma region Instructions
void Machine::Instruction::decodeInstLine(std::vector<std::string> line) {
	if (line.empty() || line.at(0) != "inst") {
		std::cerr << "[Instruction] [BUG] Instruction line argument name is not 'inst'!\n";
		return;
	}
	line.erase(line.begin()); // erase the "inst" entry
	// decode the arguments aka. the part of the machine file that looks like the mnemonic
	std::vector<std::string> instruction_arguments = Helper::splitStringByColon(line.at(0));
	std::vector<char> argument_char; // argument names, used for converting the file bits to the class bits
	mnemonic = instruction_arguments.at(0); // first entry is the mnenmonic
	for (int i = 1; i < instruction_arguments.size(); i++) {
		// get the argument bit count
		std::vector<std::string> arg = Helper::splitStringByColon(line.at(i)); // conveniently because i is offset by 1 for the mnemonic we can just use it lol
		if (arg.empty() || arg.size() == 1) {
			std::cerr << "[Instruction] Did not find valid instruction argument bit count!\n";
			exit(-1);
		}
		// convert the argument bit count
		int bit_count = 0;
		if (arg.at(1) == "r") { bit_count = 0xffff; } // 0xffff is the "bit count" for a register-only value
		else { bit_count = std::stoi(arg.at(1)); }
		arguments.push_back(bit_count);
		// save the argument "name"
		argument_char.push_back(arg.at(0).at(0));
	}
	// get the bits, should be the second to last
	std::string bits = line.at(line.size() - 2);
	instruction.clear();
	for (int i = 0; i < bits.size(); i++) {
		if (bits.at(i) == '0') {
			// a zero
			instruction.push_back(0);
		} else if (bits.at(i) == '1') {
			// a one
			instruction.push_back(1);
		} else {
			// find the argument "id"
			for (int x = 0; x < argument_char.size(); x++) {
				//std::cout << "[DEBUG] checking argument_char " << x << ", " << argument_char.at(x) << "\n";
				if (bits.at(i) == argument_char.at(x)) {
					instruction.push_back(x + 2);
					x = argument_char.size();
				}
			}
		}
	}
	// get instruction type
	instruction_type = std::stoi(line.at(line.size() - 1));
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
#pragma endregion

