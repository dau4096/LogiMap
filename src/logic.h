/* logic.h */
#ifndef LOGIC_H
#define LOGIC_H



namespace logic {


	void createGates();
	void structureLogic();
	
	void addGate(const GateType type, const std::string name, const std::string in0="", const std::string in1="", const std::string in2="");
	bool addGateIO(
		const GateType type,
		const std::unordered_map<std::string, std::string>& args
	);

}



#endif

