#include <generator.h>

using namespace generator;

bool Queue::create_def_print(std::ostream& f)
{
	this->genType(f);
	f << " " << this->module()->funcPrefix() << this->name() << "_create(void)";
	return true;
}

bool Queue::func_def_print(std::ostream& f, std::string fname)
{
	f << "bool " << this->module()->funcPrefix() << this->name() << "_" << fname << "(";
	this->genStruct(f, "o");
	f << ")";
	return true;
}

bool Queue::genPushFunctionDef(std::ostream& header)
{
	header << "bool " << this->module()->funcPrefix() << this->name() << "_push(";
	this->genStruct(header, "o");
	header << ", ";
	this->Of->genStruct(header, "v");
	header << ")";
	return true;
}

bool Queue::genPopFunctionDef(std::ostream& header)
{
	this->Of->genType(header);
	header << " " << this->module()->funcPrefix() << this->name() << "_pop(";
	this->genStruct(header, "o");
	header << ")";
	return true;
}

bool Queue::genSizeFunctionDef(std::ostream& header)
{
	header << "size_t " << this->module()->funcPrefix() << this->name() << "_size(";
	this->genStruct(header, "o");
	header << ")";
	return true;
}


bool Queue::create_func_print(std::ostream& f)
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

bool Queue::destroy_func_print(std::ostream& f)
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

bool Queue::ref_func_print(std::ostream& f)
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

bool Queue::unref_func_print(std::ostream& f)
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

bool Queue::genStruct(std::ostream& header)
{
	header << "struct " << this->module()->funcPrefix() << this->name() << "_elmnt_s {" << std::endl;
	header << "\t";
	if(!this->Of->genStruct(header, "data")) return false;
	header << ";" << std::endl;
	header << "\tstruct " << this->module()->funcPrefix() << this->name() << "_elmnt_s *next;" << std::endl;
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
	
	header << "\tstruct " << this->module()->funcPrefix() << this->name() << "_elmnt_s *first;" << std::endl;
	header << "\tstruct " << this->module()->funcPrefix() << this->name() << "_elmnt_s *last;" << std::endl;
	header << "\tsize_t size;" << std::endl;	
	header << "};" << std::endl << std::endl;

	return true;
}

bool Queue::genFunctionDefs(std::ostream& header, Module *Mod)
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
	if(!this->genPushFunctionDef(header)) return false;
	header << ";" << std::endl;
	if(!this->genPopFunctionDef(header)) return false;
	header << ";" << std::endl;
	if(!this->genSizeFunctionDef(header)) return false;
	header << ";" << std::endl;
	
	header << std::endl;
	return true;
}

bool Queue::genLogic(std::ostream& logic)
{
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
	
	// Generate each of our functions
	if(!this->genPushFunctionDef(logic)) return false;
	logic << std::endl;
	logic << "{" << std::endl;
	logic << "\tif(o == NULL) return false;" << std::endl;
	logic << std::endl;
	
	logic << "\tbool res = true;" << std::endl;

	if(!this->lock_code_print(logic, false)) return false;

	logic << "\tstruct " << this->module()->funcPrefix() << this->name() << "_elmnt_s *nE";
	logic << " = malloc(sizeof(struct " << this->module()->funcPrefix() << this->name() << "_elmnt_s";
	logic << "));" << std::endl;
	logic << "\tif(nE == NULL) res = false;" << std::endl;
	logic << std::endl;
	
	logic << "\tif(res)" << std::endl;
	logic << "\t\{" << std::endl;
	logic << "\t\tnE->data = v;" << std::endl;
	logic << "\t\tnE->next = NULL;" << std::endl;
	logic << "\t}" << std::endl;
	logic << std::endl;

	logic << "\tif(res)" << std::endl;
	logic << "\t\{" << std::endl;
	logic << "\t\tif(o->last == NULL)" << std::endl;
	logic << "\t\t{" << std::endl;
	logic << "\t\t\to->first = o->last = nE;" << std::endl;
	logic << "\t\t\to->size = 1;" << std::endl;
	logic << "\t\t} else {" << std::endl;
	logic << "\t\t\to->last->next = nE;" << std::endl;
	logic << "\t\t\to->last = nE;" << std::endl;
	logic << "\t\t\to->size++;" << std::endl;
	logic << "\t\t}" << std::endl;
	logic << "\t}" << std::endl;
	logic << std::endl;

	if(!this->unlock_code_print(logic)) return false;
	
	logic << "\treturn res;" << std::endl;

	logic << "}" << std::endl;
	logic << std::endl;

	if(!this->genPopFunctionDef(logic)) return false;
	logic << std::endl;
	logic << "{" << std::endl;
	logic << "\tif(o == NULL) return false;" << std::endl;
	logic << std::endl;
	
	logic << "\t";
	this->Of->genStruct(logic, "res");
	logic << " = " << this->Of->initValue();
	logic << ";" << std::endl;

	if(!this->lock_code_print(logic, false)) return false;

	logic << "\tstruct " << this->module()->funcPrefix() << this->name() << "_elmnt_s *tE";
	logic << " = o->first;" << std::endl;
	logic << "\tif(tE != NULL) res = tE->data;" << std::endl;
	logic << std::endl;
	
	logic << "\tif(res != " << this->Of->initValue() << ")" << std::endl;
	logic << "\t\{" << std::endl;
	logic << "\t\to->size--;" << std::endl;
	logic << "\t\to->first = tE->next;" << std::endl;
	logic << "\t}" << std::endl;
	logic << std::endl;

	logic << "\tif(res != " << this->Of->initValue() << ")" << std::endl;
	logic << "\t\{" << std::endl;
	logic << "\t\tif(o->last == tE)" << std::endl;
	logic << "\t\t{" << std::endl;
	logic << "\t\t\to->first = o->last = NULL;" << std::endl;
	logic << "\t\t\to->size = 0;" << std::endl;
	logic << "\t\t}" << std::endl;
	logic << "\t\tfree(tE);" << std::endl;
	logic << "\t}" << std::endl;
	logic << std::endl;

	if(!this->unlock_code_print(logic)) return false;
	
	logic << "\treturn res;" << std::endl;

	logic << "}" << std::endl;
	logic << std::endl;
	
	if(!this->genSizeFunctionDef(logic)) return false;
	logic << std::endl;
	logic << "{" << std::endl;
	logic << "\tif(o == NULL) return 0;" << std::endl;
	logic << std::endl;
	
	if(!this->lock_code_print(logic, false)) return false;

	logic << "\tsize_t res = o->size;" << std::endl;

	logic << std::endl;

	if(!this->unlock_code_print(logic)) return false;
	
	logic << "\treturn res;" << std::endl;

	logic << "}" << std::endl;
	logic << std::endl;


	return true;
}

bool Queue::genTemplate(std::ostream& templ)
{
	return true;
}

bool Queue::genType(std::ostream& header)
{
	header << this->module()->funcPrefix() << this->name();
	return true;
}

bool Queue::genTypeDef(std::ostream& header)
{
	header << "typedef struct " << this->module()->funcPrefix() << this->name() << "_s *" << this->module()->funcPrefix() << this->name() << ";" << std::endl;
	return true;
}

bool Queue::genStruct(std::ostream& header, std::string name)
{
	header << this->module()->funcPrefix() << this->name() << " " << name;
	return true;
}
