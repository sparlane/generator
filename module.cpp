#include <generator.h>

using namespace generator;
using namespace std;

static char *to_upper_string(const char *str)
{
	if(str == NULL) return NULL;
	size_t len = strlen(str);
	char *ret = (char *)malloc(len + 1);
	for(int i = 0; i < len; i++)
	{
		ret[i] = toupper(str[i]);
	}
	ret[len] = '\0';
	return ret;
}

std::map<std::string *, Type *>::iterator Module::objectsIterBegin()
{
	return this->Objects->begin();
}

std::map<std::string *, Type *>::iterator Module::objectsIterEnd()
{
	return this->Objects->end();
}

bool Module::objectAdd(std::string *oName, Type *object)
{
	this->Objects->insert(std::pair<std::string *, Type *>(oName, object));
	return true;
}

bool Module::generate(std::string *name)
{
	std::map<std::string *, Type *>::iterator curr = objectsIterBegin();
	std::map<std::string *, Type *>::iterator end = objectsIterEnd();

	char *path = NULL;
	int res = asprintf(&path, "output/%s", this->Path->c_str());
	if(res <= 0)
		gen_error(strerror(errno));
	res = mkdir(path, 0755);
	if(res != 0 && errno != EEXIST)
		gen_error(strerror(errno));
	free(path);
	path = NULL;
	res = asprintf(&path, "output/%s/%s", this->Path->c_str(), name->c_str());
	if(res <= 0)
		gen_error(strerror(errno));
	res = mkdir(path, 0755);
	if(res != 0 && errno != EEXIST)
		gen_error(strerror(errno));
	free(path);
	path = NULL;
	res = asprintf(&path, "output/%s/%s/include", this->Path->c_str(), name->c_str());
	if(res <= 0)
		gen_error(strerror(errno));
	res = mkdir(path, 0755);
	if(res != 0 && errno != EEXIST)
		gen_error(strerror(errno));
	free(path);
	path = NULL;

	// generate the header file first
	res = asprintf(&path, "output/%s/%s/include/%s%s.h", this->Path->c_str(), name->c_str(), this->FilePrefix->c_str(), name->c_str());
	if(res <= 0)
		gen_error(strerror(errno));
	FILE *header = fopen(path, "w");
	if(header == NULL)
		gen_error(strerror(errno));

	free(path);
	path = NULL;

	char *modUpperName = to_upper_string(name->c_str());
	char *modUpperPrefix = to_upper_string(this->FilePrefix->c_str());
	
	print_to_file(header, "/* DO NOT EDIT: This file was automatically generated */\n");
	print_to_file(header, "#ifndef %s%s_H\n#define %s%s_H\n\n", modUpperPrefix, modUpperName, modUpperPrefix, modUpperName);
	
	// print the includes
	print_to_file(header, "#include <stdbool.h>\n");
	print_to_file(header, "#include <stdlib.h>\n");
	// (pthread.h is needed for locking)
	print_to_file(header, "#include <pthread.h>\n");
	print_to_file(header, "\n");

	for( ; curr != end ; ++curr)
	{
		print_to_file(header, "typedef struct %s_s *%s;\n", curr->first->c_str(), curr->first->c_str());
	}
	print_to_file(header, "\n");

	for(curr = objectsIterBegin(); curr != end; ++curr)
	{
		print_to_file(header, "struct %s_s {\n", curr->first->c_str());
		if(!curr->second->genStruct(header)) return false;
		print_to_file(header, "};\n\n");
	}

	for(curr = objectsIterBegin(); curr != end; ++curr)
	{
		if(!curr->second->genFunctionDefs(header, this)) return false;
	}
	print_to_file(header, "\n");

	print_to_file(header, "#endif /* %s%s_H */\n", modUpperPrefix, modUpperName);
	fflush(header);
	fclose(header);
	header = NULL;
	
	free(modUpperName);
	free(modUpperPrefix);

	// now generate each of the logic files
	for(curr = objectsIterBegin() ; curr != end ; ++curr)
	{
		res = asprintf(&path, "output/%s/%s/%s%s_%s.c", this->Path->c_str(), name->c_str(), this->FilePrefix->c_str(), name->c_str(), curr->first->c_str());
		if(res <= 0)
			gen_error(strerror(errno));
		
		FILE *logic = fopen(path, "w");
		if(logic == NULL)
			gen_error(strerror(errno));

		free(path);
		path = NULL;
		
		if(!curr->second->genLogic(logic))
			return false;
	
		fflush(logic);
		fclose(logic);
	}
	
	// now generate each of the template files
	for(curr = objectsIterBegin() ; curr != end ; ++curr)
	{
		res = asprintf(&path, "output/%s/%s/%s%s_%s_logic.c.tpl", this->Path->c_str(), name->c_str(), this->FilePrefix->c_str(), name->c_str(), curr->first->c_str());
		if(res <= 0)
			gen_error(strerror(errno));
		
		FILE *tmpl = fopen(path, "w");
		if(path == NULL)
			gen_error(strerror(errno));
		
		if(!curr->second->genTemplate(tmpl)) return false;
	
		fflush(tmpl);
		fclose(tmpl);

		free(path);
		path = NULL;
	}
	
	// lastly generate the luabuild script
	// TODO

	return true;
}
