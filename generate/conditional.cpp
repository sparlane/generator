#include <generator.h>

using namespace generator;

bool Conditional::genType(std::ostream& header)
{
	header << this->module()->funcPrefix() << this->name();
	return true;
}

bool Conditional::genTypeDef(std::ostream& header)
{
	header << "typedef struct " << this->module()->funcPrefix() << this->name() << "_s *" << this->module()->funcPrefix() << this->name() << ";" << std::endl;
	return true;
}

bool Conditional::genStruct(std::ostream& header)
{
	header << "struct " << this->module()->funcPrefix() << this->name() << "_s {" << std::endl;

	header << "\tbool destroyed;" << std::endl;
	header << "\tpthread_mutex_t *lock;" << std::endl;
	header << "\tpthread_cond_t cond;" << std::endl;
	
	header << "};" << std::endl << std::endl;

	return true;
}

bool Conditional::genStruct(std::ostream& header, std::string name)
{
	header << this->module()->funcPrefix() << this->name() << " " << name;
	return true;
}

bool Conditional::genLockFunctionDef(std::ostream& header, bool unlock)
{
	header << "bool " << this->module()->funcPrefix() << this->name() << (unlock ? "_unlock(" : "_lock(");
	this->genStruct(header, "o");
	header << ")";
	return true;
}

bool Conditional::genWaitFunctionDef(std::ostream& header)
{
	header << "bool " << this->module()->funcPrefix() << this->name() << "_wait(";
	this->genStruct(header, "o");
	header << ")";
	return true;
}

bool Conditional::genSignalFunctionDef(std::ostream& header)
{
	header << "bool " << this->module()->funcPrefix() << this->name() << "_signal(";
	this->genStruct(header, "o");
	header << ",";
	header << "bool all";
	header << ")";
	return true;
}

bool Conditional::genFunctionDefs(std::ostream& header, Module *Mod)
{
	// Constructor
	if(!create_def_print(header)) return false;
	header << " __attribute__((__warn_unused_result__));" << std::endl;
	if(!func_def_print(header, "destroy")) return false;
	header << " __attribute__((__warn_unused_result__));" << std::endl;
	
	if(!this->genLockFunctionDef(header,false)) return false;
	header << ";" << std::endl;
	if(!this->genLockFunctionDef(header,true)) return false;
	header << ";" << std::endl;
	if(!this->genWaitFunctionDef(header)) return false;
	header << ";" << std::endl;
	if(!this->genSignalFunctionDef(header)) return false;
	header << ";" << std::endl;
	
	header << std::endl;
	return true;
}

