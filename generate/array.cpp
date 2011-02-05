#include <generator.h>

using namespace generator;

bool Array::genType(std::ostream& header)
{
	std::cerr << "Array::getType is invalid" << std::endl;
	return false;
}

bool Array::genStruct(std::ostream& header, std::string name)
{
	this->Of->genType(header);
	header << " *" << name << ";" << std::endl;
	header << "\tsize_t " << name << "_size";
	return true;
}

bool Array::genAddFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t)
{
	header << "bool " << Mod->funcPrefix() << t->name() << "_" << name << "_add(";
	t->genStruct(header, "o");
	header << ", ";
	this->Of->genStruct(header, "v");
	header << ")";
	return true;
}

bool Array::genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t)
{
	this->Of->genType(header);
	header << " " << Mod->funcPrefix() << t->name() << "_" << name << "_get(";
	t->genStruct(header, "o");
	header << ", size_t idx)";
	return true;
}

bool Array::genFunctionDefs(std::ostream& header, std::string *name, Module *Mod, Type *t)
{
	this->genAddFunctionDef(header, name, Mod, t);
	header << ";" << std::endl;
	this->genGetFunctionDef(header, name, Mod, t);
	header << ";" << std::endl;
	return true;
}

bool Array::genDestruct(std::ostream& logic, std::string *name)
{
	logic << "\tif(o->" << name << " != NULL)" << std::endl;
	logic << "\t{" << std::endl;
	if(this->Of->needs_disconnecting())
	{
		logic << "\t\tfor(size_t i = 0; i < o->" << name << "_size; i++)" << std::endl;
		logic << "\t\t{" << std::endl;
		logic << "\t\t\t";
		this->Of->print_disconnect(logic);
		logic << "(o->" << name << "[i]);" << std::endl;
		logic << "\t\t}" << std::endl;
	}
	logic << "\t\tfree(o->" << name << ");" << std::endl;
	logic << "\t\to->" << name << " = NULL;" << std::endl;
	logic << "\t\to->" << name << "_size = 0;" << std::endl;
	logic << "\t}" << std::endl;
	logic << std::endl;
	return true;
}

bool Array::genLogic(std::ostream& logic, std::string *name, Module *Mod, Type *t)
{
	genAddFunctionDef(logic, name, Mod, t);
	logic << std::endl;
	logic << "{" << std::endl;
	
	t->lock_code_print(logic, false);
	
	logic << "\tbool res = true;" << std::endl;
	logic << "\to->" << name << "_size++;" << std::endl;
	logic << "\tvoid *n = realloc(o->" << name << ", sizeof(";
	this->Of->genType(logic);
	logic << ") * o->" << name << "_size);" << std::endl;
	logic << "\tif(n != NULL)" << std::endl;
	logic << "\t{" << std::endl;
	logic << "\t\to->" << name << "[o->" << name << "_size-1] = v;" << std::endl;
	if(this->Of->needs_connecting())
	{
		logic << "\t\t";
		this->Of->print_connect(logic);
		logic << "(o->" << name << "[o->" << name << "_size-1]);" << std::endl;
	}
	logic << "\t} else {" << std::endl;
	logic << "\t\tres = false;" << std::endl;
	logic << "\t}" << std::endl;

	t->unlock_code_print(logic);

	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;

	genGetFunctionDef(logic, name, Mod, t);
	logic << std::endl;
	logic << "{" << std::endl;
	
	t->lock_code_print(logic, false);
	
	logic << "\t";
	this->Of->genType(logic);
	logic << " res = " << this->Of->initValue() << ";" << std::endl;
	logic << "\tif(idx < o->" << name << "_size)" << std::endl;
	logic << "\t{" << std::endl;
	logic << "\t\tres = o->" << name << "[idx];" << std::endl;
	if(this->Of->needs_connecting())
	{
		logic << "\t\t";
		this->Of->print_connect(logic);
		logic << "(res);" << std::endl;
	}
	logic << "\t} else {" << std::endl;
	logic << "\t\tres = " << this->Of->initValue() << ";" << std::endl;
	logic << "\t}" << std::endl;

	t->unlock_code_print(logic);

	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;

	return true;
}
