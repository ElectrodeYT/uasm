#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <exception>
#include "machine.h"
#include "helper.h"
#include "log.h"

// file version identifier
#define UASM_MACHINE_FILE_VERSION "1.0"


using namespace Machine;

// Turn a file into a MachineFile
MachineFile Machine::readMachine(std::vector<std::string> lines) {
	MachineFile machine = MachineFile();

	// List of instructions we still need to process
	std::vector<std::vector<std::string>> instr_list;

	// Read machine file, and pass simple things now
	for(size_t i = 0; i < lines.size(); i++) {
		std::string s = lines[i];
		std::vector<std::string> file_line;
		try {
			file_line = convertFileLineToVector(s);
		} catch (...) {
			LOG_ERR_LINE("Machine", "File line invalid!", s);
			machine.failed = true;
			return machine;
		}
		if (file_line.size() == 0) { continue; } // Skip if the line is empty
		// Throw an error if the line has too few arguments
		if (file_line.size() < 2) {
			LOG_ERR_LINE("Machine", "Not enough arguments", s);
			machine.failed = true;
			return machine;
		}
		// Check some simple commands
		std::string name = file_line[0];
		if (name == "uasm") {
			// ignore for now
		} else if (name == "name") {
			LOG_MSG("Machine", "Machine name: " << file_line[1]);
		} else if (name == "reg") {
			// Split register definition
			std::vector<std::string> split = Helper::splitString_enforceCount(file_line[1], ':', 2);
			if (split.size() == 0) {
				LOG_ERR_LINE("Machine", "Not enough arguments", s);
				machine.failed = true;
				return machine;
			}
			Register reg;
			reg.name = split[0];
			reg.bitsize = std::stoi(split[1], 0, 0);
			machine.registers.push_back(reg);
		} else if (name == "rule") {
			// Split register definition
			std::vector<std::string> split = Helper::splitString_enforceCount(file_line[1], ':', 2);
			if (split.size() == 0) {
				LOG_ERR_LINE("Machine", "Not enough arguments", s);
				machine.failed = true;
				return machine;
			}
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
			int bytes_string_length = s.length() - bytes_string_begin;
			if (s.find(';') != std::string::npos) { bytes_string_length = s.find(';') - bytes_string_begin; } // Check if the line has a comment in it
			std::string bytes_string = s.substr(bytes_string_begin, bytes_string_length);
			bytes_string = Helper::removeTrailingAndLeading(bytes_string, ' ');
			std::vector<std::string> bytes = Helper::splitString(bytes_string, ' ');
			for (size_t i = 0; i < bytes.size(); i++) {
				try {
					unsigned char byte = std::stoi(bytes[i], 0, 0);
					define.line = line;
					define.bytes.push_back(byte);
				} catch (...) {
					LOG_ERR_LINE("Machine", "Invalid number", s);
					machine.failed = true;
					return machine;
				}
			}
			machine.defines.push_back(define);
		}
	}

	LOG_MSG("Machine", "Read file, processing instructions");
	// Process all instructions
	for (size_t i = 0; i < instr_list.size(); i++) {
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
		for (size_t x = 0; x < operands.size(); x++) {
			std::vector<std::string> operand = Helper::splitString_enforceCount(instr_list[i][0], ':', 2);
			if (operand.size() == 0) {
				LOG_ERR("Machine", "Invalid Instruction");
				machine.failed = true;
				return machine;
			}
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
		if (bits.length() % 8 != 0) {
			LOG_ERR("Machine", "Bits is not a multiple of 8!");
			machine.failed = true;
			return machine;
		}
		// Get instruction mode
		try {
			int mode = std::stoi(instr_list[i][1], 0, 0);
			(void)mode;
		} catch (...) {
			LOG_ERR("Machine", "No instruction mode! Instruction name: " << name);
			machine.failed = true;
			return machine;
		}
		// Set Instruction bits array
		for (size_t x = 0; x < bits.length(); x++) {
			switch (bits[x]) {
				case '0': inst.instruction.push_back(0); break; // Always bit 0
				case '1': inst.instruction.push_back(1); break; // Always bit 1
				// Argument
				default: {
					int id = 0;
					for (size_t y = 0; y < operands_c.size(); y++) {
						if (bits[x] == operands_c[y]) { id = y + 2; break; }
					}
					if (id == 0) {
						LOG_ERR("Machine", "Invalid bit set in instruction " << name);
						machine.failed = true;
						return machine;
					}
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

std::vector<std::string> Machine::convertFileLineToVector(std::string s) {
	std::vector<std::string> ret;
	// Check if the file contains a #
	// Return nothing if it doesnt
	if (s.find("#") == std::string::npos) {
		return ret;
	}
	// Check if the file contains a :
	// Throw an error if not
	if (s.find(":") == std::string::npos) {
		throw std::invalid_argument("File line invalid!");
	}
	// Get main file line
	int main_file_line_begin = s.find('#') + 1;
	int main_file_line_length = s.length() - main_file_line_begin + 1;

	// Check if file contains a comment and update main_file_line_begin if it does
	if (s.find(';') != std::string::npos) {
		main_file_line_length = s.find(';') - main_file_line_begin;
	} 
	s = s.substr(main_file_line_begin, main_file_line_length);

	std::string name = s.substr(0, s.find(':')); // Get name
	std::string cont = s.substr(s.find(':') + 1, s.length() - s.find(':')); // Get line content
	cont = cont.substr(cont.find_first_not_of(' '), cont.find_last_not_of(' ')); // Trim cont
	// Construct ret
	ret.push_back(name);
	std::vector<std::string> cont_split = Helper::splitString(cont, ',');
	ret.insert(ret.end(), cont_split.begin(), cont_split.end());
	// Remove from all strings # and ;
	return ret;
}