bool Conditional::genLogic(std::ostream& logic)
{
	logic << "#include <errno.h>" << std::endl;

	create_def_print(logic);
	logic << "{" << std::endl;
	logic << "\t";
	this->genStruct(logic, "o");
	logic << " = calloc(1, sizeof(struct ";
	this->genType(logic);
	logic << "_s));" << std::endl;
	logic << "\tif(o == NULL) goto ERROR_EARLY;" << std::endl;
	logic << std::endl;
	logic << "\to->destroyed = false;" << std::endl;
	
	logic << "/* create the lock for this conditional */" << std::endl;
	logic << "\to->lock = calloc(1, sizeof(pthread_mutex_t));" << std::endl;
	logic << "\tif(o->lock == NULL) goto ERROR_LOCK_CREATE;" << std::endl;
	logic << "\tpthread_mutexattr_t mutex_attr;\n\tif(pthread_mutexattr_init(&mutex_attr) != 0) goto ERROR_LOCK;" << std::endl;
	logic << "\tif(pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK) != 0) goto ERROR_LOCK;" << std::endl;
	logic << "\tif(pthread_mutex_init(o->lock, &mutex_attr) != 0) goto ERROR_LOCK;\n" << std::endl;
	
	
	logic << "/* create the condition */" << std::endl;
	logic << "\tpthread_cond_init(&o->cond, NULL);" << std::endl;
	
	logic << "\treturn o;" << std::endl;
	logic << "ERROR_LOCK:" << std::endl;
	logic << "\tfree(o->lock);" << std::endl; 
	logic << "ERROR_LOCK_CREATE:" << std::endl;
	logic << "\tfree(o);" << std::endl; 
	logic << "ERROR_EARLY:" << std::endl;
	logic << "\treturn NULL;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;

	func_def_print(logic, "destroy");
	logic << std::endl;
	logic << "{" << std::endl;
	logic << "\tif(o == NULL) return false;" << std::endl;
	logic << std::endl;
	logic << "\tif(o->lock == NULL) return false;" << std::endl;
	logic << "\tif(!" << this->module()->funcPrefix() << this->name() << "_lock(o)) return false;" << std::endl;
	logic << "\to->destroyed = true;" << std::endl;
	logic << "\t/* first shake all the threads waiting on this condition */" << std::endl;
	logic << "\t"<< this->module()->funcPrefix() << this->name() << "_signal(o, true);" << std::endl;
	logic << "\t" << this->module()->funcPrefix() << this->name() << "_unlock(o);" << std::endl;
	logic << "\tint eval = pthread_cond_destroy(&o->cond);" << std::endl;
	logic << "\tif(eval != 0)" << std::endl;
	logic << "\t{" << std::endl;
	logic << "\t\tif(eval == EBUSY) return false;" << std::endl;
	logic << "\t}" << std::endl;
	
	// grab and un-grab the lock, to make sure every thread has been shaken
	logic << "\tif(!" << this->module()->funcPrefix() << this->name() << "_lock(o)) return false;" << std::endl;
	logic << "\t" << this->module()->funcPrefix() << this->name() << "_unlock(o);" << std::endl;
	// attempt to destroy the lock
	logic << "\tpthread_mutex_destroy(o->lock);" << std::endl;

	logic << "\tfree(o->lock);" << std::endl;
	logic << "\tfree(o);" << std::endl;
	logic << "\treturn true;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;
	
	genLockFunctionDef(logic, false);
	logic << std::endl;
	logic << "{" << std::endl;

	logic << "\tbool res = true;" << std::endl;

	logic << "\tif(o == NULL || o->lock == NULL) res = false;" << std::endl;
	logic << std::endl;
	logic << "\tif(res) res = (pthread_mutex_lock(o->lock) == 0);" << std::endl;
	logic << std::endl;

	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;
	
	genLockFunctionDef(logic, true);
	logic << std::endl;
	logic << "{" << std::endl;

	logic << "\tbool res = true;" << std::endl;

	logic << "\tif(o == NULL || o->lock == NULL) res = false;" << std::endl;
	logic << std::endl;
	logic << "\tif(res) res = (pthread_mutex_unlock(o->lock) == 0);" << std::endl;
	logic << std::endl;

	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;

	genWaitFunctionDef(logic);
	logic << std::endl;
	logic << "{" << std::endl;

	logic << "\tbool res = true;" << std::endl;

	logic << "\tif(o == NULL || o->lock == NULL) res = false;" << std::endl;
	logic << std::endl;
	logic << "\tif(res) res = (pthread_cond_wait(&o->cond, o->lock) == 0);" << std::endl;
	logic << std::endl;

	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;

	genSignalFunctionDef(logic);
	logic << std::endl;
	logic << "{" << std::endl;

	logic << "\tbool res = true;" << std::endl;

	logic << "\tif(o == NULL || o->lock == NULL) res = false;" << std::endl;
	logic << std::endl;
	logic << "\tif(res) res = (((all) ?  pthread_cond_broadcast(&o->cond) : pthread_cond_signal(&o->cond)) == 0);" << std::endl;
	logic << std::endl;

	logic << "\treturn res;" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;
	return true;
}

bool Conditional::genTemplate(std::ostream& templ)
{
	return true;
}

