/* hdl.h */
#ifndef HDL_H
#define HDL_H


#include <set>
#include <string>


//"Hardware Description Language"
namespace HDL {

	extern std::set<std::string> inputs;
	extern std::set<std::string> outputs;

	void parse(const std::string filePath);

}



#endif

