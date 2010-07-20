#include <generator.h>

module *module_create(const char *name)
{
	if(name == NULL) return false;
	
	module *m = malloc(sizeof(module));
	if(m == NULL) return NULL;
	memset(m, '\0', sizeof(module));
	
	m->name = strdup(name);
	
	return m;
}

bool module_add_object(module *m, object *o)
{
	if(m == NULL || o == NULL) return false;
	
	m->object_count++;
	m->objects = realloc(m->objects, sizeof(object *) * m->object_count);
	if(m->objects == NULL) gen_error("reallocing objects");
	
	m->objects[m->object_count-1] = o;
	o->m = m;
	
	return true;
}

bool module_add_include(module *m, const char *inc)
{
	if(m == NULL || inc == NULL) return false;
	
	for(int i = 0; i < m->include_count; i++)
	{
		if(!strcmp(m->includes[i], inc)) return false;
	}
	
	m->include_count++;
	m->includes = realloc(m->includes, sizeof(char *) * m->include_count);
	if(m->includes == NULL) gen_error("reallocing includes");
	
	m->includes[m->include_count-1] = strdup(inc);
	
	return true;
}

bool module_add_depend(module *m, const char *dep)
{
	if(m == NULL || dep == NULL) return false;
	
	for(int i = 0; i < m->module_count; i++)
	{
		if(!strcmp(m->modules[i], dep)) return false;
	}
	
	m->module_count++;
	m->modules = realloc(m->modules, sizeof(char *) * m->module_count);
	if(m->modules == NULL) gen_error("reallocing modules");
	
	m->modules[m->module_count-1] = strdup(dep);
	
	return true;
}
