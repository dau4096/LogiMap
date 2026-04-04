/* logic.h */
#ifndef LOGIC_H
#define LOGIC_H



namespace logic {


	void structureLogic(std::vector<types::Gate> gates);

	void addGate(const GateType type, const std::string name, const std::string in0="", const std::string in1="", const std::string in2="");

	void testLogic();


}



#endif

