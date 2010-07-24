#include <generator.h>

#define process_error(S,E) process_error_x(S,E,__FUNCTION__,__LINE__)

// module *m = moduleCreate(name, path, header_prefix, function_prefix, struct_prefix)
int process_module_create(lua_State *L)
{
	if(lua_gettop(L) <= 2) gen_error("moduleCreate() requires at least 2 arguments");
	if(lua_gettop(L) > 5) gen_error("moduleCreate() requires at most 5 arguments");
	if(lua_type(L, 1) != LUA_TSTRING) gen_error("moduleCreate() requires a string as the first argument");
	if(lua_type(L, 2) != LUA_TSTRING) gen_error("moduleCreate() requires a string as the second argument");
	if(lua_gettop(L) >= 3 && lua_type(L, 3) != LUA_TSTRING) gen_error("moduleCreate() requires a string as the third argument");
	if(lua_gettop(L) >= 4 && lua_type(L, 4) != LUA_TSTRING) gen_error("moduleCreate() requires a string as the fourth argument");
	if(lua_gettop(L) == 5 && lua_type(L, 5) != LUA_TSTRING) gen_error("moduleCreate() requires a string as the fifth argument");

	module *m = world_find_module(WORLD, lua_tostring(L, 1));
	if(m == NULL) gen_error("module not found, or could not be created");
	
	m->path = strdup(lua_tostring(L, 2));
	
	if(lua_gettop(L) >= 3)
		m->file_prefix = strdup(lua_tostring(L, 3));
	
	if(lua_gettop(L) >= 4)
		m->function_prefix = strdup(lua_tostring(L, 4));

	if(lua_gettop(L) >= 5)
		m->struct_prefix = strdup(lua_tostring(L, 5));

	lua_pushlightuserdata(L, m);
	return 1;
}

// object *o = objectCreate(module, name, lock, ref)
int process_object_create(lua_State *L)
{
	if(lua_gettop(L) != 4) gen_error("objectCreate() requires exactly 4 arguments");
	if(lua_type(L, 1) != LUA_TSTRING) gen_error("objectCreate() requires a string as the first argument");
	if(lua_type(L, 2) != LUA_TSTRING) gen_error("objectCreate() requires a string as the second argument");
	if(lua_type(L, 3) != LUA_TBOOLEAN) gen_error("objectCreate() requires a boolean as the third argument");
	if(lua_type(L, 4) != LUA_TBOOLEAN) gen_error("objectCreate() requires a boolean as the fourth argument");

	module *m = world_find_module(WORLD, lua_tostring(L, 1));
	if(m == NULL) gen_error("module not found, or could not be created");
	
	object *o = world_create_object(WORLD, lua_tostring(L, 2));
	if(o == NULL) gen_error("object already exists, or could not be created");
	module_add_object(m, o);
	o->m = m;

	o->locked = lua_toboolean(L, 3);
	if(o->locked)
		o->refcount = lua_toboolean(L, 4);

	lua_pushlightuserdata(L, o);
	return 1;
}

