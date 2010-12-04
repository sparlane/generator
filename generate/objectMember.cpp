#include <generator.h>

using namespace generator;

bool ObjectMember::genType(FILE *header)
{
	print_to_file(header, "%s", this->Object->name()->c_str());
	return true;
}

bool ObjectMember::genSetFunctionDef(FILE *header, Module *Mod, Type *t)
{
	print_to_file(header, "bool %s%s_%s_set(%s o, %s v)", Mod->funcPrefix()->c_str(), t->name()->c_str(), this->name()->c_str(), t->name()->c_str(), this->Object->name()->c_str());
	return true;
}

bool ObjectMember::genFunctionDefs(FILE *header, Module *Mod, Type *t)
{
	this->genSetFunctionDef(header, Mod, t);
	print_to_file(header, ";\n");
	return true;
}

bool ObjectMember::genDestruct(FILE *logic)
{
	print_to_file(logic, "\tif(o->%s != NULL)\n", this->name()->c_str());
	print_to_file(logic, "\t{\n");
	print_to_file(logic, "\t\t");
	this->Object->print_disconnect_function(logic);
	print_to_file(logic, "(o->%s);\n", this->name()->c_str());
	print_to_file(logic, "\t}\n\n");
	return true;
}

bool ObjectMember::genLogic(FILE *logic, Module *Mod, Type *t)
{
	genSetFunctionDef(logic, Mod, t);
	print_to_file(logic, "\n{\n");
	
	t->lock_code_print(logic, false);
	
	print_to_file(logic, "\tif(o->%s != NULL)\n\t{\n", this->name()->c_str());
	print_to_file(logic, "\t\t");
	this->Object->print_disconnect_function(logic);
	print_to_file(logic, "(o->%s);\n", this->name()->c_str());
	print_to_file(logic, "\t\to->%s = NULL;\n", this->name()->c_str());
	print_to_file(logic, "\t}\n\n");

	print_to_file(logic, "\to->%s = v;\n\n", this->name()->c_str());
	if(this->Object->needs_referencing())
	{
		print_to_file(logic, "\tif(o->%s != NULL)\n", this->name()->c_str());
		print_to_file(logic, "\t{\n");
		print_to_file(logic, "\t\t");
		this->Object->print_connect_function(logic);
		print_to_file(logic, "(o->%s);\n", this->name()->c_str());
		print_to_file(logic, "\t}\n");
	}
	
	t->unlock_code_print(logic);

	print_to_file(logic, "\treturn true;\n");

	print_to_file(logic, "}\n");
	return true;
}
