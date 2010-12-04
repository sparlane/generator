#include <generator.h>

using namespace generator;
using namespace std;

bool Type::create_def_print(FILE *f)
{
	std::map<std::string *, Member *>::iterator mcurr = memberIterBegin();
	std::map<std::string *, Member *>::iterator mend = memberIterEnd();

	print_to_file(f, "%s %s%s_create(", this->Name->c_str(), Mod->funcPrefix()->c_str(), this->Name->c_str());
	int count = 0;
	for( ; mcurr != mend; ++mcurr)
	{
		if(mcurr->second->isInit())
		{
			if(count != 0)
			print_to_file(f, " ,");
			if(!mcurr->second->genStruct(f)) return false;
			count++;
		}
	}
	if(count == 0)
		print_to_file(f, "void");
	print_to_file(f, ")");
	return true;
}

bool Type::destroy_def_print(FILE *f)
{
	print_to_file(f, "bool %s%s_destroy(%s o)", Mod->funcPrefix()->c_str(), this->Name->c_str(), this->Name->c_str());
	return true;
}

bool Type::ref_def_print(FILE *f)
{
	print_to_file(f, "bool %s%s_ref(%s o)", Mod->funcPrefix()->c_str(), this->Name->c_str(), this->Name->c_str());
	return true;
}

bool Type::unref_def_print(FILE *f)
{
	print_to_file(f, "bool %s%s_unref(%s o)", Mod->funcPrefix()->c_str(), this->Name->c_str(), this->Name->c_str());	
	return true;
}

bool Type::print_disconnect_function(FILE *f)
{
	print_to_file(f, "%s%s_%s", Mod->funcPrefix()->c_str(), this->Name->c_str(), (nolock && noref) ? "destroy" : "unref");
	return true;	
}

bool Type::print_connect_function(FILE *f)
{
	print_to_file(f, "%s%s_ref", Mod->funcPrefix()->c_str(), this->Name->c_str());
	return true;	
}

bool Type::lock_code_print(FILE *f, bool null)
{
	if(!nolock)
	{
		print_to_file(f, "\t{\n\t\tint lock_res = pthread_mutex_lock(&o->lock);\n");
		print_to_file(f, "\t\tif(lock_res != 0) return %s;\n", (null) ? "NULL" : "false");
		print_to_file(f, "\t\tif(o->destroyed)\n\t\t{\n\t\t\tpthread_mutex_unlock(&o->lock);\n\t\t\treturn %s;\n\t\t}\n\t}\n\n", null ? "NULL" : "false");
	}
	return true;
}

bool Type::unlock_code_print(FILE *f)
{
	if(!nolock)
		print_to_file(f, "\t{\n\t\t/* int lock_res = */ pthread_mutex_unlock(&o->lock);\n\t}\n\n");
	return true;
}

bool Type::destroy_lock_code_print(FILE *f)
{
	if(!nolock)
	print_to_file(f, "\tpthread_mutex_destroy(&o->lock);\n");
	return true;
}

bool Type::create_func_print(FILE *f)
{
	std::map<std::string *, Member *>::iterator mcurr = memberIterBegin();
	std::map<std::string *, Member *>::iterator mend = memberIterEnd();

	print_to_file(f, " /* Create a %s */\n", this->Name->c_str());
	print_to_file(f, "{\n");
	print_to_file(f, "\t%s o = calloc(1, sizeof(struct %s_s));\n", this->Name->c_str(), this->Name->c_str());
	print_to_file(f, "\tif(o == NULL) goto ERROR_EARLY;\n\n");
	
	if(!nolock)
	{
		print_to_file(f, "\tpthread_mutexattr_t mutex_attr;\n\tif(pthread_mutexattr_init(&mutex_attr) != 0) goto ERROR_LOCK;\n");
		print_to_file(f, "\tif(pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK) != 0) goto ERROR_LOCK;\n");
		print_to_file(f, "\tif(pthread_mutex_init(&o->lock, &mutex_attr) != 0) goto ERROR_LOCK;\n\n");
		
		if(!noref)
			print_to_file(f, "\to->references = 1;\n\n");
	}
	
	for(; mcurr != mend; ++mcurr)
	{
		if(mcurr->second->isInit())
		{
			if(mcurr->second->isInput())
			{
				print_to_file(f, "\to->%s = %s;\n", mcurr->first->c_str(), mcurr->first->c_str());
			} else {
				print_to_file(f, "\to->%s = %s;\n", mcurr->first->c_str(), mcurr->second->initValue()->c_str());
			}
		}
	}
	
	if(!nolock)
	{
		print_to_file(f, "ERROR_LOCK:\n");
		print_to_file(f, "\tfree(o);\n"); 
	}
	print_to_file(f, "ERROR_EARLY:\n");
	print_to_file(f, "\treturn NULL;\n}\n\n");
	return true;
}

bool Type::destroy_func_print(FILE *f)
{
	std::map<std::string *, Member *>::iterator mcurr = memberIterBegin();
	std::map<std::string *, Member *>::iterator mend = memberIterEnd();

	print_to_file(f, " /* Destroy a %s */\n", this->Name->c_str());
	print_to_file(f, "{\n");
	print_to_file(f, "\tif(o == NULL) return false;\n\n");

	if(noref)
	{
		if(!lock_code_print(f, false)) return false;
	}

	for(; mcurr != mend; ++mcurr)
	{
		if(mcurr->second != NULL)
			if(!mcurr->second->genDestruct(f)) return false;
	}
	
	if(!unlock_code_print(f)) return false;
	if(!destroy_lock_code_print(f)) return false;
	
	print_to_file(f, "\tfree(o);\n");
	print_to_file(f, "\treturn true;\n}\n\n");
	return true;
}

