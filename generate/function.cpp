#include <generator.h>

using namespace generator;
using namespace std;

std::map<std::string *, Member *>::iterator Function::paramsIterBegin()
{
	return this->parameters.begin();
}

std::map<std::string *, Member *>::iterator Function::paramsIterEnd()
{
	return this->parameters.end();
}

bool Function::paramAdd(Member *m)
{
	this->parameters.insert(std::pair<std::string *, Member *>(m->name(), m));
	return true;
}

bool Function::genFunctionDef(FILE *f, Module *Mod, Type *t, bool tpl, bool type)
{
	std::map<std::string *, Member *>::iterator pcurr = paramsIterBegin();
	std::map<std::string *, Member *>::iterator pend = paramsIterEnd();

	if(type)
		this->ReturnType->genType(f);
	print_to_file(f, " %s%s_%s%s(%s o", Mod->funcPrefix()->c_str(), t->name()->c_str(), this->name()->c_str(), (tpl) ? "_s" : "", t->name()->c_str());
	for( ; pcurr != pend; ++pcurr)
	{
		if(pcurr->second->isInit())
		{
			print_to_file(f, ", ");
			if(!pcurr->second->genStruct(f)) return false;
		}
	}
	print_to_file(f, ")");
	return true;
}

bool Function::genFunctionDefs(FILE *header, Module *Mod, Type *t)
{
	this->genFunctionDef(header, Mod, t, false, true);
	print_to_file(header, ";");
	return true;
}

bool Function::genLogic(FILE *logic, Module *Mod, Type *t)
{	
	this->genFunctionDef(logic, Mod, t, false, true);
	print_to_file(logic, "\n{\n");
	
	t->lock_code_print(logic, false);

	print_to_file(logic, "\t");
	this->ReturnType->genType(logic);
	print_to_file(logic, " res = ");
	this->genFunctionDef(logic, Mod, t, true, false);
	print_to_file(logic, ";\n");

	t->unlock_code_print(logic);

	print_to_file(logic, "\treturn res;\n");
	print_to_file(logic, "}\n");
	
	return true;
}

bool Function::genTemplate(FILE *logic, Module *Mod, Type *t)
{
	this->genFunctionDef(logic, Mod, t, true, true);
	print_to_file(logic, "\n{\n");

	print_to_file(logic, "\t");
	this->ReturnType->genType(logic);
	print_to_file(logic, " res = /* SOME VALUE */;\n");
	
	print_to_file(logic, "/* Insert Your CODE HERE */\n");
	
	print_to_file(logic, "\treturn res;\n");
	print_to_file(logic, "}\n\n");
	return true;
}
