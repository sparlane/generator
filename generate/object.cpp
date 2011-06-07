#include <generator.h>

using namespace generator;
using namespace std;

bool Object::lock_code_print(std::ostream& f, bool null)
{
	if(!nolock)
	{
		f << "\t{\n\t\tint lock_res = pthread_mutex_lock(&o->lock);" << std::endl;
		f << "\t\tif(lock_res != 0) return " << ((null) ? "NULL" : "false") << ";" << std::endl;
		f << "\t\tif(o->destroyed)" << std::endl;
		f << "\t\t{\n\t\t\tpthread_mutex_unlock(&o->lock);\n\t\t\treturn " << (null ? "NULL" : "false") << ";\n\t\t}\n\t}\n\n";
	}
	return true;
}

bool Object::unlock_code_print(std::ostream& f)
{
	if(!nolock)
	{
		f << "\t{" << std::endl;
		f << "\t\t/* int lock_res = */ pthread_mutex_unlock(&o->lock);" << std::endl;
		f << "\t}" << std::endl;
		f << std::endl;
	}
	return true;
}

bool Object::destroy_lock_code_print(std::ostream& f)
{
	if(!nolock)
	f << "\tpthread_mutex_destroy(&o->lock);" << std::endl;
	return true;
}

bool Object::genSetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t)
{
	header << "bool " << Mod->funcPrefix() << t->name() << "_" << name << "_set(";
	t->genStruct(header, "o");
	header << ", ";
	this->genStruct(header, "v");
	header << ")";
	return true;
}

bool Object::genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t)
{
	this->genType(header);
	header << " " << Mod->funcPrefix() << t->name() << "_" << name << "_get(";
	t->genStruct(header, "o");
	header << ")";
	return true;
}

bool Object::genFunctionDefs(std::ostream& header, std::string *name, Module *Mod, Object *t)
{
	this->genSetFunctionDef(header, name, Mod, t);
	header << ";" << std::endl;
	this->genGetFunctionDef(header, name, Mod, t);
	header << ";" << std::endl;
	return true;
}

bool Object::genLogic(std::ostream& logic, std::string *name, Module *Mod, Object *t)
{
	this->genSetFunctionDef(logic, name, Mod, t);
	logic << std::endl;
	logic << "{" << std::endl;
	
	t->lock_code_print(logic, false);
	
	logic << "\tbool res = true;" << std::endl;
	logic << "\tif(o->" << name << " != NULL)" << std::endl;
	logic << "\t{" << std::endl;
	logic << "\t\tif(!";
	this->print_disconnect(logic);
	logic << "(o->" << name << "))" << std::endl;
	logic << "\t\t\tres = false;" << std::endl;
	logic << "\t\tif(res)" << std::endl;
	logic << "\t\t\to->" << name << " = NULL;" << std::endl;
	logic << "\t}" << std::endl;
	logic << std::endl;

	logic << "\tif(res)";
	logic << "\t\to->" << name << " = v;" << std::endl;
	logic << std::endl;

	if(this->needs_referencing())
	{
		logic << "\tif(res && o->" << name << " != NULL)" << std::endl;
		logic << "\t{" << std::endl;
		logic << "\t\tif(!";
		this->print_connect(logic);
		logic << "(o->" << name << "))" << std::endl;
		logic << "\t\t\tres = false;" << std::endl;
		logic << "\t}" << std::endl;
	}
	
	t->unlock_code_print(logic);

	logic << "\treturn res;" << std::endl;
	
	logic << "}" << std::endl;
	logic << std::endl;

	this->genGetFunctionDef(logic, name, Mod, t);
	logic << std::endl;
	logic << "{" << std::endl;
	
	t->lock_code_print(logic, false);
	
	logic << "\t";
	this->genType(logic);
	logic << " res = o->" << name << ";" << std::endl;

	if(this->needs_referencing())
	{
		logic << "\tif(res != NULL)" << std::endl;
		logic << "\t{" << std::endl;
		logic << "\t\tif(!";
		this->print_connect(logic);
		logic << "(res))" << std::endl;
		logic << "\t\t\tres = NULL;" << std::endl;
		logic << "\t}" << std::endl;
		logic << std::endl;
	}
	
	t->unlock_code_print(logic);

	logic << "\treturn res;" << std::endl;
	
	logic << "}" << std::endl;
	logic << std::endl;

	return true;
}

bool Object::genDestruct(std::ostream& logic, std::string *name)
{
	return true;
}

std::string Object::include()
{
	std::string str;
	str = *this->module()->filePrefix() + *this->module()->name() + std::string(".h");
	std::cout << "Include: " << str << std::endl;
	return str;
}
