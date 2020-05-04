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
	// get some important rules
	int origin = std::stoi(getRule(machine, "default-origin"), 0, 0);
	int inst_size = std::stoi(getRule(machine, "inst-size"), 0, 0);
	// define some variables
	std::vector<Variable> vars; // for things like labels and stuff
	std::vector<Instruction> instructions; // instructions
	// create a fstream for the assembly code
	std::fstream file(path, std::ios::in);

	/// Pass 1: calculate the "length" of each line and get the addresses of all labels and stuff
	std::string s;
	while (std::getline(file, s)) {
		// seperate the string by whitespace
		std::vector<std::string> split = Helper::splitString(s, ' ');
		std::string arguments_string;
		std::vector<std::string> arguments;
		if (split.size() > 1) {
			// concatante everything past the 
			std::stringstream concat(arguments_string);
			for (int i = 1; i < split.size(); i++) {
				concat << split.at(i);
			}
			arguments = Helper::splitString(concat.str(), ',');
		}
		if (s == "") { continue; }
		// things we need to check for this
		if (s.at(0) == '.') {
			if (split.empty()) { std::cerr << "[Assembler] [BUG] Split string does not contain any entries!\n"; exit(-2); }
			if (split.size() == 1) { QUIT_ERR_LINE("Assembler", "Line invalid!", s); }
			// if we are here, then the line is at least somewhat valid
			if (split.at(0) == ".lbl") { LOG_MSG_LINE("Assembler", "Found label, address " << prog.size() + origin, s); vars.push_back(Variable(split.at(1), prog.size() + origin)); continue; }
			if (split.at(0) == ".uasm") { continue; } // not required, just a indication to the programmer for the version the source code was made for
			if (split.at(0) == ".origin") {
				if (split.size() == 1) { QUIT_ERR_LINE("Assembler", "Missing operand: address", s); }
				try {
					LOG_MSG("Assembler", "DEBUG: origin line split.at(1): \"" << split.at(1) << "\"");
					LOG_MSG("Assembler", "DEBUG: previous origin, decimal: " << origin);
					origin = std::stoi(split.at(1), 0, 0);
					LOG_MSG("Assembler", "DEBUG: new origin, decimal: " << origin);

				} catch (std::invalid_argument) {
					QUIT_ERR_LINE("Assembler", "Error decoding origin", s);
				}
				continue;
			}
			LOG_WRN_LINE("Assembler", "Found invalid dot line, ignoring.", s);
			continue;
		}
		// probably an instruction
		for (int i = 0; i < machine.instructions.size(); i++) {
			if (split.at(0) == machine.instructions.at(i).mnemonic) {
				Instruction inst;
				// mnemonic matches, check arguments
				if (machine.instructions.at(i).arguments.size() < arguments.size()) { goto instruction_argument_not_correct; }
				// check if the argument type matches
				for (int x = 0; x < machine.instructions.at(i).arguments.size(); x++) {
					if (machine.instructions.at(i).arguments.at(x) == 0xffff) {
						// a argument requires a register
						for (int y = 0; y < machine.registers.size(); y++) {
							if (arguments.at(x) == machine.registers.at(y).name) { goto argument_register_matches; }
						}
						goto instruction_argument_not_correct; // not a register
					argument_register_matches:
						continue;
					}
				}
				// correct instruction, add instruction length to prog
				inst.address = prog.size(); // not address in computer, but in prog
				inst.machine_instruction = machine.instructions.at(i);
				inst.arguments = arguments;
				instructions.push_back(inst); // add instruction to vector
				LOG_MSG_LINE("Assembler", "Got line length: " << machine.instructions.at(i).instruction_length, s);
				for (int insert = 0; insert < machine.instructions.at(i).instruction_length; insert++) { prog.push_back(insert); }
				goto line_done;
			instruction_argument_not_correct: // simply skip this instruction
				continue;
			}
		}
		QUIT_ERR_LINE("Assembler", "Invalid Instruction!", s);
	line_done:
		continue;
	}

	/// Pass 2: insert instructions into prog
	file.close();
	file.open(path, std::ios::in);
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
					long long data = std::stoll(instructions.at(i).arguments.at(x));
					arguments.push_back(data);
				} catch (std::invalid_argument) {
					// check if the argument is a variable
					bool fail = true;
					for (int v = 0; v < vars.size(); v++) {
						if (vars.at(v).name == instructions.at(i).arguments.at(x)) {
							fail = false;
							arguments.push_back(vars.at(v).value);
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
	if (name == "inst-pad") { ret = "0"; }
	if (name == "jump-label-offset") { ret = "0"; }
	for (int i = 0; i < machine.rules.size(); i++) {
		if (machine.rules.at(i).name == name) {
			ret = machine.rules.at(i).data;
		}
	}
	return ret;
}