// bool objectAddObject(object, object, name, init, input, {params})
int process_object_add_object(lua_State *L)
{
	if(lua_gettop(L) < 3) gen_error("objectAddObject() requires at least 3 arguments");
	if(lua_gettop(L) > 6) gen_error("objectAddObject() requires no more than 6 arguments");
	if(lua_type(L, 1) != LUA_TLIGHTUSERDATA) gen_error("objectAddObject() requires an object as the first argument");
	if(lua_type(L, 2) != LUA_TLIGHTUSERDATA) gen_error("objectAddObject() requires an object as the second argument");
	if(lua_type(L, 3) != LUA_TSTRING) gen_error("objectAddObject() requires a string as the third argument");
	bool init = false;
	bool input = false;
	if(lua_gettop(L) >= 4)
	{
		if(lua_type(L, 4) != LUA_TBOOLEAN) gen_error("objectAddObject() requires a boolean as the fourth argument (if present)");
		init = lua_toboolean(L, 4);
		if(init) input = true;
	}
	if(lua_gettop(L) >= 5)
	{
		if(lua_type(L, 5) != LUA_TBOOLEAN) gen_error("objectAddObject() requires a boolean as the fifth argument (if present)");
		input = lua_toboolean(L, 5);
	}
	if(lua_gettop(L) == 6)
	{
		if(lua_type(L, 6) != LUA_TTABLE) gen_error("objectAddObject() requires a table as the sixth argument (if present)");
	}

	object *o1 = (object *)lua_topointer(L, 1);
	if(o1 == NULL) gen_error("object could not be found");
	
	object *o2 = (object *)lua_topointer(L, 2);
	if(o2 == NULL) gen_error("object could not be found");

	// create a member and reference o2, and store it in o1
	member *m = calloc(1,sizeof(member));
	if(m == NULL)
		gen_error("Allocating member failed");
	
	m->name = strdup(lua_tostring(L, 3));
	m->type = member_type_object;
	m->o = o2;
	m->init = init;
	m->init_input = input;
	
	if(lua_gettop(L) == 6)
	{
		lua_pushnil(L);  /* first key */
		while (lua_next(L, 6) != 0) {
			/* uses 'key' (at index -2) and 'value' (at index -1) */
			if(lua_type(L, -1) != LUA_TSTRING) gen_error("objectAddObject() requires a table of strings as the sixth argument (if present)\n");
			m->param_count++;
			m->init_params = realloc(m->init_params, sizeof(char *) * m->param_count);
			m->init_params[m->param_count-1] = strdup(lua_tostring(L, -1));
			/* removes 'value'; keeps 'key' for next iteration */
			lua_pop(L, 1);
		}
	}
	
	if(!object_add_member(o1, m))
		gen_error("Failed adding member to object");
	
	lua_pushboolean(L, true);
	return 1;
}

// bool objectAddType(object, type, name, init, input, value)
int process_object_add_type(lua_State *L)
{
	if(lua_gettop(L) < 3) gen_error("objectAddType() requires at least 3 arguments");
	if(lua_gettop(L) > 6) gen_error("objectAddType() requires no more than 6 arguments");
	if(lua_type(L, 1) != LUA_TLIGHTUSERDATA) gen_error("objectAddType() requires an object as the first argument");
	if(lua_type(L, 2) != LUA_TSTRING) gen_error("objectAddType() requires a string as the second argument");
	if(lua_type(L, 3) != LUA_TSTRING) gen_error("objectAddType() requires a string as the third argument");
	bool init = false;
	bool input = false;
	const char *value = NULL;
	if(lua_gettop(L) >= 4)
	{
		if(lua_type(L, 4) != LUA_TBOOLEAN) gen_error("objectAddType() requires a boolean as the fourth argument (if present)");
		init = lua_toboolean(L, 4);
		if(init) input = true;
	}
	if(lua_gettop(L) >= 5)
	{
		if(lua_type(L, 5) != LUA_TBOOLEAN) gen_error("objectAddType() requires a boolean as the fifth argument (if present)");
		input = lua_toboolean(L, 5);
	}
	if(lua_gettop(L) == 6)
	{
		if(lua_type(L, 6) != LUA_TSTRING) gen_error("objectAddType() requires a string as the sixth argument (if present)");
		value = lua_tostring(L, 6);
	}

	if(init && !input && value == NULL)
		gen_error("Must specify value, if initializing, but not taking as input");

	object *o1 = (object *)lua_topointer(L, 1);
	if(o1 == NULL) gen_error("object could not be found");


	// create a member with type = type, and store it in o1
	member *m = calloc(1,sizeof(member));
	if(m == NULL)
		gen_error("Allocating member failed");
	
	m->name = strdup(lua_tostring(L, 3));
	m->type = member_type_type;
	m->type_name = strdup(lua_tostring(L, 2));
	m->init = init;
	m->init_input = input;
	if(value != NULL)
	{
		m->param_count = 1;
		m->init_params = calloc(1,sizeof(char *));
		if(m->init_params == NULL)
			gen_error("failed allocating init_params");
		m->init_params[0] = strdup(value);
	}

	if(!object_add_member(o1, m))
		gen_error("Failed adding member to object");


	lua_pushboolean(L, true);
	return 1;
}
// bool objectAddPointer(object, type, name, destroy, init, input, create)
int process_object_add_pointer(lua_State *L)
{
	if(lua_gettop(L) < 4) gen_error("objectAddPointer() requires at least 3 arguments");
	if(lua_gettop(L) > 7) gen_error("objectAddPointer() requires no more than 6 arguments");
	if(lua_type(L, 1) != LUA_TLIGHTUSERDATA) gen_error("objectAddPointer() requires an object as the first argument");
	if(lua_type(L, 2) != LUA_TSTRING) gen_error("objectAddPointer() requires a string as the second argument");
	if(lua_type(L, 3) != LUA_TSTRING) gen_error("objectAddPointer() requires a string as the third argument");
	if(lua_type(L, 4) != LUA_TSTRING) gen_error("objectAddPointer() requires a string as the fourth argument");
	bool init = false;
	bool input = false;
	const char *value = NULL;
	if(lua_gettop(L) >= 5)
	{
		if(lua_type(L, 5) != LUA_TBOOLEAN) gen_error("objectAddPointer() requires a boolean as the fifth argument (if present)");
		init = lua_toboolean(L, 5);
		if(init) input = true;
	}
	if(lua_gettop(L) >= 6)
	{
		if(lua_type(L, 6) != LUA_TBOOLEAN) gen_error("objectAddPointer() requires a boolean as the sixth argument (if present)");
		input = lua_toboolean(L, 6);
	}
	if(lua_gettop(L) == 7)
	{
		if(lua_type(L, 7) != LUA_TSTRING) gen_error("objectAddPointer() requires a string as the seventh argument (if present)");
		value = lua_tostring(L, 7);
	}

	if(init && !input && value == NULL)
		gen_error("Must specify value, if initializing, but not taking as input");

	object *o1 = (object *)lua_topointer(L, 1);
	if(o1 == NULL) gen_error("object could not be found");


	// create a member with type = type, and store it in o1
	member *m = calloc(1,sizeof(member));
	if(m == NULL)
		gen_error("Allocating member failed");
	
	m->name = strdup(lua_tostring(L, 3));
	m->type = member_type_pointer;
	m->type_name = strdup(lua_tostring(L, 2));
	m->destruct = strdup(lua_tostring(L, 4));
	m->init = init;
	m->init_input = input;
	if(value != NULL)
	{
		m->param_count = 1;
		m->init_params = calloc(1,sizeof(char *));
		if(m->init_params == NULL)
			gen_error("failed allocating init_params");
		m->init_params[0] = strdup(value);
	}

	if(!object_add_member(o1, m))
		gen_error("Failed adding member to object");


	lua_pushboolean(L, true);
	return 1;
}

