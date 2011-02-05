#include <generator.h>

using namespace generator;

bool Pointer::genType(std::ostream& header)
{
	this->To->genType(header);
	header << "*";
	return true;
}

bool Pointer::genStruct(std::ostream& header, std::string name)
{
	this->To->genType(header);
	header << "* " << name;
	return true;
}

bool Pointer::genSetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t)
{
	header << "bool " << Mod->funcPrefix() << t->name() << "_" << name << "_set(";
	t->genStruct(header, "o");
	header << ", ";
	this->genStruct(header, "v");
	header << ")";
	return true;
}

bool Pointer::genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t)
{
	this->genType(header);
	header << " " << Mod->funcPrefix() << t->name() << "_" << name << "_get(";
	t->genStruct(header, "o");
	header << ")";
	return true;
}

bool Pointer::genFunctionDefs(std::ostream& header, std::string *name, Module *Mod, Type *t)
{
	this->genSetFunctionDef(header, name, Mod, t);
	header << ";" << std::endl;
	this->genGetFunctionDef(header, name, Mod, t);
	header << ";" << std::endl;
	return true;
}

bool Pointer::genDestruct(std::ostream& header, std::string *name)
{
	header << "\tif(o->" << name << " != NULL)" << std::endl;
	header << "\t{" << std::endl;
	header << "\t\t" << this->Destroy << "(o->" << name << ");" << std::endl;
	header << "\t}" << std::endl;
	header << std::endl;
	return true;
}

bool Pointer::genLogic(std::ostream& logic, std::string *name, Module *Mod, Type *t)
{
	genSetFunctionDef(logic, name, Mod, t);
	logic << std::endl;
	logic << "{" << std::endl;

	t->lock_code_print(logic, false);
	
	logic << "\tif(o->" << name << " != NULL)" << std::endl;
	logic << "\t{" << std::endl;
	logic << "\t\t" << this->Destroy << "(o->" << name << ");" << std::endl;
	logic << "\t\to->" << name << " = NULL;" << std::endl;
	logic << "\t}" << std::endl;
	logic << std::endl;

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
	this->genType(logic);
	logic << " res = o->" << name << ";" << std::endl;

	t->unlock_code_print(logic);

	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;
	return true;
}
