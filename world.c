#include <generator.h>

world *world_create()
{
	world *w = malloc(sizeof(world));
	if(w == NULL) return NULL;
	memset(w, '\0', sizeof(world));
	
	return w;
}

module *world_find_module(world *w, const char *name)
{
	if(w == NULL || name == NULL) return NULL;
	
	for(int i = 0; i < w->module_count; i++)
	{
		if(w->modules[i])
			if(!strcmp(w->modules[i]->name, name))
				return w->modules[i];
	}
	
	module *m = module_create(name);
	if(m == NULL) return NULL;
	
	w->module_count++;
	w->modules = realloc(w->modules, sizeof(module *) * w->module_count);
	if(w->modules == NULL) gen_error("reallocing modules failed");
	
	w->modules[w->module_count-1] = m;
	m->w = w;

	return m;
}

object *world_find_object(world *w, const char *name)
{
	if(w == NULL || name == NULL) return NULL;
	
	for(int i = 0; i < w->object_count; i++)
	{
		if(w->objects[i])
			if(!strcmp(w->objects[i]->name, name))
				return w->objects[i];
	}
	
	return NULL;
}

object *world_create_object(world *w, const char *name)
{
	if(w == NULL || name == NULL) return NULL;
	
	if(world_find_object(w, name) != NULL) gen_error(name);
	
	object *o = object_create(name);
	if(o == NULL) return NULL;
	
	w->object_count++;
	w->objects = realloc(w->objects, sizeof(object *) * w->object_count);
	if(w->objects == NULL) gen_error("reallocing objects failed");
	
	w->objects[w->object_count-1] = o;
	
	o->w = w;
	
	return o;
}

