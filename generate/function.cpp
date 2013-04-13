#include <generator.h>

using namespace generator;
using namespace std;

MemberList::iterator Function::paramsIterBegin()
{
	return this->parameters.begin();
}

MemberList::iterator Function::paramsIterEnd()
{
	return this->parameters.end();
}

bool Function::paramAdd(std::string *name, Member<Element> *m)
{
	this->parameters.push_back(std::make_pair(name, m));
	std::string inc = m->include();
	if(inc != "") this->myIncludes.push_back(new std::string(inc));
	return true;
}

std::list<std::string *> Function::includes()
{
	std::string inc = this->ReturnType->include();
	if(inc != "") this->myIncludes.push_back(new std::string(inc));
	return this->myIncludes;
}

bool Function::populate_dependencies(std::set<Module *>& deps)
{
	for(MemberList::iterator pcurr = paramsIterBegin(); pcurr != paramsIterEnd(); ++pcurr)
	{
		deps.insert(pcurr->second->module());
	}
	return this->ReturnType->populate_dependencies(deps);
}


bool Function::genFunctionDef(std::ostream &of, Module *Mod, Type *t, bool tpl, bool type)
{
	MemberList::iterator pcurr = paramsIterBegin();
	MemberList::iterator pend = paramsIterEnd();

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

bool Function::genFunctionCall(std::ostream &of, Module *Mod, Type *t, bool tpl, bool type)
{
	MemberList::iterator pcurr = paramsIterBegin();
	MemberList::iterator pend = paramsIterEnd();

	of << Mod->funcPrefix() << t->name() << "_" << this->name();
	if(tpl)
		of << "_s";
	of << "(o";

	for( ; pcurr != pend; ++pcurr)
	{
		of << ", " << pcurr->first;
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
	this->genFunctionCall(logic, Mod, t, true, false);
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
	logic << " res = " << this->ReturnType->initValue() << ";" << std::endl;
	
	logic << "/* Insert Your CODE HERE */" << std::endl;
	
	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;
	return true;
}
