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
	if (!ifstream.is_open()) { QUIT_ERR("Machine", "Machine file could not be opened! File name: " << path); }
	std::string s;

	// List of instructions we still need to process
	std::vector<std::vector<std::string>> instr_list;

	// Read machine file, and pass simple things now
	while (std::getline(ifstream, s)) {
		std::vector<std::string> file_line = convertFileLineToVector(s);
		if (file_line.size() == 0) { continue; } // Skip if the line is empty
		if (file_line.size() < 2) { QUIT_ERR_LINE("Machine", "Not enough arguments", s); } // Throw an error if the line has too few arguments
		// Check some simple commands
		std::string name = file_line[0];
		if (name == "uasm") {
			// ignore for now
		} else if (name == "name") {
			LOG_MSG("Machine", "Machine name: " << file_line[1]);
		} else if (name == "reg") {
			// Split register definition
			std::vector<std::string> split = Helper::splitString_enforceCount(file_line[1], ':', 2, "Machine", "Not enough arguments");
			Register reg;
			reg.name = split[0];
			reg.bitsize = std::stoi(split[1], 0, 0);
			machine.registers.push_back(reg);
		} else if (name == "rule") {
			// Split register definition
			std::vector<std::string> split = Helper::splitString_enforceCount(file_line[1], ':', 2, "Machine", "Not enough arguments");
			Rule rule;
			rule.name = split[0];
			rule.data = split[1];
			machine.rules.push_back(rule);
		} else if (name == "inst") {
			std::vector<std::string> inst_vec;
			// Copy stuff from file_line
			inst_vec.insert(inst_vec.end(), file_line.begin() + 1, file_line.end());
			// Save in instructions
			instr_list.push_back(inst_vec);
		} else if (name == "define") {
			Define define;
			// Split the line
			int line_begin = s.find_first_of(' ');
			int line_length = s.find_last_of(':') - line_begin;
			std::string line = s.substr(line_begin, line_length);
			line = line.substr(line.find_first_not_of(' '), line.find_last_not_of(' ')); // Trim line
			// Get bytes
			int bytes_string_begin = s.find_last_of(':') + 1;
			int bytes_string_length = s.find_last_of(';') - bytes_string_begin;
			std::string bytes_string = s.substr(bytes_string_begin, bytes_string_length);
			bytes_string = bytes_string.substr(s.find_first_not_of(' '), s.find_last_not_of(' '));
			std::vector<std::string> bytes = Helper::splitString(bytes_string, ' ');
			for (int i = 0; i < bytes.size(); i++) {
				try {
					unsigned char byte = std::stoi(bytes[i], 0, 0);
					define.line = line;
					define.bytes.push_back(byte);
				} catch (std::invalid_argument) {
					QUIT_ERR_LINE("Machine", "Invalid number", s);
				}
			}
			machine.defines.push_back(define);
		}
	}

	LOG_MSG("Machine", "Read file, processing instructions");
	// Process all instructions
	for (int i = 0; i < instr_list.size(); i++) {
		Instruction inst;
		// Get instruction name and arguments
		std::vector<std::string> operands = Helper::splitString(instr_list[i][0], ':');
		std::string name = operands[0];
		// Set name in inst
		inst.mnemonic = name;
		// Erase name from operands list
		operands.erase(operands.begin());
		// Erase begining from instruction list
		instr_list[i].erase(instr_list[i].begin());
		// Add operands
		std::vector<char> operands_c; // char of operand, used for creating the instruction bits
		for (int x = 0; x < operands.size(); x++) {
			std::vector<std::string> operand = Helper::splitString_enforceCount(instr_list[i][0], ':', 2, "Machine", "Instruction Operand Invalid!");
			operands_c.push_back(operand[0][0]);
			int bit_count = 0;
			if (operand[1] == "r") {
				bit_count = 0xFFFF; // Register
			} else {
				bit_count = std::stoi(operand[1], 0, 0);
			}
			inst.arguments.push_back(bit_count); // Store bit count
			instr_list[i].erase(instr_list[i].begin()); // Remove that from the instruction list
		}
		std::string bits = instr_list[i][0];
		if (bits.length() % 8 != 0) { QUIT_ERR("Machine", "Bits is not a multiple of 8!"); }
		int mode = std::stoi(instr_list[i][1], 0, 0);
		// Set Instruction bits array
		for (int x = 0; x < bits.length(); x++) {
			switch (bits[x]) {
				case '0': inst.instruction.push_back(0); break; // Always bit 0
				case '1': inst.instruction.push_back(1); break; // Always bit 1
				// Argument
				default: {
					int id = 0;
					for (int y = 0; y < operands_c.size(); y++) {
						if (bits[x] == operands_c[y]) { id = y + 2; break; }
					}
					if (id == 0) { QUIT_ERR("Machine", "Invalid bit set in instruction " << name); }
					inst.instruction.push_back(id);
					break;
				}
			}
		}
		// Reverse bits array
		std::reverse(inst.instruction.begin(), inst.instruction.end());
		// Save instruction
		inst.instruction_length = bits.length() / 8;
		machine.instructions.push_back(inst);
	}
	return machine;
}

#pragma region Helper functions
std::vector<std::string> Machine::convertFileLineToVector(std::string s) {
	std::vector<std::string> ret;
	// Check if the file contains no important symbols
	// Return nothing if it doesnt
	if (s.find(";") == std::string::npos && s.find("#") == std::string::npos) {
		return ret;
	}
	// Check if the file contains a ;
	// Throw an error if not
	if (s.find(";") == std::string::npos) {
		QUIT_ERR_LINE("Machine", "File line invalid!", s);
	}
	// Check if the file contains a #
	// Return nothing if it doesnt
	if (s.find("#") == std::string::npos) {
		return ret;
	}
	// Check if the file contains a :
	// Throw an error if not
	if (s.find(":") == std::string::npos) {
		QUIT_ERR_LINE("Machine", "File line invalid!", s);
	}
	s = s.substr(s.find("#") + 1, s.find(";")); // Get main line
	std::string name = s.substr(0, s.find(":")); // Get name
	std::string cont = s.substr(s.find(":") + 1, s.length()); // Get line content
	cont = cont.substr(cont.find_first_not_of(' '), cont.find_last_not_of(' ')); // Trimm cont
	// Construct ret
	ret.push_back(name);
	std::vector<std::string> cont_split = Helper::splitString(cont, ',');
	ret.insert(ret.end(), cont_split.begin(), cont_split.end());
	// Remove from all strings # and ;
	return ret;
}
#pragma endregion

