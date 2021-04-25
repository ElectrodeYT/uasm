#pragma once
#include <string>
#include <vector>

namespace Machine {
	class Instruction {
	public:
		std::string mnemonic = "";
		std::vector<int> arguments;		// bit count of argument, except when:
										// 0xffff: register
		std::vector<int> instruction;	// every entry is one bit
										// 0: always 0
										// 1: always 1
										// 2+: arguments 0+
		int instruction_type = 0;
		int instruction_length = 0; // length of the instruction, in bytes
	};
	class Register {
	public:
		std::string name;
		int bitsize;
	};
	class Rule {
	public:
		std::string name;
		std::string data;
	};
	class Define {
	public:
		std::string line;
		std::vector<unsigned char> bytes;
	};

	class MachineFile {
	public:
		std::vector<Instruction> instructions;
		std::vector<Register> registers;
		std::vector<Rule> rules;
		std::vector<Define> defines;

		std::string version;
		std::string name;
		std::string assembly_mode;

		bool failed = false;

		// Check if the machine file was empty and so basically no machine was constructed
		// (Except rules)
		bool isEmpty() {
			if (instructions.size() == 0 && defines.size() == 0
				&& registers.size() == 0) { return true; }
			return false;
		}
	};

	MachineFile readMachine(std::vector<std::string> lines);
	/// Convert a file line to a vector
	/// First entry is the "name", others are the arguments
	std::vector<std::string> convertFileLineToVector(std::string s);
}