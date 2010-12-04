#include <generator.h>

using namespace generator;

bool RType::genType(FILE *header)
{
	print_to_file(header, "%s", this->TypeName->c_str());
	return true;
}

bool RType::genSetFunctionDef(FILE *header, Module *Mod, Type *t)
{
	print_to_file(header, "bool %s%s_%s_set(%s o, %s v)", Mod->funcPrefix()->c_str(), t->name()->c_str(), this->name()->c_str(), t->name()->c_str(), this->TypeName->c_str());
	return true;
}

bool RType::genFunctionDefs(FILE *header, Module *Mod, Type *t)
{
	if(!genSetFunctionDef(header, Mod, t)) return false;
	print_to_file(header, ";\n");
	return true;
}

bool RType::genLogic(FILE *logic, Module *Mod, Type *t)
{
	genSetFunctionDef(logic, Mod, t);
	print_to_file(logic, "\n{\n");
	
	t->lock_code_print(logic, false);
	
	print_to_file(logic, "\to->%s = v;\n\n", this->name()->c_str());

	t->unlock_code_print(logic);

	print_to_file(logic, "\treturn true;\n");
	print_to_file(logic, "}\n\n");
	return true;
}

bool RType::genDestruct(FILE *logic)
{
	return true;
}

