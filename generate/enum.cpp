#include <generator.h>

using namespace generator;

bool Enum::genType(std::ostream& header)
{
	std::cerr << "Enum::genType is invalid" << std::endl;
	return false;
}

bool Enum::genStruct(std::ostream& header, std::string name)
{
	header << "enum " << this->name() << " " << name << ";" << std::endl;
	return true;
}

bool Enum::genTypeDef(std::ostream& header)
{
	header << "enum " << this->name() << " {" << std::endl;
	for(string_int_set::iterator miter = this->members.begin(); miter != this->members.end(); ++miter)
	{
		header << "\t" << this->name() << "_" << miter->first;
		if(miter->second != 0)
		{
			header << " = " << miter->second;
		}
		header << "," << std::endl;
	}
	header << "};" << std::endl;
	return true;
}
