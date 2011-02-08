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

bool Array::genAddFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t, bool tpl, bool call)
{
	if(!call) header << "bool ";
	header << Mod->funcPrefix() << t->name() << "_" << name << "_add";
	if(tpl) header << "_s";
	header << "(";
	if(call) header << "o";
	else t->genStruct(header, "o");
	header << ", ";
	if(call) header << "v";
	else this->Of->genStruct(header, "v");
	header << ")";
	return true;
}

bool Array::genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t, bool tpl, bool call)
{
	if(!call) this->Of->genType(header);
	header << " " << Mod->funcPrefix() << t->name() << "_" << name << "_get";
	if(tpl) header << "_s";
	header << "(";
	if(call) header << "o";
	else t->genStruct(header, "o");
	header << ", ";
	if(call) header << "idx";
	else header << "size_t idx";
	header << ")";
	return true;
}

bool Array::genDelFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t, bool tpl, bool call)
{
	if(!call) header << "bool ";
	header << Mod->funcPrefix() << t->name() << "_" << name << "_del";
	if(tpl) header << "_s";
	header << "(";
	if(call) header << "o";
	else t->genStruct(header, "o");
	header << ", ";
	if(call) header << "idx";
	else header << "size_t idx";
	header << ")";
	return true;
}

bool Array::genSizeFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t, bool tpl, bool call)
{
	if(!call) header << "size_t ";
	header << Mod->funcPrefix() << t->name() << "_" << name << "_size";
	if(tpl) header << "_s";
	header << "(";
	if(call) header << "o";
	else t->genStruct(header, "o");
	header << ")";
	return true;
}

