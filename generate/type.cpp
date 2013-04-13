#include <generator.h>

using namespace generator;
using namespace std;

bool Type::create_def_print(std::ostream& f)
{
	MemberList::iterator mcurr = memberIterBegin();
	MemberList::iterator mend = memberIterEnd();

	this->genType(f);
	f << " " << this->module()->funcPrefix() << this->name() << "_create(";
	int count = 0;
	for( ; mcurr != mend; ++mcurr)
	{
		if(mcurr->second->isInput())
		{
			if(count != 0)
			f << ", ";
			if(!mcurr->second->genStruct(f, *mcurr->first)) return false;
			count++;
		}
	}
	if(count == 0)
		f << "void";
	f << ")";
	return true;
}

bool Type::func_def_print(std::ostream& f, std::string fname)
{
	f << "bool " << this->module()->funcPrefix() << this->name() << "_" << fname << "(";
	this->genStruct(f, "o");
	f << ")";
	return true;
}

bool Type::print_disconnect(std::ostream& f)
{
	f << this->module()->funcPrefix() << this->name() << "_" << ((nolock && noref) ? "destroy" : "unref");
	return true;	
}

bool Type::print_connect(std::ostream& f)
{
	f << this->module()->funcPrefix() << this->name() << "_ref";
	return true;	
}

bool Type::create_func_print(std::ostream& f)
{
	MemberList::iterator mcurr = memberIterBegin();
	MemberList::iterator mend = memberIterEnd();

	f << " /* Create a " << this->name() << " */" << std::endl;
	f << "{" << std::endl;
	f << "\t";
	this->genStruct(f, "o");
	f << " = calloc(1, sizeof(struct ";
	this->genType(f);
	f << "_s));" << std::endl;
	f << "\tif(o == NULL) goto ERROR_EARLY;\n" << std::endl;
	
	if(!nolock)
	{
		f << "\tpthread_mutexattr_t mutex_attr;\n\tif(pthread_mutexattr_init(&mutex_attr) != 0) goto ERROR_LOCK;" << std::endl;
		f << "\tif(pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK) != 0) goto ERROR_LOCK;" << std::endl;
		f << "\tif(pthread_mutex_init(&o->lock, &mutex_attr) != 0) goto ERROR_LOCK;\n" << std::endl;
		
		if(!noref)
		{
			f << "\to->references = 1;" << std::endl;
			f << std::endl;
		}
	}
	
	for(; mcurr != mend; ++mcurr)
	{
		f << "\to->" << mcurr->first << " = ";
		if(mcurr->second->isInit())
		{
			if(!mcurr->second->isInput())
				f << *mcurr->second->getInit();
			else
				f << *mcurr->first;
		} else {
			f << mcurr->second->get()->initValue();
		}
		f << ";" << std::endl;
	}
	
	f << "\treturn o;" << std::endl;
	
	if(!nolock)
	{
		f << "ERROR_LOCK:" << std::endl;
		f << "\tfree(o);" << std::endl; 
	}
	f << "ERROR_EARLY:" << std::endl;
	f << "\treturn NULL;" << std::endl;
	f << "}" << std::endl;
	f << std::endl;
	return true;
}

bool Type::destroy_func_print(std::ostream& f)
{
	MemberList::iterator mcurr = memberIterBegin();
	MemberList::iterator mend = memberIterEnd();

	f << " /* Destroy a " << this->name() << " */" << std::endl;
	f << "{" << std::endl;
	f << "\tif(o == NULL) return false;" << std::endl;
	f << std::endl;
	
	f << "\tbool res = true;" << std::endl;

	if(noref)
	{
		if(!lock_code_print(f, false)) return false;
	}

	for(; mcurr != mend; ++mcurr)
	{
		if(mcurr->second != NULL)
			if(!mcurr->second->genDestruct(f, mcurr->first)) return false;
	}
	
	if(!unlock_code_print(f)) return false;
	if(!this->destroy_lock_code_print(f)) return false;
	
	f << "\tfree(o);" << std::endl;
	f << "\treturn res;" << std::endl;
	f << "}" << std::endl;
	f << std::endl;
	return true;
}

bool Type::ref_func_print(std::ostream& f)
{
	f << " /* Reference a " << this->name() << " */" << std::endl;
	f << "{" << std::endl;
	if(!lock_code_print(f, false)) return false;
	f << "\to->references++;" << std::endl;
	f << std::endl;
	if(!this->unlock_code_print(f)) return false;
	f << "\treturn true;" << std::endl;
	f << "}" << std::endl;
	f << std::endl;
	return true;
}

bool Type::unref_func_print(std::ostream& f)
{
	f << " /* Unreference a " << this->name() << " */" << std::endl;
	f << "{" << std::endl;
	if(!lock_code_print(f, false)) return false;
	f << "\to->references--;" << std::endl;
	f << std::endl;
	f << "\tif(o->references == 0) return " << this->module()->funcPrefix() << this->name() << "_destroy(o);" << std::endl;
	f << std::endl;
	if(!unlock_code_print(f)) return false;
	f << "\treturn true;" << std::endl;
	f << "}" << std::endl;
	f << std::endl;
	return true;
}

MemberList::iterator Type::memberIterBegin()
{
	return this->members.begin();
}

MemberList::iterator Type::memberIterEnd()
{
	return this->members.end();
}

std::map<std::string *, Function *>::iterator Type::functionIterBegin()
{
	return this->functions.begin();
}

std::map<std::string *, Function *>::iterator Type::functionIterEnd()
{
	return this->functions.end();
}

