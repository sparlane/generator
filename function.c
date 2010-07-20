#include <generator.h>

function *function_create(const char *name)
{
	function *func = calloc(1, sizeof(function));
	if(func == NULL) return NULL;

	func->name = strdup(name);
	
	return func;
}