bool Array::genFunctionDefs(std::ostream& header, std::string *name, Module *Mod, Type *t)
{
	this->genAddFunctionDef(header, name, Mod, t, false, false);
	header << ";" << std::endl;
	this->genGetFunctionDef(header, name, Mod, t, false, false);
	header << ";" << std::endl;
	this->genDelFunctionDef(header, name, Mod, t, false, false);
	header << ";" << std::endl;
	this->genSizeFunctionDef(header, name, Mod, t, false, false);
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
		logic << "\t\t\tif(!";
		this->Of->print_disconnect(logic);
		logic << "(o->" << name << "[i]))" << std::endl;
		logic << "\t\t\t\tres = false;" << std::endl;
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
	logic << "static ";
	genAddFunctionDef(logic, name, Mod, t, true, false);
	logic << std::endl;
	logic << "{" << std::endl;
	logic << "\tbool res = true;" << std::endl;
	logic << "\to->" << name << "_size++;" << std::endl;
	logic << "\tvoid *n = realloc(o->" << name << ", sizeof(";
	this->Of->genType(logic);
	logic << ") * o->" << name << "_size);" << std::endl;
	logic << "\tif(n != NULL)" << std::endl;
	logic << "\t{" << std::endl;
	logic << "\t\to->" << name << " = n;" << std::endl;
	logic << "\t\to->" << name << "[o->" << name << "_size-1] = v;" << std::endl;
	if(this->Of->needs_connecting())
	{
		logic << "\t\tif(!";
		this->Of->print_connect(logic);
		logic << "(o->" << name << "[o->" << name << "_size-1]))" << std::endl;
		logic << "\t\t{" << std::endl;
		logic << "\t\t\tres = false;" << std::endl;
		logic << "\t\t\to->" << name << "_size--;" << std::endl;
		logic << "\t\t\tif(o->" << name << "_size == 0)" << std::endl;
		logic << "\t\t\t{" << std::endl;
		logic << "\t\t\t\tfree(o->" << name << ");" << std::endl;
		logic << "\t\t\t\to->" << name << " = NULL;" << std::endl;
		logic << "\t\t\t}" << std::endl;
		logic << "\t\t}" << std::endl;
	}
	logic << "\t} else {" << std::endl;
	logic << "\t\tres = false;" << std::endl;
	logic << "\t}" << std::endl;
	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;

	genAddFunctionDef(logic, name, Mod, t, false, false);
	logic << std::endl;
	logic << "{" << std::endl;
	
	t->lock_code_print(logic, false);
	
	logic << "\tbool res = ";
	genAddFunctionDef(logic, name, Mod, t, true, true);
	logic << ";" << std::endl;

	t->unlock_code_print(logic);

	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;

	genGetFunctionDef(logic, name, Mod, t, true, false);
	logic << std::endl;
	logic << "{" << std::endl;
	
	logic << "\t";
	this->Of->genType(logic);
	logic << " res = " << this->Of->initValue() << ";" << std::endl;
	logic << "\tif(idx < o->" << name << "_size)" << std::endl;
	logic << "\t{" << std::endl;
	logic << "\t\tres = o->" << name << "[idx];" << std::endl;
	if(this->Of->needs_connecting())
	{
		logic << "\t\tif(!";
		this->Of->print_connect(logic);
		logic << "(res))" << std::endl;
		logic << "\t\t\tres = " << this->Of->initValue() << ";" << std::endl;
	}
	logic << "\t} else {" << std::endl;
	logic << "\t\tres = " << this->Of->initValue() << ";" << std::endl;
	logic << "\t}" << std::endl;

	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;

	genGetFunctionDef(logic, name, Mod, t, false , false);
	logic << std::endl;
	logic << "{" << std::endl;
	
	t->lock_code_print(logic, false);
	
	logic << "\t";
	this->Of->genType(logic);
	logic << " res = ";
	genGetFunctionDef(logic, name, Mod, t, true, true);
	logic << ";" << std::endl;

	t->unlock_code_print(logic);

	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;

	genDelFunctionDef(logic, name, Mod, t, true, false);
	logic << std::endl;
	logic << "{" << std::endl;
	
	logic << "\tbool res = true;" << std::endl;
	logic << "\tif(idx >= o->" << name << "_size)" << std::endl;
	logic << "\t\tres = false;" << std::endl;
	if(this->Of->needs_disconnecting())
	{
		logic << "\tif(res)" << std::endl;
		logic << "\t{" << std::endl;
		logic << "\t\tif(!";
		this->Of->print_disconnect(logic);
		logic << "(o->" << name << "[idx]))" << std::endl;
		logic << "\t\t\tres = false;" << std::endl;
		logic << "\t}" << std::endl;
	}
	logic << "\tif(res)" << std::endl;
	logic << "\t{" << std::endl;
	logic << "\t\to->" << name << "[idx] = o->" << name << "[o->" << name << "_size-1];" << std::endl;
	logic << "\t\to->" << name << "_size--;" << std::endl;
	logic << "\t\tif(o->" << name << "_size == 0)" << std::endl;
	logic << "\t\t{" << std::endl;
	logic << "\t\t\tfree(o->" << name << ");" << std::endl;
	logic << "\t\t\to->" << name << " = NULL;" << std::endl;
	logic << "\t\t}" << std::endl;
	logic << "\t}" << std::endl;

	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;

	genDelFunctionDef(logic, name, Mod, t, false, false);
	logic << std::endl;
	logic << "{" << std::endl;
	
	t->lock_code_print(logic, false);
	
	logic << "\tbool res = ";
	genDelFunctionDef(logic, name, Mod, t, true, true);
	logic << ";" << std::endl;
	t->unlock_code_print(logic);

	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;

	genSizeFunctionDef(logic, name, Mod, t, true, false);
	logic << std::endl;
	logic << "{" << std::endl;
	
	logic << "\tsize_t res = o->" << name << "_size;" << std::endl;

	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;

	genSizeFunctionDef(logic, name, Mod, t, false, false);
	logic << std::endl;
	logic << "{" << std::endl;
	
	t->lock_code_print(logic, false);
	
	logic << "\tsize_t res = ";
	genSizeFunctionDef(logic, name, Mod, t, true, true);
	logic << ";" << std::endl;
	t->unlock_code_print(logic);

	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;

	return true;
}