bool Type::genStruct(std::ostream& header)
{
	MemberList::iterator curr = memberIterBegin();
	MemberList::iterator end = memberIterEnd();
	
	header << "struct " << this->module()->funcPrefix() << this->name() << "_s {" << std::endl;

	header << "\tbool destroyed;" << std::endl;
	if(!this->nolock)
	{
		header << "\tpthread_mutex_t lock;" << std::endl;
		if(!noref)
		{
			header << "\tunsigned int references;" << std::endl;
		}
	}
	
	for( ; curr != end; ++curr)
	{
		header << "\t";
		if(!curr->second->genStruct(header, *curr->first)) return false;
		header << ";" << std::endl;
	}

	header << "};" << std::endl << std::endl;

	return true;
}

bool Type::genFunctionDefs(std::ostream& header, Module *Mod)
{
	std::map<std::string *, Function *>::iterator fcurr = functionIterBegin();
	std::map<std::string *, Function *>::iterator fend = functionIterEnd();
	MemberList::iterator mcurr = memberIterBegin();
	MemberList::iterator mend = memberIterEnd();

	// Constructor
	if(!create_def_print(header)) return false;
	header << " __attribute__((__warn_unused_result__));" << std::endl;
	if(!nolock && !noref)
	{ // Reference counters
		if(!func_def_print(header, "ref")) return false;
		header << " __attribute__((__warn_unused_result__));" << std::endl;
		if(!func_def_print(header, "unref")) return false;
		header << " __attribute__((__warn_unused_result__));" << std::endl;
	} else { // Destructor
		if(!func_def_print(header, "destroy")) return false;
		header << " __attribute__((__warn_unused_result__));" << std::endl;
	}
	
	for( mcurr = memberIterBegin(); mcurr != mend; ++mcurr)
	{
		if(!mcurr->second->isInit())
		{
			if(!mcurr->second->genFunctionDefs(header, mcurr->first, this->module(), this)) return false;
		}
	}
	
	for( ; fcurr != fend; ++fcurr)
	{
		if(!fcurr->second->genFunctionDefs(header, this->module(), this)) return false;	
	}
	
	header << std::endl;
	return true;
}

bool Type::genLogic(std::ostream& logic)
{
	std::map<std::string *, Function *>::iterator fcurr = functionIterBegin();
	std::map<std::string *, Function *>::iterator fend = functionIterEnd();
	MemberList::iterator mcurr = memberIterBegin();
	MemberList::iterator mend = memberIterEnd();

	// Generate the constructor
	if(!create_def_print(logic)) return false;
	if(!create_func_print(logic)) return false;
	
	// Generate the destructor
	if(!nolock && !noref)
		logic << "static ";
	if(!func_def_print(logic, "destroy")) return false;
	if(!destroy_func_print(logic)) return false;
	if(!nolock && !noref)
	{
		if(!func_def_print(logic, "ref")) return false;
		if(!ref_func_print(logic)) return false;
	
		if(!func_def_print(logic, "unref")) return false;
		if(!unref_func_print(logic)) return false;
	}

	for( mcurr = memberIterBegin(); mcurr != mend; ++mcurr)
	{
		if(!mcurr->second->isInit())
		{
			if(!mcurr->second->genLogic(logic, mcurr->first, this->module(), this)) return false;
		}
	}
	
	if(fcurr != fend) logic << "#include <" << this->module()->filePrefix() << this->module()->name() << "_" << this->name() << "_logic.c>" << std::endl << std::endl;
	
	for( ; fcurr != fend; ++fcurr)
	{
		if(!fcurr->second->genLogic(logic, this->module(), this)) return false;	
	}

	return true;
}

bool Type::genTemplate(std::ostream& templ)
{
	std::map<std::string *, Function *>::iterator fcurr = functionIterBegin();
	std::map<std::string *, Function *>::iterator fend = functionIterEnd();

	for( ; fcurr != fend; ++fcurr)
	{
		if(!fcurr->second->genTemplate(templ, this->module(), this)) return false;	
	}

	return true;
}

bool Type::haveFunctions()
{
	if(functionIterBegin() != functionIterEnd())
		return true;
	return false;
}

bool Type::populate_dependencies(std::set<Module *>& deps)
{
	std::map<std::string *, Function *>::iterator fcurr;
	for(fcurr = functionIterBegin(); fcurr != functionIterEnd(); ++fcurr)
	{
		if(!fcurr->second->populate_dependencies(deps))
			return false;
	}
	MemberList::iterator mcurr = memberIterBegin();
	MemberList::iterator mend = memberIterEnd();
	for(; mcurr != mend; ++mcurr)
	{
		deps.insert(mcurr->second->module());
	}
	return true;
}

bool Type::memberAdd(Member<Element> *m, std::string *name)
{
	this->members.push_back(std::make_pair(name, m));
	return true;
}

bool Type::functionAdd(Function *f)
{
	this->functions.insert(std::pair<std::string *, Function *>(f->name(), f));
	return true;
}

bool Type::genType(std::ostream& header)
{
	header << this->module()->funcPrefix() << this->name();
	return true;
}

bool Type::genTypeDef(std::ostream& header)
{
	header << "typedef struct " << this->module()->funcPrefix() << this->name() << "_s *" << this->module()->funcPrefix() << this->name() << ";" << std::endl;
	return true;
}

bool Type::genStruct(std::ostream& header, std::string name)
{
	header << this->module()->funcPrefix() << this->name() << " " << name;
	return true;
}


