#include <generator.h>

using namespace generator;

ElementList::iterator FunctionPointer::paramsIterBegin()
{
	return this->parameters.begin();
}

ElementList::iterator FunctionPointer::paramsIterEnd()
{
	return this->parameters.end();
}

bool FunctionPointer::memberAdd(Element *m, std::string *name)
{
	this->parameters.push_back(std::make_pair(name, m));
	return true;
}


bool FunctionPointer::genType(std::ostream& header)
{
	header << this->module()->funcPrefix() << this->module()->name() << "_" << this->name();
	return true;
}

bool FunctionPointer::genTypeDef(std::ostream& header)
{
	ElementList::iterator pcurr = paramsIterBegin();
	ElementList::iterator pend = paramsIterEnd();

	header << "typedef ";
	this->ReturnType->genType(header);
	header << " (*" << this->module()->funcPrefix() << this->module()->name() << "_" << this->name() << ")(";

	if(pcurr == pend)
	{
		header << "void";
	} else {
		bool first = true;
		for( ; pcurr != pend; ++pcurr)
		{
			if(!first) header << ", ";
			else first = false;
			if(!pcurr->second->genStruct(header, *pcurr->first)) return false;
		}
	}
	header << ");" << std::endl;
	return true;
}

bool FunctionPointer::genStruct(std::ostream& header)
{
	return true;
}

bool FunctionPointer::genStruct(std::ostream& header, std::string name)
{
	header << this->module()->funcPrefix() << this->module()->name() << "_" << this->name() << " " << name;
	return true;
}


bool FunctionPointer::genFunctionDefs(std::ostream& header, Module *Mod)
{
	return true;
}

bool FunctionPointer::create_def_print(std::ostream& f)
{
	return true;
}

bool FunctionPointer::func_def_print(std::ostream& f, std::string fname)
{
	return true;
}

bool FunctionPointer::create_func_print(std::ostream& f)
{
	return false;
}

bool FunctionPointer::destroy_func_print(std::ostream& f)
{
	return false;
}

bool FunctionPointer::ref_func_print(std::ostream& f)
{
	return false;
}

bool FunctionPointer::unref_func_print(std::ostream& f)
{
	return false;
}


bool FunctionPointer::genTemplate(std::ostream& templ)
{
	return true;
}

