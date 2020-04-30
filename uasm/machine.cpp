#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "machine.h"
#include "helper.h"
#include "log.h"

// file version identifier
#define UASM_MACHINE_FILE_VERSION "1.0"


using namespace Machine;

// Turn a file into a MachineFile
MachineFile Machine::readMachine(std::string path) {
	MachineFile machine = MachineFile();
	int warn_count = 0; // warning counter
	// Check if path contains a extension
	if (path.find('.') == std::string::npos) {
		path = path += ".mach";
	}
	std::ifstream ifstream(path);
	if (!ifstream.is_open()) { std::cerr << "[Machine] Machine file could not be opened!\n"; return machine; }
	std::string s;
	// Get Rules and other, not really that important info
	while (std::getline(ifstream, s)) {
		std::vector<std::string> v = convertFileLineToVector(s);
		// check if a line is a rule line
		if (!v.empty() && v.size() > 1 && v.at(0) == "rule") {
			std::vector<std::string> r = Helper::splitStringByColon(v.at(1));
			if (!r.empty() || !r.size() == 1) {
				Rule rule;
				rule.name = r.at(0);
				rule.data = r.at(1);
				machine.rules.push_back(rule);
			}
		}
		// check if a line defines the file version
		if (!v.empty() && v.size() > 1 && v.at(0) == "uasm") {
			machine.version = v.at(1);
		}
		// check if a line defines the machine (long) name
		if (!v.empty() && v.size() > 1 && v.at(0) == "name") {
			machine.name = v.at(1);
		}
	}
	if (machine.version == "") { std::cerr << "[Machine] [WARN] File does not define a version!\n"; warn_count++; }
	else if (machine.version != UASM_MACHINE_FILE_VERSION) { std::cerr << "[Machine] [WARN] File version file incorrect! File Version: " << machine.version << "\n"; warn_count++; }
	if (machine.name == "") { std::cerr << "[Machine] [WARN] File does not define a machine name!\n"; warn_count++; } else { std::cout << "[Machine] Machine name: " << machine.name << "\n"; }
	// close and reopen the file
	ifstream.close();
	ifstream.open(path, std::ios::in);
	// Get Instructions and registers
	while (std::getline(ifstream, s)) {
		// split the file line into a more easily workable vector
		std::vector<std::string> line = convertFileLineToVector(s);
		// check if the line is invalid/empty/just a comment
		if (line.empty()) { continue; }
		// check if the file declares a instruction
		if (line.at(0) == "inst") {
			// instruction definition
			Instruction new_inst;
			// decode the line
			new_inst.decodeInstLine(line);
			// add the instruction to the machine
			machine.instructions.push_back(new_inst);
		}
		// check if the file declares a register
		if (line.at(0) == "reg") {
			// get the register nameand bit size
			std::vector<std::string> reg = Helper::splitStringByColon(line.at(1));
			if (reg.empty() || reg.size() == 1) {
				std::cerr << "[Register] Register line invalid!\n";
				exit(-1);
			}
			// cannt be bothered to check for correctness, this will do for now
			Register new_reg;
			new_reg.name = reg.at(0);
			new_reg.bitsize = std::stoi(reg.at(1));
			machine.registers.push_back(new_reg);
		}
	}
	if (warn_count != 1) {
		std::cout << "[Machine] Finished reading machine file, " << warn_count << " Warnings\n";
	} else {
		std::cout << "[Machine] Finished reading machine file, " << warn_count << " Warning\n";
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
	// calculate the instruction length
	instruction_length = instruction.size() / 8;
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
	// seperate data by commas
	std::vector<std::string> arguments;
	std::stringstream data_sstream(data);
	while (data_sstream.good()) {
		std::string t;
		getline(data_sstream, t, ',');
		arguments.push_back(t);
	}
	// remove all leading and trailing whitespaces and construct return vector
	ret.push_back(name);
	for (int i = 0; i < arguments.size(); i++) {
		ret.push_back(arguments.at(i).substr(arguments.at(i).find_first_not_of(' '), arguments.at(i).find_last_not_of(' '))); // disgusting, isnt it?
	}

	return ret;
}
#pragma endregion

