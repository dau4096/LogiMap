/* hdl.h */
#ifndef HDL_H
#define HDL_H


#include <unordered_set>
#include <string>


//"Hardware Description Language"
namespace HDL {

	extern std::unordered_set<std::string> inputs;
	extern std::unordered_set<std::string> outputs;

	void parse(const std::string filePath);

}



#endif

