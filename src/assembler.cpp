#include <fstream>
#include <iostream>
#include <bitset>
#include <sstream>
#include "assembler.h"
#include "helper.h"
#include "log.h"

#define BITSET_INSTRUCTION_SIZE 128

Assembler::Assembled Assembler::assembleMachine(Machine::MachineFile machine, std::vector<std::string> lines) {
	Assembler::Assembled ret;
	std::vector<unsigned char> prog;
	std::vector<unsigned char> data; // Used only if the cpu is a harvard style chip
	// Check if the machine file was empty
	if(machine.isEmpty()) {
		LOG_ERR("Assembler", "Machine empty!");
		ret.failed = true;
		return ret;
	}
	int prog_seek = 0; // Current "PC" of the assembler
	int data_seek = 0; // Current data pointer of the assembler
	// check if the cpu has a harvard architecture
	bool cpu_is_harvard = std::stoi(getRule(machine, "cpu-is-harvard"), 0, 0);
	bool data_segment = false; // Used to determine wether something should be written to prog or data

    // get some important rules
	int origin = std::stoi(getRule(machine, "default-origin"), 0, 0);
	int data_origin;
	// If the cpu is a harvard style cpu, read the rule for the data origin
	// else, mirror the code origin
	if (cpu_is_harvard) { data_origin = std::stoi(getRule(machine, "default-data-origin"), 0, 0); } else { data_origin = origin; }
	
	int inst_size = std::stoi(getRule(machine, "inst-size"), 0, 0);
	// define some variables
	std::vector<Variable> symbols; // for things like labels and stuff
	std::vector<Instruction> instructions; // instructions
	/// Pass 1: calculate the "length" of each line and get the addresses of all labels and stuff
	for (int i = 0; i < lines.size(); i++) {
		std::string s = lines[i];
		// remove everything past #
		if (s.find("#") != std::string::npos) {
			s = s.substr(0, s.find("#"));
		}
		// seperate the string by whitespace
		s = Helper::trimString(s);
		std::vector<std::string> split = Helper::splitString(s, ' ');
		
		std::string command = split[0]; // Command or mnemonic;
		std::vector<std::string> arguments; // Arguments to instruction
		if (split.size() > 1) {
			// concatante everything past the mnemonic
			std::string arguments_string;
			std::stringstream concat(arguments_string);
			for (int i = 1; i < split.size(); i++) {
				concat << split[i];
			}
			arguments = Helper::splitString(concat.str(), ',');
		}
		if (s == "") { continue; }
		// Check if the line is a dot command
		if (s[0] == '.') {
			if (split.empty()) { std::cerr << "[Assembler] [BUG] Split string does not contain any entries!\n"; exit(-2); }
			if (split.size() == 1) {
				LOG_ERR_LINE("Assembler", "Line invalid!", s);
				ret.failed = true;
				return ret;
			}
			// Label
			// Acts like a symbol
			if (command == ".lbl") {
				if(cpu_is_harvard && data_segment) { symbols.push_back(Variable(split[1], data_seek + data_origin, true)); continue; }
				symbols.push_back(Variable(split[1], prog_seek + origin, false)); continue;
			}
			// UASM Version
			// Not required
			if (command == ".uasm") { continue; } // not required, just a indication to the programmer for the version the source code was made for
			// Change origin
			// Does not change the current offset from 0
			if (command == ".origin") {
				try {
					// If the CPU is a harvard style and the data segment is the current segment, set the data segment origin
					if (cpu_is_harvard && data_segment) {
						data_origin = std::stoi(split[1], 0, 0);
					}
					origin = std::stoi(split[1], 0, 0);
				} catch (std::invalid_argument) {
					LOG_ERR_LINE("Assembler", "Error decoding origin", s);
					ret.failed = true;
					return ret;
				}
				continue;
			}
			// Change current offset from 0
			// Doesnt change the origin
			if (command == ".seek") {
				if (split.size() == 1) {
					LOG_ERR_LINE("Assembler", "Missing operand: offset-from-origin", s);
					ret.failed = true;
					return ret;
				}
				try {
					// Spew warning if not used in data segment or on non-harvard cpus
					if (!cpu_is_harvard || !data_segment) {
						LOG_WRN_LINE("Assembler", "Using .seek can cause assembled instructions to be overwritten. It is not recommended to use this.", s);
					}
					if (data_segment) {
						data_seek = std::stoi(split[1], 0, 0);
					} else {
						prog_seek = std::stoi(split[1], 0, 0);
					}
				} catch (std::invalid_argument) {
					LOG_ERR_LINE("Assembler", "Error decoding offset-from-origin", s);
					ret.failed = true;
					return ret;
				}
				continue;
			}
			// Skip n bytes
			// Value not determined
			if (command == ".skip") {
				if (split.size() == 1) {
					LOG_ERR_LINE("Assembler", "Missing operand: offset-from-origin", s);
					ret.failed = true;
					return ret;
				}
				try {
					// Spew warning if not used in the data segment or on non-harvard cpus
					if (!cpu_is_harvard || !data_segment) {
						LOG_WRN_LINE("Assembler", "Using .seek can cause assembled instructions to be overwritten. It is not recommended to use this.", s);
					}
					int amount_to_skip = std::stoi(split[1], 0, 0);
					if (cpu_is_harvard && data_segment) {
						data_seek += amount_to_skip;
					} else {
						prog_seek += amount_to_skip;
					}
				} catch (std::invalid_argument) {
					LOG_ERR_LINE("Assembler", "Error decoding offset-from-origin", s);
					ret.failed = true;
					return ret;
				}
				continue;
			}
			// Define byte(s)
			// Value determined
			if (command == ".db") {
				// Check if the data segment is selected on harvard cpus
				if (cpu_is_harvard && data_segment) {
					if (data_seek > data.size()) { data.resize(data_seek); } // Ensure prog has sufficent capacity
					for (int i = 0; i < arguments.size(); i++) {
						data.insert(data.begin() + data_seek++, std::stoi(arguments[i], 0, 0));
					}
				} else {
					if (prog_seek > prog.size()) { prog.resize(prog_seek); } // Ensure prog has sufficent capacity
					for (int i = 0; i < arguments.size(); i++) {
						prog.insert(prog.begin() + prog_seek++, std::stoi(arguments[i], 0, 0));
					}
				}
				continue;
			}
			// Select segment
			if (command == ".segment") {
				if (!cpu_is_harvard) {
					LOG_ERR_LINE("Assembler", "Not supported on non-harvard cpus", s);
					ret.failed = true;
					return ret;
				}
				if (split[1] == "code") { data_segment = false; continue; }
				if (split[1] == "data") { data_segment = true; continue; }
				LOG_ERR_LINE("Assembler", "Invalid segment", s);
				ret.failed = true;
				return ret;
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

		if (cpu_is_harvard && data_segment) {
			LOG_ERR_LINE("Assembler", "Instruction definition in data segment", s);
			ret.failed = true;
			return ret;
		}

		// probably an instruction
		// Determine argument types
		std::vector<bool> argument_registers; // true if register
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
		// Loop through every instruction this machine has
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
		if (!instruction_was_valid) {
			LOG_ERR_LINE("Assembler", "Invalid Instruction!", s);
			ret.failed = true;
			return ret;
		}
	}

	/// Pass 2: insert instructions into prog
	for (int i = 0; i < instructions.size(); i++) {
		std::vector<long long> arguments; // arguments for this instruction
		std::vector<int> arguments_bit_offset; // used in step 2
		// Step 1: get arguments as long longs
		for (int x = 0; x < instructions[i].machine_instruction.arguments.size(); x++) {
			if (instructions[i].machine_instruction.arguments[x] == 0xffff) {
				// register
				int reg = -0xff;
				for (int y = 0; y < machine.registers.size(); y++) {
					if (machine.registers[y].name == instructions[i].arguments[x]) {
						reg = y;
						break;
					}
				}
				if (reg == -0xff) {
					LOG_ERR("Assembler", "BUG: Could not find register in pass 2!");
					ret.failed = true;
					return ret;
				}
				arguments.push_back(reg);
			} else {
				// data
				try {
					long long data = std::stoll(instructions[i].arguments[x], 0, 0);
					arguments.push_back(data);
				} catch (std::invalid_argument) {
					// check if the argument is a symbol
					bool fail = true;
					for (int v = 0; v < symbols.size(); v++) {
						if (symbols[v].name == instructions[i].arguments[x]) {
							fail = false;
							arguments.push_back(symbols[v].value);
							arguments_bit_offset.push_back(0);
						}
					}
					if (!fail) { continue; }
					LOG_ERR("Assembler", "Could not convert string to int! TODO: Perform a check in pass 1");
					ret.failed = true;
					return ret;
				}
			}
			arguments_bit_offset.push_back(0);
		}
		// Step 2: put them into prog
		size_t bit_count = instructions[i].machine_instruction.instruction.size();
		Machine::Instruction machine_instruction = instructions[i].machine_instruction;
		//std::bitset<BITSET_INSTRUCTION_SIZE> bits;
		int byte = 0;
		for (int x = 0; x < bit_count; x++) {
			bool bit = false;
			// check if the instruction bit is always a 1 or a 0
			if (machine_instruction.instruction[x] == 0 || machine_instruction.instruction[x] == 1) {
				bit = machine_instruction.instruction[x] == 1;
			} else {
				int arg = machine_instruction.instruction[x] - 2; // argument id
				bit = ((arguments[arg] >> arguments_bit_offset[arg]++) & 1);
			}
			// check if a byte has passed
			if (x % 8 == 0 && x != 0) { byte++; }

			unsigned char b = prog[instructions[i].address + ((machine_instruction.instruction_length - (int)1) - (int)byte)]; // get the current byte
			int bit_position = x % 8; // calculate the position in the byte
			prog[instructions[i].address + ((machine_instruction.instruction_length - (int)1) - (int)byte)] = (b & ~(1UL << bit_position)) | (bit << bit_position); // set the bit
		}
	}
	ret.prog = prog; // Store assembled instructions
	ret.data = data; // Store data segment
	ret.harvard = cpu_is_harvard; // Store if the cpu is of harvard architecture
	ret.origin = origin; // Store instruction origin
	return ret;
}

std::string Assembler::getRule(Machine::MachineFile machine, std::string name) {
	std::string ret;
	// some defaults
	if (name == "default-origin") { ret = "0x0"; }
	if (name == "default-data-origin") { ret = "0x0"; }
	if (name == "inst-size") { ret = "0"; }
	if (name == "endian") { ret = "0"; }
	if (name == "inst-pad") { ret = "1"; }
	if (name == "jump-label-offset") { ret = "0"; }
	if (name == "cpu-is-harvard") { ret = "0"; }
	for (int i = 0; i < machine.rules.size(); i++) {
		if (machine.rules[i].name == name) {
			ret = machine.rules[i].data;
		}
	}
	return ret;
}