// bool objectAddArrayObject(object, object, name)
int process_object_add_array_object(lua_State *L)
{
	if(lua_gettop(L) != 3) gen_error("objectAddArrayObject() requires exactly 3 arguments");
	if(lua_type(L, 1) != LUA_TLIGHTUSERDATA) gen_error("objectAddArrayObject() requires an object as the first argument");
	if(lua_type(L, 2) != LUA_TLIGHTUSERDATA) gen_error("objectAddArrayObject() requires an object as the second argument");
	if(lua_type(L, 3) != LUA_TSTRING) gen_error("objectAddArrayObject() requires a string as the third argument");

	object *o1 = (object *)lua_topointer(L, 1);
	if(o1 == NULL) gen_error("object could not be found");
	
	object *o2 = (object *)lua_topointer(L, 2);
	if(o2 == NULL) gen_error("object could not be found");

	// create a member and reference o2, and store it in o1
	member *m1 = calloc(1,sizeof(member));
	if(m1 == NULL)
		gen_error("Allocating member failed");
	
	m1->name = NULL;
	m1->type = member_type_object;
	m1->o = o2;
	
	member *m2 = calloc(1,sizeof(member));
	if(m2 == NULL)
		gen_error("Allocating member failed");
	
	m2->name = strdup(lua_tostring(L, 3));
	m2->type = member_type_array;
	m2->array_of = m1;
	
	if(!object_add_member(o1, m2))
		gen_error("Failed adding member to object");
	
	lua_pushboolean(L, true);
	return 1;
}

