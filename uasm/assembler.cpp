#include <fstream>
#include <iostream>
#include <bitset>
#include <sstream>
#include "assembler.h"
#include "helper.h"
#include "log.h"

#define BITSET_INSTRUCTION_SIZE 128

Assembler::Assembled Assembler::assembleMachine(Machine::MachineFile machine, std::string path) {
	Assembler::Assembled ret;
	std::vector<unsigned char> prog;
	int prog_seek = 0; // Current "PC" of the assembler
	// get some important rules
	int origin = std::stoi(getRule(machine, "default-origin"), 0, 0);
	int inst_size = std::stoi(getRule(machine, "inst-size"), 0, 0);
	// define some variables
	std::vector<Variable> symbols; // for things like labels and stuff
	std::vector<Instruction> instructions; // instructions
	// create a fstream for the assembly code
	std::fstream file(path, std::ios::in);

	/// Pass 1: calculate the "length" of each line and get the addresses of all labels and stuff
	std::string s;
	while (std::getline(file, s)) {
		// remove everything past #
		if (s.find("#") != std::string::npos) {
			s = s.substr(0, s.find("#"));
		}
		// seperate the string by whitespace
		std::vector<std::string> split = Helper::splitString(s, ' ');
		
		std::string command = split[0]; // Command or mnemonic;
		std::vector<std::string> arguments; // Arguments to instruction
		if (split.size() > 1) {
			// concatante everything past the mnemonic
			std::string arguments_string;
			std::stringstream concat(arguments_string);
			for (int i = 1; i < split.size(); i++) {
				concat << split.at(i);
			}
			arguments = Helper::splitString(concat.str(), ',');
		}
		if (s == "") { continue; }
		// Check if the line is a dot command
		if (s.at(0) == '.') {
			if (split.empty()) { std::cerr << "[Assembler] [BUG] Split string does not contain any entries!\n"; exit(-2); }
			if (split.size() == 1) { QUIT_ERR_LINE("Assembler", "Line invalid!", s); }
			// Label
			// Acts like a symbol
			if (command == ".lbl") { symbols.push_back(Variable(split.at(1), prog.size() + origin)); continue; }
			// UASM Version
			// Not required
			if (command == ".uasm") { continue; } // not required, just a indication to the programmer for the version the source code was made for
			// Change origin
			// Does not change the current offset from 0
			if (command == ".origin") {
				if (split.size() == 1) { QUIT_ERR_LINE("Assembler", "Missing operand: address", s); }
				try {
					origin = std::stoi(split.at(1), 0, 0);
				} catch (std::invalid_argument) {
					QUIT_ERR_LINE("Assembler", "Error decoding origin", s);
				}
				continue;
			}
			// Change current offset from 0
			// Doesnt change the origin
			if (command == ".seek") {
				if (split.size() == 1) { QUIT_ERR_LINE("Assembler", "Missing operand: offset-from-origin", s); }
				LOG_WRN_LINE("Assembler", "Using .seek can cause assembled instructions to be overwritten. It is not recommended to use this.", s);
				try {
					prog_seek = std::stoi(split.at(1), 0, 0);
				} catch (std::invalid_argument) {
					QUIT_ERR_LINE("Assembler", "Error decoding offset-from-origin", s);
				}
				continue;
			}
			// Skip n bytes
			// Value not determined
			if (command == ".skip") {
				if (split.size() == 1) { QUIT_ERR_LINE("Assembler", "Missing operand: offset-from-origin", s); }
				LOG_WRN_LINE("Assembler", "Using .seek can cause assembled instructions to be overwritten. It is not recommended to use this.", s);
				try {
					int amount_to_skip = std::stoi(split.at(1), 0, 0);
					prog_seek += amount_to_skip;
				} catch (std::invalid_argument) {
					QUIT_ERR_LINE("Assembler", "Error decoding offset-from-origin", s);
				}
				continue;
			}
			// Define byte(s)
			// Value determined
			if (command == ".db") {
				LOG_MSG_LINE("Assembler", "Writing bytes", s);
				if (prog_seek > prog.size()) { prog.resize(prog_seek); } // Ensure prog has sufficent capacity
				for (int i = 0; i < arguments.size(); i++) {
					prog.insert(prog.begin() + prog_seek++, std::stoi(arguments[i], 0, 0));
					LOG_MSG("Assembler", "Wrote byte " << std::stoi(arguments[i], 0, 0));
				}
				continue;
			}
			LOG_WRN_LINE("Assembler", "Found invalid dot line, ignoring.", s);
			continue;
		}
		// Check if the line is a define
		bool was_define = false;
		for (int i = 0; i < machine.defines.size(); i++) {
			if (machine.defines[i].line == s) {
				// Add in the bytes
				prog.insert(prog.begin() + prog_seek, machine.defines[i].bytes.begin(), machine.defines[i].bytes.end());
				prog_seek += machine.defines[i].bytes.size();
				was_define = true;
				break;
			}
		}
		if (was_define) { continue; } // Check if the line was a define, and if so skip everything else

		// probably an instruction
		// Determine argument types
		std::vector<bool> argument_registers;
		for (int i = 0; i < arguments.size(); i++) {
			bool is_register = false;
			// Check if the argument has any characters (if it doesnt it can be discarded now)
			if (arguments[i].find_first_not_of("123456789") == std::string::npos) { continue; }
			// Check if the argument is a register
			for (int r = 0; r < machine.registers.size(); r++) {
				if (machine.registers[r].name == arguments[i]) { is_register = true; break; }
			}
			argument_registers.push_back(is_register);
		}

		bool instruction_was_valid = false;
		for (int i = 0; i < machine.instructions.size(); i++) {
			// Check if the instruction argument count matches
			if (machine.instructions[i].arguments.size() != arguments.size()) { continue; }
			// Check if the mnemonic matches
			if (machine.instructions[i].mnemonic != command) { continue; }
			// Check if the instruction has the same registers / immediate arguments
			bool match = true;
			for (int r = 0; r < machine.instructions[i].arguments.size(); r++) {
				if (machine.instructions[i].arguments[r] == 0xFFFF && argument_registers[r] != true) { match = false; break; } else { continue; } // Fail if the argument should be a instruction but it isnt
				if (argument_registers[r] != false) { match = false; break; } // Fail if the argument should not be a register but it is
			}
			if (match == false) { continue; } // Skip instruction if false
			// We have found a compatible instruction
			// Construct instruction variable
			Instruction inst;
			inst.address = prog_seek;
			inst.arguments = arguments;
			inst.machine_instruction = machine.instructions[i];
			// Add instruction to instruction vector to process later
			instructions.push_back(inst);
			// Insert placeholder values into prog
			if (prog_seek > prog.size()) { prog.resize(prog_seek + machine.instructions[i].instruction_length); } // Ensure prog has sufficent capacity
			for (int insert = 0; insert < machine.instructions[i].instruction_length; insert++) { prog.insert(prog.begin() + prog_seek++, insert); }
			// Set boolean and break out
			instruction_was_valid = true;
			break;
		}
		if (!instruction_was_valid) { QUIT_ERR_LINE("Assembler", "Invalid Instruction!", s); }
	}

	/// Pass 2: insert instructions into prog
	file.close();
	for (int i = 0; i < instructions.size(); i++) {
		std::vector<long long> arguments; // arguments for this instruction
		std::vector<int> arguments_bit_offset; // used in step 2
		// Step 1: get arguments as long longs
		for (int x = 0; x < instructions.at(i).machine_instruction.arguments.size(); x++) {
			if (instructions.at(i).machine_instruction.arguments.at(x) == 0xffff) {
				// register
				int reg = -0xff;
				for (int y = 0; y < machine.registers.size(); y++) {
					if (machine.registers.at(y).name == instructions.at(i).arguments.at(x)) {
						reg = y;
						break;
					}
				}
				if (reg == -0xff) {
					QUIT_ERR("Assembler", "BUG: Could not find register in pass 2!");
				}
				arguments.push_back(reg);
			} else {
				// data
				try {
					long long data = std::stoll(instructions.at(i).arguments[x], 0, 0);
					arguments.push_back(data);
				} catch (std::invalid_argument) {
					// check if the argument is a symbol
					bool fail = true;
					for (int v = 0; v < symbols.size(); v++) {
						if (symbols.at(v).name == instructions.at(i).arguments.at(x)) {
							fail = false;
							arguments.push_back(symbols.at(v).value);
							arguments_bit_offset.push_back(0);
						}
					}
					if (!fail) { continue; }
					QUIT_ERR("Assembler", "Could not convert string to int! TODO: Perform a check in pass 1");
				}
			}
			arguments_bit_offset.push_back(0);
		}
		// Step 2: put them into prog
		size_t bit_count = instructions.at(i).machine_instruction.instruction.size();
		Machine::Instruction machine_instruction = instructions.at(i).machine_instruction;
		//std::bitset<BITSET_INSTRUCTION_SIZE> bits;
		int byte = 0;
		for (int x = 0; x < bit_count; x++) {
			bool bit = false;
			// check if the instruction bit is always a 1 or a 0
			if (machine_instruction.instruction.at(x) == 0 || machine_instruction.instruction.at(x) == 1) {
				bit = machine_instruction.instruction.at(x) == 1;
			} else {
				int arg = machine_instruction.instruction.at(x) - 2; // argument id
				bit = ((arguments.at(arg) >> arguments_bit_offset.at(arg)++) & 1);
			}
			// check if a byte has passed
			if (x % 8 == 0 && x != 0) { byte++; }

			unsigned char b = prog[instructions.at(i).address + ((machine_instruction.instruction_length - (int)1) - (int)byte)]; // get the current byte
			int bit_position = x % 8; // calculate the position in the byte
			prog[instructions.at(i).address + ((machine_instruction.instruction_length - (int)1) - (int)byte)] = (b & ~(1UL << bit_position)) | (bit << bit_position); // set the bit
		}
		LOG_MSG("Assembler", "Assembled instruction " << i);
	}
	ret.data = prog;
	ret.origin = origin;
	return ret;
}

std::string Assembler::getRule(Machine::MachineFile machine, std::string name) {
	std::string ret;
	// some defaults
	if (name == "default-origin") { ret = "0x0"; }
	if (name == "inst-size") { ret = "0"; }
	if (name == "endian") { ret = "0"; }
	if (name == "inst-pad") { ret = "1"; }
	if (name == "jump-label-offset") { ret = "0"; }
	for (int i = 0; i < machine.rules.size(); i++) {
		if (machine.rules.at(i).name == name) {
			ret = machine.rules.at(i).data;
		}
	}
	return ret;
}