bool Conditional::create_def_print(std::ostream& f)
{
	this->genType(f);
	f << " " << this->module()->funcPrefix() << this->name() << "_create(void)";
	return true;
}

bool Conditional::func_def_print(std::ostream& f, std::string fname)
{
	f << "bool " << this->module()->funcPrefix() << this->name() << "_" << fname << "(";
	this->genStruct(f, "o");
	f << ")";
	return true;
}

bool Conditional::create_func_print(std::ostream& f)
{
	f << " /* Create a " << this->name() << " */" << std::endl;
	f << "{" << std::endl;
	f << "\t";
	this->genStruct(f, "o");
	f << " = calloc(1, sizeof(struct ";
	this->genType(f);
	f << "_s));" << std::endl;
	f << "if(o == NULL) goto ERROR_EARLY;" << std::endl;
	
	f << "\tbool res = true;" << std::endl;
	f << "\to->lock = malloc(sizeof(pthread_mutex_t));" << std::endl;
	f << "\tif(o->lock == NULL) goto ERROR_ALLOC;" << std::endl;
	f << "\telse {" << std::endl;
	f << "\t\tpthread_mutexattr_t mutex_attr;" << std::endl;
	f << "\t\tif(pthread_mutexattr_init(&mutex_attr) != 0) goto ERROR_LOCK;" << std::endl;
	f << "\t\tif(pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK) != 0) goto ERROR_LOCK;" << std::endl;
	f << "\t\tif(pthread_mutex_init(o->lock, &mutex_attr) != 0) goto ERROR_LOCK;\n" << std::endl;
	f << "\t}" << std::endl;
	f << std::endl;
	
	f << "\to->cond = malloc(sizeof(pthread_cond_t));" << std::endl;
	f << "\tif(o->cond == NULL) goto ERROR_LOCK_DONE;" << std::endl;
	f << "\telse if(pthread_cond_init(&c->cond, NULL) != 0) goto ERROR_COND;" << std::endl;
	f << std::endl;
	f << "\treturn o;";
	
	f << "ERROR_COND:" << std::endl;
	f << "\tfree(o->cond);" << std::endl;
	f << "ERROR_LOCK_DONE:" << std::endl;
	f << "\tpthread_mutex_destroy(o->lock);" << std::endl;
	f << "ERROR_LOCK:" << std::endl;
	f << "\tfree(o->lock);" << std::endl;
	f << "ERROR_ALLOC:" << std::endl;
	f << "\tfree(o);" << std::endl;
	f << "ERROR_EARLY:" << std::endl;
	f << "\treturn NULL;" << std::endl;
	f << "}" << std::endl;
	f << std::endl;
	return true;
}

bool Conditional::destroy_func_print(std::ostream& logic)
{
	logic << "\tif(o->cond != NULL && o->lock != NULL)" << std::endl;
	logic << "\t{" << std::endl;
	this->lock_code_print(logic, false);
	logic << "\t\tpthread_cond_signal(o->cond, true);" << std::endl;
	logic << std::endl;
	this->unlock_code_print(logic);
	this->lock_code_print(logic, false);
	logic << "\tpthread_cond_destroy(o->cond);" << std::endl;
	this->unlock_code_print(logic);
	logic << "\tpthread_mutex_destroy(o->lock);" << std::endl;
	logic << std::endl;
	logic << "\tfree(o->cond);" << std::endl;
	logic << "\tfree(o->lock);" << std::endl;
	logic << std::endl;
	logic << "\to->cond = NULL;" << std::endl;
	logic << "\to->lock = NULL;" << std::endl;
	logic << "\t}" << std::endl;
	return true;
}

bool Conditional::ref_func_print(std::ostream& f)
{
	std::cerr << "Conditional does not support referencing" << std::endl;
	return false;
}
bool Conditional::unref_func_print(std::ostream& f)
{
	std::cerr << "Conditional does not support referencing" << std::endl;
	return false;
}


