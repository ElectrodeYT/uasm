#include <fstream>
#include <iostream>
#include <bitset>
#include <sstream>
#include <map>
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
	size_t prog_seek = 0; // Current "PC" of the assembler
	size_t data_seek = 0; // Current data pointer of the assembler
	// check if the cpu has a harvard architecture
	bool cpu_is_harvard = std::stoi(getRule(machine, "cpu-is-harvard"), 0, 0);
	bool data_segment = false; // Used to determine wether something should be written to prog or data

    // get some important rules
	size_t origin = std::stoi(getRule(machine, "default-origin"), 0, 0);
	size_t data_origin;
	// If the cpu is a harvard style cpu, read the rule for the data origin
	// else, mirror the code origin
	if (cpu_is_harvard) { data_origin = std::stoi(getRule(machine, "default-data-origin"), 0, 0); } else { data_origin = origin; }
	
	// size_t inst_size = std::stoi(getRule(machine, "inst-size"), 0, 0);
	// define some variables
	std::vector<Variable> symbols; // for things like labels and stuff
	std::vector<Instruction> instructions; // instructions
    std::map<std::string, std::string> macros; // macros
    /// Pass 0: remove unused stuff
    for(size_t i = 0; i < lines.size(); i++) {
        std::string s = lines[i];
        // remove everything past #
        if (s.find("#") != std::string::npos) {
            s = s.substr(0, s.find("#"));
        }
        s = Helper::trimString(s);
        // If this line is empty now, delete it
        if(s == "") { lines.erase(lines.begin() + i); i--; continue; }
        lines[i] = s;
    }

    /// Pass 1.0: macro fetching
    /*for(size_t i = 0; i < lines.size(); i++) {
        std::string s = lines[i];
        if(s[0] == '.') {

        }
    }*/


    /// Pass 1.5: macro inserting


	/// Pass 2: calculate the "length" of each line and get the addresses of all labels and stuff
	for (size_t i = 0; i < lines.size(); i++) {
		std::string s = lines[i];

		// seperate the string by whitespace
		std::vector<std::string> split = Helper::splitString(s, ' ');

		std::string command = split[0]; // Command or mnemonic;
		std::vector<std::string> arguments; // Arguments to instruction
		if (split.size() > 1) {
			// concatante everything past the mnemonic
			std::string arguments_string;
			std::stringstream concat(arguments_string);
			for (size_t x = 1; x < split.size(); x++) {
				concat << split[x];
			}
			arguments = Helper::splitString(concat.str(), ',');
            // Trim everything
            // After this, none of these should have any whitespace in them
            for(size_t x = 0; x < arguments.size(); x++) {
                arguments[x] = Helper::trimString(arguments[x]);
                if(arguments[x].find_first_of(" \t") != std::string::npos) {
                    LOG_ERR_LINE("Assembler", "Whitespace in arguments", s);
                    ret.failed = true;
                    return ret;
                }
            }
		}

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
				} catch (...) {
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
						LOG_WRN_LINE("Assembler", "Using .seek can cause assembled instructions to be overwritten. It is not recommended to use this", s);
					}
					if (data_segment) {
						data_seek = std::stoi(split[1], 0, 0);
					} else {
						prog_seek = std::stoi(split[1], 0, 0);
					}
				} catch (...) {
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
						LOG_WRN_LINE("Assembler", "Using .skip can cause assembled instructions to be overwritten. It is not recommended to use this", s);
					}
					int amount_to_skip = std::stoi(split[1], 0, 0);
					if (cpu_is_harvard && data_segment) {
						data_seek += amount_to_skip;
					} else {
						prog_seek += amount_to_skip;
					}
				} catch (...) {
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
					// Check if the argument is a string
					if(arguments.size() && arguments[0][0] == '\"') {
					    // We have to loop until we either get a " or until the end
					    // If we get to the end and we dont have a ", we need to raise an error
					    bool did_get_end = false;
					    for(size_t y = 1; y < arguments[0].size(); y++) {
					        switch(arguments[0][y]) {
					            case '\"': did_get_end = true; break;
					            case '\\': {
					                // Get the next one
					                if(y + 1 == arguments.size()) { LOG_ERR_LINE("Assembler", "Error parsing line: End-of-line after backslash", s); }
					                y++;
					                switch(arguments[0][y]) {
					                    case 'n': case 'N': data.insert(data.begin() + data_seek++, '\n'); break;
					                    case '0': data.insert(data.begin() + data_seek++, 0); break;
					                    default: data.insert(data.begin() + data_seek++, arguments[0][y]);
					                }
					                break;
					            }
					            default: data.insert(data.begin() + data_seek++, arguments[0][y]);
					        }
					    }
					    if(!did_get_end) { LOG_ERR_LINE("Assembler", "Error parsing line: End-of-line before \"", s); }
					    // Add NULL terminator
					    data.insert(data.begin() + data_seek, 0);
					} else {
                        for (size_t y = 0; y < arguments.size(); y++) {
                            data.insert(data.begin() + data_seek++, std::stoi(arguments[i], 0, 0));
                        }
                    }
				} else {
					if (prog_seek > prog.size()) { prog.resize(prog_seek); } // Ensure prog has sufficent capacity
					for (size_t i = 0; i < arguments.size(); i++) {
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
		for (size_t i = 0; i < machine.defines.size(); i++) {
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
		for (size_t i = 0; i < arguments.size(); i++) {
			bool is_register = false;
			// Check if the argument has any characters (if it doesnt it can be discarded now)
			if (arguments[i].find_first_not_of("123456789") == std::string::npos) { continue; }
			// Check if the argument is a register
			for (size_t r = 0; r < machine.registers.size(); r++) {
				if (machine.registers[r].name == arguments[i]) { is_register = true; break; }
			}
			argument_registers.push_back(is_register);
		}

		bool instruction_was_valid = false;
		// Loop through every instruction this machine has
		for (size_t x = 0; x < machine.instructions.size(); x++) {
			// Check if the instruction argument count matches
			if (machine.instructions[x].arguments.size() != arguments.size()) { continue; }
			// Check if the mnemonic matches
			if (machine.instructions[x].mnemonic != command) { continue; }
			
			// Check if the instruction has the same registers / immediate arguments
			bool match = true;
			for (size_t r = 0; r < machine.instructions[x].arguments.size(); r++) {
				if (machine.instructions[x].arguments[r] == 0xFFFF && argument_registers[r] != true) { match = false; break; } else { continue; } // Fail if the argument should be a instruction but it isnt
				if (argument_registers[r] != false) { match = false; break; } // Fail if the argument should not be a register but it is
			}
			if (match == false) { continue; } // Skip instruction if false

			// We have found a compatible instruction
			// Construct instruction variable
			Instruction inst;
			inst.address = prog_seek;
			inst.arguments = arguments;
			inst.machine_instruction = machine.instructions[x];
			// Add instruction to instruction vector to process later
			instructions.push_back(inst);
			// Insert placeholder values into prog
			if (prog_seek > prog.size()) { prog.resize(prog_seek + machine.instructions[x].instruction_length); } // Ensure prog has sufficent capacity
			for (int insert = 0; insert < machine.instructions[x].instruction_length; insert++) { prog.insert(prog.begin() + prog_seek++, insert); }
			// Set boolean and break out
			instruction_was_valid = true;
			break;
		}
		if (!instruction_was_valid) {
			LOG_ERR_LINE("Assembler", "Invalid Instruction on line " << i << "!", s);
			ret.failed = true;
			return ret;
		}
	}

	/// Pass 3: insert instructions into prog
	for (size_t i = 0; i < instructions.size(); i++) {
		std::vector<long long> arguments; // arguments for this instruction
		std::vector<int> arguments_bit_offset; // used in step 2
		// Step 1: get arguments as long longs
		for (size_t x = 0; x < instructions[i].machine_instruction.arguments.size(); x++) {
			if (instructions[i].machine_instruction.arguments[x] == 0xffff) {
				// register
				int reg = -0xff;
				for (size_t y = 0; y < machine.registers.size(); y++) {
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
				} catch (...) {
					// check if the argument is a symbol
					bool fail = true;
					for (size_t v = 0; v < symbols.size(); v++) {
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
		for (size_t x = 0; x < bit_count; x++) {
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
    if (name == "elf-output-supported") { ret = "0"; }
    if (name == "bin-output-supported") { ret = "1"; }
    if (name == "bin-data-segment-offset") { ret = "0"; } // The offset of the data segment
    if (name == "bin-data-segment-has-seek") { ret = "0"; } // Is the data segment (by convention) supposed to be at a specific location?
    if (name == "bin-data-segment-seek") { ret = "0"; } // Prefereed offset of the data segment
	for (size_t i = 0; i < machine.rules.size(); i++) {
		if (machine.rules[i].name == name) {
			ret = machine.rules[i].data;
		}
	}
	return ret;
}