// bool objectAddArrayPointer(object, type, name, destruct)
int process_object_add_array_pointer(lua_State *L)
{
	if(lua_gettop(L) != 4) gen_error("objectAddArrayPointer() requires exactly 4 arguments");
	if(lua_type(L, 1) != LUA_TLIGHTUSERDATA) gen_error("objectAddArrayPointer() requires an object as the first argument");
	if(lua_type(L, 2) != LUA_TSTRING) gen_error("objectAddArrayPointer() requires a string as the second argument");
	if(lua_type(L, 3) != LUA_TSTRING) gen_error("objectAddArrayPointer() requires a string as the third argument");
	if(lua_type(L, 4) != LUA_TSTRING) gen_error("objectAddArrayPointer() requires a string as the fourth argument");

	object *o1 = (object *)lua_topointer(L, 1);
	if(o1 == NULL) gen_error("object could not be found");
	
	// create a member and reference type, and store it in o1
	member *m1 = calloc(1,sizeof(member));
	if(m1 == NULL)
		gen_error("Allocating member failed");
	
	m1->name = NULL;
	m1->type = member_type_pointer;
	m1->type_name = strdup(lua_tostring(L, 2));
	m1->destruct = strdup(lua_tostring(L, 4));
	
	member *m2 = calloc(1,sizeof(member));
	if(m2 == NULL)
		gen_error("Allocating member failed");
	
	m2->name = strdup(lua_tostring(L, 3));
	m2->type = member_type_array;
	m2->array_of = m1;
	
	if(!object_add_member(o1, m2))
		gen_error("Failed adding member to object");
	
	lua_pushboolean(L, true);
	return 1;
}

// bool objectAddArrayType(object, type, name, destruct)
int process_object_add_array_type(lua_State *L)
{
	if(lua_gettop(L) != 4) gen_error("objectAddArrayType() requires exactly 4 arguments");
	if(lua_type(L, 1) != LUA_TLIGHTUSERDATA) gen_error("objectAddArrayType() requires an object as the first argument");
	if(lua_type(L, 2) != LUA_TSTRING) gen_error("objectAddArrayType() requires a string as the second argument");
	if(lua_type(L, 3) != LUA_TSTRING) gen_error("objectAddArrayType() requires a string as the third argument");
	if(lua_type(L, 4) != LUA_TSTRING) gen_error("objectAddArrayType() requires a string as the fourth argument");

	object *o1 = (object *)lua_topointer(L, 1);
	if(o1 == NULL) gen_error("object could not be found");
	
	// create a member and reference type, and store it in o1
	member *m1 = calloc(1,sizeof(member));
	if(m1 == NULL)
		gen_error("Allocating member failed");
	
	m1->name = NULL;
	m1->type = member_type_type;
	m1->type_name = strdup(lua_tostring(L, 2));
	m1->destruct = strdup(lua_tostring(L, 4));
	
	member *m2 = calloc(1,sizeof(member));
	if(m2 == NULL)
		gen_error("Allocating member failed");
	
	m2->name = strdup(lua_tostring(L, 3));
	m2->type = member_type_array;
	m2->array_of = m1;
	
	if(!object_add_member(o1, m2))
		gen_error("Failed adding member to object");
	
	lua_pushboolean(L, true);
	return 1;
}

// bool moduleAddInclude(module, include)
int process_module_add_include(lua_State *L)
{
	if(lua_gettop(L) != 2) gen_error("moduleAddInclude() requires exactly 2 arguments");
	if(lua_type(L, 1) != LUA_TSTRING) gen_error("moduleAddInclude() requires a string as the first argument");
	if(lua_type(L, 2) != LUA_TSTRING) gen_error("moduleAddInclude() requires a string as the second argument");

	module *m = world_find_module(WORLD, lua_tostring(L, 1));
	if(m == NULL) gen_error("module not found, or could not be created");

	lua_pushboolean(L, module_add_include(m, lua_tostring(L, 2)));
	return 1;
}

// bool moduleAddDep(module, module)
int process_module_add_depend(lua_State *L)
{
	if(lua_gettop(L) != 2) gen_error("moduleAddDep() requires exactly 2 arguments");
	if(lua_type(L, 1) != LUA_TSTRING) gen_error("moduleAddDep() requires a string as the first argument");
	if(lua_type(L, 2) != LUA_TSTRING) gen_error("moduleAddDep() requires a string as the second argument");

	module *m = world_find_module(WORLD, lua_tostring(L, 1));
	if(m == NULL) gen_error("module not found, or could not be created");

	lua_pushboolean(L, module_add_depend(m, lua_tostring(L, 2)));
	return 1;
}