bool Type::ref_func_print(FILE *f)
{
	print_to_file(f, " /* Reference a %s */\n", this->Name->c_str());
	print_to_file(f, "{\n");
	if(!lock_code_print(f, false)) return false;
	print_to_file(f, "\to->references++;\n\n");
	if(!unlock_code_print(f)) return false;
	print_to_file(f, "\treturn true;\n}\n\n");
	return true;
}

bool Type::unref_func_print(FILE *f)
{
	print_to_file(f, " /* Unreference a %s */\n", this->Name->c_str());
	print_to_file(f, "{\n");
	if(!lock_code_print(f, false)) return false;
	print_to_file(f, "\to->references--;\n\n");
	print_to_file(f, "\tif(o->references == 0) return %s%s_destroy(o);\n\n", Mod->funcPrefix()->c_str(), this->Name->c_str());
	if(!unlock_code_print(f)) return false;
	print_to_file(f, "\treturn true;\n}\n\n");
	return true;
}

std::map<std::string *, Member *>::iterator Type::memberIterBegin()
{
	return this->members.begin();
}

std::map<std::string *, Member *>::iterator Type::memberIterEnd()
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

bool Type::genStruct(FILE *header)
{
	std::map<std::string *, Member *>::iterator curr = memberIterBegin();
	std::map<std::string *, Member *>::iterator end = memberIterEnd();
	
	print_to_file(header, "\tbool destroyed;\n");
	if(!this->nolock)
	{
		print_to_file(header, "\tpthread_mutex_t lock;\n");
		if(!noref)
		{
			print_to_file(header, "\tunsigned int references;\n");
		}
	}
	
	for( ; curr != end; ++curr)
	{
		if(!curr->second->genStruct(header)) return false;
	}
	return true;
}

bool Type::genFunctionDefs(FILE *header, Module *Mod)
{
	std::map<std::string *, Function *>::iterator fcurr = functionIterBegin();
	std::map<std::string *, Function *>::iterator fend = functionIterEnd();
	std::map<std::string *, Member *>::iterator mcurr = memberIterBegin();
	std::map<std::string *, Member *>::iterator mend = memberIterEnd();

	// Constructor
	if(!create_def_print(header)) return false;
	print_to_file(header, " __attribute__((__warn_unused_result__));\n");
	if(!nolock && !noref)
	{ // Reference counters
		if(!ref_def_print(header)) return false;
		print_to_file(header, " __attribute__((__warn_unused_result__));\n");
		if(!unref_def_print(header)) return false;
		print_to_file(header, " __attribute__((__warn_unused_result__));\n");
	} else { // Destructor
		if(!destroy_def_print(header)) return false;
		print_to_file(header, " __attribute__((__warn_unused_result__));\n");
	}
	
	for( mcurr = memberIterBegin(); mcurr != mend; ++mcurr)
	{
		if(!mcurr->second->isInit())
		{
			if(!mcurr->second->genFunctionDefs(header, Mod, this)) return false;
		}
	}
	
	for( ; fcurr != fend; ++fcurr)
	{
		if(!fcurr->second->genFunctionDefs(header, Mod, this)) return false;	
	}
	
	print_to_file(header, "\n");
	return true;
}

bool Type::genLogic(FILE *logic)
{
	std::map<std::string *, Function *>::iterator fcurr = functionIterBegin();
	std::map<std::string *, Function *>::iterator fend = functionIterEnd();
	std::map<std::string *, Member *>::iterator mcurr = memberIterBegin();
	std::map<std::string *, Member *>::iterator mend = memberIterEnd();

	// Generate the constructor
	if(!create_def_print(logic)) return false;
	if(!create_func_print(logic)) return false;
	
	// Generate the destructor
	if(!nolock && !noref)
		print_to_file(logic, "static ");
	if(!destroy_def_print(logic)) return false;
	if(!destroy_func_print(logic)) return false;
	if(!nolock && !noref)
	{
		if(!ref_def_print(logic)) return false;
		if(!ref_func_print(logic)) return false;
	
		if(!unref_def_print(logic)) return false;
		if(!unref_func_print(logic)) return false;
	}

	for( mcurr = memberIterBegin(); mcurr != mend; ++mcurr)
	{
		if(!mcurr->second->isInit())
		{
			if(!mcurr->second->genLogic(logic, Mod, this)) return false;
		}
	}
	
	if(fcurr != fend) print_to_file(logic, "#include <%s%s_%s_logic.c>\n\n", Mod->filePrefix()->c_str(), Mod->name()->c_str(), this->Name->c_str());
	
	for( ; fcurr != fend; ++fcurr)
	{
		if(!fcurr->second->genLogic(logic, Mod, this)) return false;	
	}
	return true;
}

bool Type::genTemplate(FILE *templ)
{
	std::map<std::string *, Function *>::iterator fcurr = functionIterBegin();
	std::map<std::string *, Function *>::iterator fend = functionIterEnd();

	for( ; fcurr != fend; ++fcurr)
	{
		if(!fcurr->second->genTemplate(templ, Mod, this)) return false;	
	}

	return true;
}

bool Type::memberAdd(Member *m)
{
	this->members.insert(std::pair<std::string *, Member *>(m->name(), m));
	return true;
}

bool Type::functionAdd(Function *f)
{
	this->functions.insert(std::pair<std::string *, Function *>(f->name(), f));
	return true;
}
