#include <generator.h>

using namespace generator;

bool Array::genType(FILE *header)
{
	std::cerr << "Array::getType is invalid" << std::endl;
	return false;
}

bool Array::genStruct(FILE *header)
{
	print_to_file(header, "\t");
	this->Of->genType(header);
	print_to_file(header, " *%s;\n", this->name()->c_str());
	print_to_file(header, "\tsize_t %s_size;\n", this->name()->c_str());
	return true;
}

bool Array::genAddFunctionDef(FILE *header, Module *Mod, Type *t)
{
	print_to_file(header, "bool %s%s_%s_add(%s o, ", Mod->funcPrefix()->c_str(), t->name()->c_str(), this->name()->c_str(), t->name()->c_str());
	this->Of->genType(header);
	print_to_file(header, " v)");
	return true;
}

bool Array::genFunctionDefs(FILE *header, Module *Mod, Type *t)
{
	this->genAddFunctionDef(header, Mod, t);
	print_to_file(header, ";\n");
	return true;
}

bool Array::genDestruct(FILE *logic)
{
	print_to_file(logic, "\tif(o->%s != NULL)\n\t{\n", this->name()->c_str());
	if(this->Of->needs_disconnecting())
	{
		print_to_file(logic, "\t\tfor(size_t i = 0; i < o->%s_size; i++)\n" , this->name()->c_str());
		print_to_file(logic, "\t\t{\n");
		print_to_file(logic, "\t\t\t");
		this->Of->print_disconnect(logic);
		print_to_file(logic, "(o->%s[i]);\n", this->name()->c_str());
		print_to_file(logic, "\t\t}\n");
	}
	print_to_file(logic, "\t\tfree(o->%s)\n", this->name()->c_str());
	print_to_file(logic, "\t\to->%s = NULL;\n", this->name()->c_str());
	print_to_file(logic, "\t\to->%s_size = 0;\n", this->name()->c_str());
	print_to_file(logic, "\t}\n\n");
	return true;
}

bool Array::genLogic(FILE *logic, Module *Mod, Type *t)
{
	genAddFunctionDef(logic, Mod, t);
	print_to_file(logic, "\n{\n");
	
	t->lock_code_print(logic, false);
	
	print_to_file(logic, "\tbool res = true;\n");
	print_to_file(logic, "\to->%s_size++;\n", this->name()->c_str());
	print_to_file(logic, "\tvoid *n = realloc(o->%s, sizeof(", this->name()->c_str());
	this->Of->genType(logic);
	print_to_file(logic, ") * o->%s_size);\n", this->name()->c_str());
	print_to_file(logic, "\tif(n != NULL)\n\t{\n");
	print_to_file(logic, "\t\to->%s[o->%s_size-1] = v;\n", this->name()->c_str(), this->name()->c_str());
	if(this->Of->needs_connecting())
	{
		this->Of->print_connect(logic);
		print_to_file(logic, "(o->%s[o->%s_size-1]);\n", this->name()->c_str(), this->name()->c_str());
	}
	print_to_file(logic, "\t} else {\n\t\tres = false;\n\t}\n");

	t->unlock_code_print(logic);

	print_to_file(logic, "\treturn res;\n");
	print_to_file(logic, "}\n\n");
	return true;
}
