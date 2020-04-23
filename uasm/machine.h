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
		/// Decode a file line into the instruction
		void decodeInstLine(std::vector<std::string> line);

	};
	class MachineFile {
	public:
		std::vector<Instruction> instructions;
	};

	MachineFile readMachine(std::string path);
	/// Convert a file line to a vector
	/// First entry is the "name", others are the arguments
	std::vector<std::string> convertFileLineToVector(std::string s);
}