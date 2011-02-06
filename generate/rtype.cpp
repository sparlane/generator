#include <generator.h>

using namespace generator;

bool SystemType::genType(std::ostream& header)
{
	header << this->TypeName;
	return true;
}

bool SystemType::genStruct(std::ostream& header, std::string name)
{
	header << this->TypeName << " " << name;
	return true;
}

bool SystemType::genSetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t)
{
	header << "bool " << Mod->funcPrefix() << t->name() << "_" << name << "_set(";
	t->genStruct(header, "o");
	header << ", ";
	this->genStruct(header, "v");
	header << ")";
	return true;
}

bool SystemType::genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t)
{
	this->genType(header);
	header << " " << Mod->funcPrefix() << t->name() << "_" << name << "_get(";
	t->genStruct(header, "o");
	header << ")";
	return true;
}

bool SystemType::genFunctionDefs(std::ostream& header, std::string *name, Module *Mod, Type *t)
{
	if(!genSetFunctionDef(header, name, Mod, t)) return false;
	header << ";" << std::endl;
	if(!genGetFunctionDef(header, name, Mod, t)) return false;
	header << ";" << std::endl;
	return true;
}

bool SystemType::genLogic(std::ostream& logic, std::string *name, Module *Mod, Type *t)
{
	genSetFunctionDef(logic, name, Mod, t);
	logic << std::endl;
	logic << "{" << std::endl;
	
	t->lock_code_print(logic, false);
	
	logic << "\to->" << name << " = v;" << std::endl;
	logic << std::endl;

	t->unlock_code_print(logic);

	logic << "\treturn true;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;

	genGetFunctionDef(logic, name, Mod, t);
	logic << std::endl;
	logic << "{" << std::endl;
	
	t->lock_code_print(logic, false);
	
	logic << "\t";
	this->genStruct(logic, "res");
	
	logic << " = o->" << name << ";" << std::endl;
	logic << std::endl;

	t->unlock_code_print(logic);

	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;

	return true;
}

bool SystemType::genDestruct(std::ostream& logic, std::string *name)
{
	return true;
}

