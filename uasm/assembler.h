#pragma once
#include "machine.h"

namespace Assembler {
	class Variable {
	public:
		std::string name;
		long long value;
		Variable(std::string n, long long v) : name(n), value(v) {};
	};

	class Instruction {
	public:
		Machine::Instruction machine_instruction;
		std::vector<std::string> arguments;
		long long address = 0;
	};

	class Assembled {
	public:
		std::vector<unsigned char> data;
		int origin = 0;
	};

	Assembled assembleMachine(Machine::MachineFile machine, std::string path);
	std::string getRule(Machine::MachineFile, std::string name);
}