// function *f = objectAddFunctionType(object, return, name)
int process_object_add_function_type(lua_State *L)
{
	if(lua_gettop(L) != 3) gen_error("objectAddFunc() requires exactly 3 arguments");
	if(lua_type(L, 1) != LUA_TLIGHTUSERDATA) gen_error("objectAddFunc() requires an object as the first argument");
	if(lua_type(L, 2) != LUA_TSTRING) gen_error("objectAddFunc() requires a string as the second argument");
	if(lua_type(L, 3) != LUA_TSTRING) gen_error("objectAddFunc() requires a string as the third argument");

	object *o = (object *)lua_topointer(L, 1);
	if(o == NULL) gen_error("object could not be found");

	function *func = object_add_function(o, lua_tostring(L, 3));
	if(func == NULL)
		gen_error("failed to generate function, or it already exists");

	func->returns = malloc(sizeof(member));
	if(func->returns == NULL)
		gen_error("failed allocating returns");
	
	func->returns->type = member_type_type;
	func->returns->type_name = strdup(lua_tostring(L, 2));
	func->o = o;

	lua_pushlightuserdata(L, func);
	return 1;
}

// function *f = objectAddFunctionPointer(object, return, name)
int process_object_add_function_pointer(lua_State *L)
{
	if(lua_gettop(L) != 3) gen_error("objectAddFunc() requires exactly 3 arguments");
	if(lua_type(L, 1) != LUA_TLIGHTUSERDATA) gen_error("objectAddFunc() requires an object as the first argument");
	if(lua_type(L, 2) != LUA_TSTRING) gen_error("objectAddFunc() requires a string as the second argument");
	if(lua_type(L, 3) != LUA_TSTRING) gen_error("objectAddFunc() requires a string as the third argument");

	object *o = (object *)lua_topointer(L, 1);
	if(o == NULL) gen_error("object could not be found");

	function *func = object_add_function(o, lua_tostring(L, 3));
	if(func == NULL)
		gen_error("failed to generate function, or it already exists");

	func->returns = malloc(sizeof(member));
	if(func->returns == NULL)
		gen_error("failed allocating returns");
	
	func->returns->type = member_type_pointer;
	func->returns->type_name = strdup(lua_tostring(L, 2));
	func->o = o;
	
	lua_pushlightuserdata(L, func);
	return 1;
}

// function *f = objectAddFunctionObject(object, return, name)
int process_object_add_function_object(lua_State *L)
{
	if(lua_gettop(L) != 3) gen_error("objectAddFunc() requires exactly 3 arguments");
	if(lua_type(L, 1) != LUA_TLIGHTUSERDATA) gen_error("objectAddFunc() requires an object as the first argument");
	if(lua_type(L, 2) != LUA_TLIGHTUSERDATA) gen_error("objectAddFunc() requires a string as the second argument");
	if(lua_type(L, 3) != LUA_TSTRING) gen_error("objectAddFunc() requires a string as the third argument");

	object *o = (object *)lua_topointer(L, 1);
	if(o == NULL) gen_error("object could not be found");

	function *func = object_add_function(o, lua_tostring(L, 3));
	if(func == NULL)
		gen_error("failed to generate function, or it already exists");

	func->returns = malloc(sizeof(member));
	if(func->returns == NULL)
		gen_error("failed allocating returns");
	
	func->returns->type = member_type_object;
	func->returns->o = (object *)lua_topointer(L, 2);
	func->o = o;

	lua_pushlightuserdata(L, func);
	return 1;
}

// bool functionAddParamType(function, type, name)
int process_function_add_param_type(lua_State *L)
{
	if(lua_gettop(L) != 3) gen_error("fuctionAddParam() requires exactly 3 arguments");
	if(lua_type(L, 1) != LUA_TLIGHTUSERDATA) gen_error("functionAddParam() requires an object as the first argument");
	if(lua_type(L, 2) != LUA_TSTRING) gen_error("functionAddParam() requires a string as the second argument");
	if(lua_type(L, 3) != LUA_TSTRING) gen_error("functionAddParam() requires a string as the third argument");

	function *f = (function *)lua_topointer(L, 1);
	if(f == NULL) gen_error("function could not be found");

	member *m = malloc(sizeof(member));
	if(m == NULL)
		gen_error("failed allocating member");
	
	m->type = member_type_type;
	m->type_name = strdup(lua_tostring(L, 2));
	m->name = strdup(lua_tostring(L, 3));
	
	f->param_count++;
	f->params = realloc(f->params, f->param_count * sizeof(member *));
	if(f->params == NULL)
		gen_error("realloc function parameters");
	
	f->params[f->param_count-1] = m;

	lua_pushboolean(L, true);
	return 1;
}

