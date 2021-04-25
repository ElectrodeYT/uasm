#pragma once
#include "machine.h"

namespace Assembler {
	class Variable {
	public:
		std::string name;
		long long value;
		bool data_segment;
		Variable(std::string n, long long v, bool d) : name(n), value(v), data_segment(d) {};
	};

	class Instruction {
	public:
		Machine::Instruction machine_instruction;
		std::vector<std::string> arguments;
		long long address = 0;
	};

	class Assembled {
	public:
		std::vector<unsigned char> prog;
		std::vector<unsigned char> data;
		bool harvard = false;
		int origin = 0;
		
		bool failed = false;
	};

	Assembled assembleMachine(Machine::MachineFile machine, std::vector<std::string> lines);
	std::string getRule(Machine::MachineFile, std::string name);
}