#include <generator.h>

using namespace generator;
using namespace std;

static char *to_upper_string(const char *str)
{
	if(str == NULL) return NULL;
	size_t len = strlen(str);
	char *ret = (char *)malloc(len + 1);
	for(size_t i = 0; i < len; i++)
	{
		ret[i] = toupper(str[i]);
	}
	ret[len] = '\0';
	return ret;
}

std::map<std::string *, Object *>::iterator Module::objectsIterBegin()
{
	return this->Objects->begin();
}

std::map<std::string *, Object *>::iterator Module::objectsIterEnd()
{
	return this->Objects->end();
}

bool Module::objectAdd(std::string *oName, Object *object)
{
	this->Objects->insert(std::pair<std::string *, Object *>(oName, object));
	return true;
}

std::map<std::string *, Object *>::iterator Module::fpIterBegin()
{
	return this->FunctionPointers->begin();
}

std::map<std::string *, Object *>::iterator Module::fpIterEnd()
{
	return this->FunctionPointers->end();
}

bool Module::fpAdd(std::string *oName, Object *object)
{
	this->FunctionPointers->insert(std::pair<std::string *, Object *>(oName, object));
	return true;
}

bool Module::generate(std::string *name, const char *output_dir)
{
	std::map<std::string *, Object *>::iterator curr = objectsIterBegin();
	std::map<std::string *, Object *>::iterator end = objectsIterEnd();
	std::map<std::string *, Object *>::iterator fp_curr = fpIterBegin();
	std::map<std::string *, Object *>::iterator fp_end = fpIterEnd();
	std::set<Module *> dependencies;

	char *path = NULL;
	int res = asprintf(&path, "%s/%s", output_dir, this->Path->c_str());
	if(res <= 0)
		gen_error(strerror(errno));
	res = mkdir(path, 0755);
	if(res != 0 && errno != EEXIST)
		gen_error(strerror(errno));
	free(path);
	path = NULL;
	res = asprintf(&path, "%s/%s/%s", output_dir, this->Path->c_str(), name->c_str());
	if(res <= 0)
		gen_error(strerror(errno));
	res = mkdir(path, 0755);
	if(res != 0 && errno != EEXIST)
		gen_error(strerror(errno));
	free(path);
	path = NULL;
	res = asprintf(&path, "%s/%s/%s/include", output_dir, this->Path->c_str(), name->c_str());
	if(res <= 0)
		gen_error(strerror(errno));
	res = mkdir(path, 0755);
	if(res != 0 && errno != EEXIST)
		gen_error(strerror(errno));
	free(path);
	path = NULL;
	res = asprintf(&path, "%s/lb/%s", output_dir, this->Path->c_str());
	if(res <= 0)
		gen_error(strerror(errno));
	res = mkdir(path, 0755);
	if(res != 0 && errno != EEXIST)
		gen_error(strerror(errno));
	free(path);
	path = NULL;
	// generate the header file first
	res = asprintf(&path, "%s/%s/%s/include/%s%s.h", output_dir, this->Path->c_str(), name->c_str(), this->FilePrefix->c_str(), name->c_str());
	if(res <= 0)
		gen_error(strerror(errno));
	
	std::ofstream header(path);
	
	free(path);
	path = NULL;
	
	char *modUpperName = to_upper_string(name->c_str());
	char *modUpperPrefix = to_upper_string(this->FilePrefix->c_str());
	
	header << "/* DO NOT EDIT: This file was automatically generated */" << std::endl;
	header << "#ifndef " << modUpperPrefix << modUpperName << "_H" << std::endl;
	header << "#define " << modUpperPrefix << modUpperName << "_H" << std::endl << std::endl;
	
	// print the includes
	header << "#include <stdbool.h>" << std::endl;
	header << "#include <stdlib.h>" << std::endl;
	// (pthread.h is needed for locking)
	header << "#include <pthread.h>" << std::endl;
	
	for(curr = objectsIterBegin(); curr != end; ++curr)
	{
		curr->second->populate_dependencies(dependencies);
	}

	for(std::set<Module *>::iterator iter = dependencies.begin(); iter != dependencies.end(); ++iter)
	{
		if((*iter) != NULL && (*iter) != this)
		{
			char *inc_file = NULL;
			asprintf(&inc_file, "%s%s.h", (*iter)->filePrefix()->c_str(), (*iter)->name()->c_str());
			this->includes.insert(std::string(inc_file));
			free(inc_file);
		}
	}

	for(std::set<std::string>::iterator I = this->includes.begin(); I != this->includes.end(); I++)
	{
		header << "#include <" << *I << ">" << std::endl;
	}
	
	header << std::endl;

	for(curr = objectsIterBegin() ; curr != end ; ++curr)
	{
		curr->second->genTypeDef(header);
	}
	header << std::endl;

	for( ; fp_curr != fp_end ; ++fp_curr)
	{
		fp_curr->second->genTypeDef(header);
	}
	header << std::endl;

	for(curr = objectsIterBegin(); curr != end; ++curr)
	{
		if(!curr->second->genStruct(header))
		{
			std::cerr << "Error generating structure: " << curr->first << std::endl;
			return false;
		}
	}

	for(curr = objectsIterBegin(); curr != end; ++curr)
	{
		if(!curr->second->genFunctionDefs(header, this))
		{
			std::cerr << "Error generating function definitions: " << curr->first << std::endl;
			return false;
		}
	}
	header << std::endl;

	header << "#endif /* " << modUpperPrefix << modUpperName << "_H */" << std::endl;
	header.flush();
	
	free(modUpperName);
	free(modUpperPrefix);

	// now generate each of the logic files
	for(curr = objectsIterBegin() ; curr != end ; ++curr)
	{
		if(curr->second->haveLogic())
		{
			res = asprintf(&path, "%s/%s/%s/%s%s_%s.c", output_dir, this->Path->c_str(), name->c_str(), this->FilePrefix->c_str(), name->c_str(), curr->first->c_str());
			if(res <= 0)
				gen_error(strerror(errno));
		
			std::ofstream logic(path);
			free(path);
			path = NULL;
		
			logic << "#include <" << this->FilePrefix << name << ".h>" << std::endl;
		
			if(!curr->second->genLogic(logic))
			{
				std::cerr << "Error generating logic: " << curr->first << std::endl;
				return false;
			}
			logic.flush();
		}
	}
	
	// now generate each of the template files
	for(curr = objectsIterBegin() ; curr != end ; ++curr)
	{
		if(curr->second->haveFunctions())
		{
			res = asprintf(&path, "%s/%s/%s/%s%s_%s_logic.c.tpl", output_dir, this->Path->c_str(), name->c_str(), this->FilePrefix->c_str(), name->c_str(), curr->first->c_str());
			if(res <= 0)
				gen_error(strerror(errno));
		
			std::ofstream tmpl(path);
		
			if(!curr->second->genTemplate(tmpl)) return false;

			tmpl.flush();

			free(path);
			path = NULL;
		}
	}
	
	// Generate some makefile data
	{
		res = asprintf(&path, "%s/%s/%s/Makefile.%s", output_dir, this->Path->c_str(), name->c_str(), name->c_str());
		
		if(res <= 0)
			gen_error(strerror(errno));
	
		std::ofstream mkout(path);

		// for each file we produce, add it
		for(curr = objectsIterBegin() ; curr != end ; ++curr)
		{
			if(curr->second->haveLogic())
			{
				mkout << "SRC_C+= " << this->Path << "/" << name << "/" << this->FilePrefix << name << "_" << curr->first << ".c" << std::endl;
			}
		}

		// add our path to cflags
		mkout << "INCLUDES+= -I" << this->Path << "/" << name << "/include/" << std::endl;
		mkout << "INCLUDES+= -I" << this->Path << "/" << name << std::endl;

		// add our path
		mkout << "PATHS+= " << this->Path << "/" << name << std::endl;

		mkout.flush();

		free(path);
		path = NULL;


	}

	// lastly generate the luabuild script
	{
		res = asprintf(&path, "%s/lb/%s/%s.lua", output_dir, this->Path->c_str(), name->c_str());
		
		if(res <= 0)
			gen_error(strerror(errno));
	
		std::ofstream lbout(path);
	
		lbout << "function " << this->Path << "_" << name << "_dep(files,cflags,deps,done)" << std::endl;

		lbout << "\tif done." << this->Path << "_" << name << " == nil then" << std::endl;
		lbout << "\t\tdone['" << this->Path << "_" << name << "'] = true" << std::endl;
		lbout << "\t\t" << std::endl;
		// for each other module we use, depend on it
		// for each file we produce, add it
		for(curr = objectsIterBegin() ; curr != end ; ++curr)
		{
			if(curr->second->haveLogic())
			{
				lbout << "\t\ttable.insert(files, file('" << this->Path 
					<< "/" << name << "/" << this->FilePrefix << name 
					<< "_" << curr->first << ".c'))" << std::endl;
			}
		}
		// add the logic files (if needed)
		for(curr = objectsIterBegin() ; curr != end ; ++curr)
		{
			if(curr->second->haveFunctions())
			{
				lbout << "\t\ttable.insert(deps, file('" << this->Path 
					<< "/" << name << "/" << this->FilePrefix << name 
					<< "_" << curr->first << "_logic.c'))" << std::endl;
			}
		}		
		// add our header to the dependencies list
		lbout << "\t\ttable.insert(deps, file('" << this->Path << "/"
			<< name << "/include/" << this->FilePrefix << name << ".h'))"
			<< std::endl;
		
		lbout << std::endl;
		// depend on other modules that we need/use
		for(std::set<Module *>::iterator iter = dependencies.begin(); iter != dependencies.end(); ++iter)
		{
			if((*iter) != NULL && (*iter) != this)
			{
				lbout << "\t\t" << (*iter)->path() << "_" << (*iter)->name() << "_dep(files,cflags,deps,done)" << std::endl;
			}
		}
		lbout << "\tend" << std::endl;
		
		lbout << "end" << std::endl;

		lbout.flush();
		
		free(path);
		path = NULL;
	}
	return true;
}