// bool functionAddParamPointer(function, type, name)
int process_function_add_param_pointer(lua_State *L)
{
	if(lua_gettop(L) != 3) gen_error("fuctionAddParam() requires exactly 3 arguments");
	if(lua_type(L, 1) != LUA_TLIGHTUSERDATA) gen_error("functionAddParam() requires an object as the first argument");
	if(lua_type(L, 2) != LUA_TSTRING) gen_error("functionAddParam() requires a string as the second argument");
	if(lua_type(L, 3) != LUA_TSTRING) gen_error("functionAddParam() requires a string as the third argument");

	function *f = (function *)lua_topointer(L, 1);
	if(f == NULL) gen_error("function could not be found");

	member *m = malloc(sizeof(member));
	if(m == NULL)
		gen_error("failed allocating member");
	
	m->type = member_type_pointer;
	m->type_name = strdup(lua_tostring(L, 2));
	m->name = strdup(lua_tostring(L, 3));

	f->param_count++;
	f->params = realloc(f->params, f->param_count * sizeof(member *));
	if(f->params == NULL)
		gen_error("realloc function parameters");
	
	f->params[f->param_count-1] = m;

	lua_pushboolean(L, true);
	return 1;
}

// bool functionAddParamObject(function, object, name)
int process_function_add_param_object(lua_State *L)
{
	if(lua_gettop(L) != 3) gen_error("fuctionAddParam() requires exactly 3 arguments");
	if(lua_type(L, 1) != LUA_TLIGHTUSERDATA) gen_error("functionAddParam() requires an object as the first argument");
	if(lua_type(L, 2) != LUA_TLIGHTUSERDATA) gen_error("functionAddParam() requires an object as the second argument");
	if(lua_type(L, 3) != LUA_TSTRING) gen_error("functionAddParam() requires a string as the third argument");

	function *f = (function *)lua_topointer(L, 1);
	if(f == NULL) gen_error("function could not be found");

	member *m = malloc(sizeof(member));
	if(m == NULL)
		gen_error("failed allocating member");
	
	m->type = member_type_object;
	m->o = (object *)lua_topointer(L, 2);
	m->name = strdup(lua_tostring(L, 3));

	f->param_count++;
	f->params = realloc(f->params, f->param_count * sizeof(member *));
	if(f->params == NULL)
		gen_error("realloc function parameters");
	
	f->params[f->param_count-1] = m;

	lua_pushboolean(L, true);
	return 1;
}

int luaopen_process(lua_State *L)
{
	lua_register(L, "moduleCreate", process_module_create);
	lua_register(L, "objectCreate", process_object_create);
	lua_register(L, "moduleAddInclude", process_module_add_include);
	lua_register(L, "moduleAddDep", process_module_add_depend);
	lua_register(L, "objectAddObject", process_object_add_object);
	lua_register(L, "objectAddType", process_object_add_type);
	lua_register(L, "objectAddPointer", process_object_add_pointer);
	lua_register(L, "objectAddArrayObject", process_object_add_array_object);
	lua_register(L, "objectAddArrayPointer", process_object_add_array_pointer);
	lua_register(L, "objectAddArrayType", process_object_add_array_type);
	lua_register(L, "objectAddFunctionType", process_object_add_function_type);
	lua_register(L, "objectAddFunctionPointer", process_object_add_function_pointer);
	lua_register(L, "objectAddFunctionObject", process_object_add_function_object);
	lua_register(L, "functionAddParamType", process_function_add_param_type);
	lua_register(L, "functionAddParamPointer", process_function_add_param_pointer);
	lua_register(L, "functionAddParamObject", process_function_add_param_object);
	return 1;
}
