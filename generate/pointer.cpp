#include <generator.h>

using namespace generator;

bool Pointer::genType(FILE *header)
{
	this->To->genType(header);
	print_to_file(header, "*");
	return true;
}

bool Pointer::genSetFunctionDef(FILE *header, Module *Mod, Type *t)
{
	print_to_file(header, "bool %s%s_%s_set(%s o, ", Mod->funcPrefix()->c_str(), t->name()->c_str(), this->name()->c_str(), t->name()->c_str());
	this->To->genType(header);
	print_to_file(header, " *v)");
	return true;
}

bool Pointer::genFunctionDefs(FILE *header, Module *Mod, Type *t)
{
	this->genSetFunctionDef(header, Mod, t);
	print_to_file(header, ";\n");
	return true;
}

bool Pointer::genDestruct(FILE *header)
{
	print_to_file(header, "\tif(o->%s != NULL)\n\t{\n\t\t%s(o->%s);\n\t}\n\n", this->name()->c_str(), this->Destroy->c_str(), this->name()->c_str());
	return true;
}

bool Pointer::genLogic(FILE *logic, Module *Mod, Type *t)
{
	genSetFunctionDef(logic, Mod, t);
	print_to_file(logic, "\n{\n");
	
	t->lock_code_print(logic, false);
	
	print_to_file(logic, "\tif(o->%s != NULL)\n\t{\n", this->name()->c_str());
	print_to_file(logic, "\t\t%s(o->%s);\n", this->Destroy->c_str(), this->name()->c_str());
	print_to_file(logic, "\t\to->%s = NULL;\n", this->name()->c_str());
	print_to_file(logic, "\t}\n\n");

	print_to_file(logic, "\to->%s = v;\n\n", this->name()->c_str());

	t->unlock_code_print(logic);

	print_to_file(logic, "\treturn true;\n");
	print_to_file(logic, "}\n\n");
	return true;
}
