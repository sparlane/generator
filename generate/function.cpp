#include <generator.h>

using namespace generator;
using namespace std;

std::map<std::string *, Member<Element> *>::iterator Function::paramsIterBegin()
{
	return this->parameters.begin();
}

std::map<std::string *, Member<Element> *>::iterator Function::paramsIterEnd()
{
	return this->parameters.end();
}

bool Function::paramAdd(std::string *name, Member<Element> *m)
{
	this->parameters.insert(std::make_pair(name, m));
	return true;
}

bool Function::genFunctionDef(std::ostream &of, Module *Mod, Type *t, bool tpl, bool type)
{
	std::map<std::string *, Member<Element> *>::iterator pcurr = paramsIterBegin();
	std::map<std::string *, Member<Element> *>::iterator pend = paramsIterEnd();

	this->ReturnType->genType(of);

	of << " " << Mod->funcPrefix() << t->name() << "_" << this->name();
	if(tpl)
		of << "_s";
	of << "(";
	t->genStruct(of, "o");

	for( ; pcurr != pend; ++pcurr)
	{
		of << ", ";
		if(!pcurr->second->genStruct(of, *pcurr->first)) return false;
	}
	of << ")";
	return true;
}

bool Function::genFunctionDefs(std::ostream& of, Module *Mod, Type *t)
{
	this->genFunctionDef(of, Mod, t, false, true);
	of << ";" << std::endl;
	return true;
}

bool Function::genLogic(std::ostream& logic, Module *Mod, Type *t)
{	
	this->genFunctionDef(logic, Mod, t, false, true);
	logic << std::endl;
	logic << "{" << std::endl;
	
	t->lock_code_print(logic, false);

	logic << "\t";
	this->ReturnType->genType(logic);
	logic << " res = ";
	this->genFunctionDef(logic, Mod, t, true, false);
	logic << ";" << std::endl;

	t->unlock_code_print(logic);

	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	
	return true;
}

bool Function::genTemplate(std::ostream& logic, Module *Mod, Type *t)
{
	this->genFunctionDef(logic, Mod, t, true, true);
	logic << std::endl;
	logic << "{" << std::endl;

	logic << "\t";
	this->ReturnType->genType(logic);
	logic << " res = /* SOME VALUE */;" << std::endl;
	
	logic << "/* Insert Your CODE HERE */" << std::endl;
	
	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;
	return true;
}
