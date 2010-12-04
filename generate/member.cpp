#include <generator.h>

using namespace generator;

bool Member::genStruct(FILE *header)
{
	print_to_file(header, "\t");
	if(!this->genType(header)) return false;
	print_to_file(header, " %s;\n", this->Name->c_str());
	return true;
}

bool Member::genType(FILE *header)
{
	std::cerr << "Member (" << *this->name() << ") has not yet declared its genType method" << std::endl;
	return false;
}

bool Member::genFunctionDefs(FILE *header, Module *Mod, Type *t)
{
	std::cerr << "Member (" << *this->name() << ") has not yet declared its genFunctionDefs method" << std::endl;
	return false;
}

bool Member::genLogic(FILE *header, Module *Mod, Type *t)
{
	std::cerr << "Member (" << *this->name() << ") has not yet declared its genLogic method" << std::endl;
	return false;
}

bool Member::genTemplate(FILE *header)
{
	std::cerr << "Member (" << *this->name() << ") has not yet declared its genTemplate method" << std::endl;
	return false;
}

bool Member::genDestruct(FILE *header)
{
	std::cerr << "Member (" << *this->name() << ") has not yet declared its genDestruct method" << std::endl;
	return false;
}

bool Member::print_connect(FILE *logic)
{
	std::cerr << "Member (" << *this->name() << ") has not yet declared its print_connect method" << std::endl;
	return false;
}

bool Member::print_disconnect(FILE *logic)
{
	std::cerr << "Member (" << *this->name() << ") has not yet declared its print_disconnect method" << std::endl;
	return false;
}
