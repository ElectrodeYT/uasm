#include <fstream>
#include <iostream>
#include "assembler.h"
#include "helper.h"
#include "log.h"

std::vector<unsigned char> Assembler::assembleMachine(Machine::MachineFile machine, std::string path) {
	std::vector<unsigned char> prog;
	// get some important rules
	int origin = std::stoi(getRule(machine, "default-origin"));
	int inst_size = std::stoi(getRule(machine, "inst-size"));
	// define some variables
	std::vector<Variable> vars; // for things like labels and stuff
	// create a fstream for the assembly code
	std::fstream file(path, std::ios::in);

	/// Pass 1: calculate the "length" of each line, and get the addresses of all labels and stuff
	std::string s;
	while (std::getline(file, s)) {
		// seperate the string by whitespace
		std::vector<std::string> split = Helper::splitString(s, ' ');
		std::vector<std::string> arguments(1);
		if (s == "") { goto line_done; } // skip everything, continue doesnt seem to always work
		// things we need to check for this
		if (s.at(0) == '.') {
			if (split.empty()) { std::cerr << "[Assembler] [BUG] Split string does not contain any entries!\n"; exit(-2); }
			if (split.size() == 1) { QUIT_ERR_LINE("Assembler", "Line invalid!", s); }
			// if we are here, then the line is at least somewhat valid
			if (split.at(0) == ".lbl") { std::cout << "[Assembler] [DEBUG] Found label " << split.at(1) << "\n"; goto line_done; }
			
			LOG_WRN_LINE("Assembler", "Found invalid dot line, ignoring.", s);
			goto line_done;
		}
		// probably an instruction
		arguments = Helper::splitString(split.at(1), ',');
		for (int i = 0; i < machine.instructions.size(); i++) {
			if (split.at(0) == machine.instructions.at(i).mnemonic) {
				// mnemonic matches, check arguments
				if (machine.instructions.at(i).arguments.size() < split.size()) { goto instruction_argument_not_correct; }
				// check if the argument type matches
				for (int x = 0; x < machine.instructions.at(i).arguments.size(); x++) {
					if (machine.instructions.at(i).arguments.at(x) == 0xffff) {
						// a argument requires a register
						for (int y = 0; y < machine.registers.size(); y++) {
							if (split.at(x + 1) == machine.registers.at(y).name) { goto argument_register_matches; }
						}
						goto instruction_argument_not_correct; // not a register
					argument_register_matches:
						continue;
					}
				}
				// correct instruction, add instruction length to prog
				LOG_MSG_LINE("Assembler", "Got line length: " << machine.instructions.at(i).instruction_length, s);
				goto line_done;
			instruction_argument_not_correct: // simply skip this instruction
				continue;
			}
		}
		QUIT_ERR_LINE("Assembler", "Invalid Instruction!", s);
	line_done:
		continue;
	}

	return prog;
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
