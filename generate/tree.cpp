#include <generator.h>

using namespace generator;

bool Tree::create_def_print(std::ostream& f)
{
	this->genType(f);
	f << " " << this->module()->funcPrefix() << this->name() << "_create(void)";
	return true;
}

bool Tree::func_def_print(std::ostream& f, std::string fname)
{
	f << "bool " << this->module()->funcPrefix() << this->name() << "_" << fname << "(";
	this->genStruct(f, "o");
	f << ")";
	return true;
}

bool Tree::genInsertFunctionDef(std::ostream& header)
{
	header << "bool " << this->module()->funcPrefix() << this->name() << "_insert(";
	this->genStruct(header, "o");
	if(!this->haveFunctions())
		header << ", unsigned long long k";
	header << ", ";
	this->Of->genStruct(header, "v");
	header << ")";
	return true;
}

bool Tree::genFindFunctionDef(std::ostream& header)
{
	this->Of->genType(header);
	header << " " << this->module()->funcPrefix() << this->name() << "_find(";
	this->genStruct(header, "o");
	header << ", ";
	if(this->haveFunctions())
		this->Of->genStruct(header, "v");
	else
		header << "unsigned long long k";
	header << ")";
	return true;
}

bool Tree::genRemoveFunctionDef(std::ostream& header)
{
	this->Of->genType(header);
	header << " " << this->module()->funcPrefix() << this->name() << "_remove(";
	this->genStruct(header, "o");
	header << ", ";
	if(this->haveFunctions())
		this->Of->genStruct(header, "v");
	else
		header << "unsigned long long k";
	header << ")";
	return true;
}

bool Tree::genNodeStruct(std::ostream& header, std::string name)
{
	header << "struct " << this->module()->funcPrefix() << this->name() << "_node_s *" << name;
	return true;
}

bool Tree::create_func_print(std::ostream& f)
{
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

bool Tree::destroy_func_print(std::ostream& f)
{
	f << " /* Destroy a " << this->name() << " */" << std::endl;
	f << "{" << std::endl;
	f << "\tif(o == NULL) return false;" << std::endl;
	f << std::endl;
	
	f << "\tbool res = true;" << std::endl;

	if(noref)
	{
		if(!lock_code_print(f, false)) return false;
	}

	if(!unlock_code_print(f)) return false;
	if(!destroy_lock_code_print(f)) return false;
	
	f << "\tfree(o);" << std::endl;
	f << "\treturn res;" << std::endl;
	f << "}" << std::endl;
	f << std::endl;
	return true;
}

bool Tree::ref_func_print(std::ostream& f)
{
	f << " /* Reference a " << this->name() << " */" << std::endl;
	f << "{" << std::endl;
	if(!lock_code_print(f, false)) return false;
	f << "\to->references++;" << std::endl;
	f << std::endl;
	if(!unlock_code_print(f)) return false;
	f << "\treturn true;" << std::endl;
	f << "}" << std::endl;
	f << std::endl;
	return true;
}

bool Tree::unref_func_print(std::ostream& f)
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

bool Tree::genStruct(std::ostream& header)
{
	header << "struct " << this->module()->funcPrefix() << this->name() << "_node_s {" << std::endl;
	header << "\t";
	if(!this->Of->genStruct(header, "data")) return false;
	header << ";" << std::endl;
	if(!this->haveFunctions())
		header << "\tunsigned long long idx;" << std::endl;
	header << "\tstruct " << this->module()->funcPrefix() << this->name() << "_node_s *parent;" << std::endl;
	header << "\tstruct " << this->module()->funcPrefix() << this->name() << "_node_s *left;" << std::endl;
	header << "\tstruct " << this->module()->funcPrefix() << this->name() << "_node_s *right;" << std::endl;
	header << "};" << std::endl << std::endl;

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
	
	header << "\tstruct " << this->module()->funcPrefix() << this->name() << "_node_s *root;" << std::endl;
	header << "};" << std::endl << std::endl;

	return true;
}

bool Tree::genFunctionDefs(std::ostream& header, Module *Mod)
{
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
	if(!this->genInsertFunctionDef(header)) return false;
	header << ";" << std::endl;
	if(!this->genFindFunctionDef(header)) return false;
	header << ";" << std::endl;
	if(!this->genRemoveFunctionDef(header)) return false;
	header << ";" << std::endl;
	
	header << std::endl;
	return true;
}

bool Tree::genType(std::ostream& header)
{
	header << this->module()->funcPrefix() << this->name();
	return true;
}

bool Tree::genTypeDef(std::ostream& header)
{
	header << "typedef struct " << this->module()->funcPrefix() << this->name() << "_s *" << this->module()->funcPrefix() << this->name() << ";" << std::endl;
	return true;
}

bool Tree::genStruct(std::ostream& header, std::string name)
{
	header << this->module()->funcPrefix() << this->name() << " " << name;
	return true;
}
