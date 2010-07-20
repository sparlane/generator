#include <generator.h>

object *object_create(const char *name)
{
	if(name == NULL) return false;
	
	object *o = malloc(sizeof(object));
	if(o == NULL) return NULL;
	memset(o, '\0', sizeof(object));
	
	o->name = strdup(name);
	
	return o;
}

bool object_add_member(object *o, member *m)
{
	if(o == NULL || m == NULL) return false;
	
	o->member_count++;
	o->members = realloc(o->members, sizeof(member *) * o->member_count);
	if(o->members == NULL)
		gen_error("reallocing members");
	
	o->members[o->member_count-1] = m;
	
	return true;
}

function *object_add_function(object *o, const char *name)
{
	if(o == NULL || name == NULL) return NULL;
	
	for(int i = 0; i < o->function_count; i++)
	{
		if(!strcmp(o->functions[i]->name, name))
			return false;
	}
	
	o->function_count++;
	o->functions = realloc(o->functions, o->function_count * sizeof(function *));
	
	o->functions[o->function_count-1] = function_create(name);
	
	return o->functions[o->function_count-1];
}
