#include <generator.h>

#define process_error(S,E) process_error_x(S,E,__FUNCTION__,__LINE__)


// object *o = objectCreate(module, name, lock, ref)
int process_object_create(lua_State *L)
{
	if(lua_gettop(L) != 4) gen_error("objectCreate() requires exactly 4 arguments");
	if(lua_type(L, 1) != LUA_TSTRING) gen_error("objectCreate() requires as a string as the first argument");
	if(lua_type(L, 2) != LUA_TSTRING) gen_error("objectCreate() requires as a string as the second argument");
	if(lua_type(L, 3) != LUA_TBOOLEAN) gen_error("objectCreate() requires as a boolean as the third argument");
	if(lua_type(L, 4) != LUA_TBOOLEAN) gen_error("objectCreate() requires as a boolean as the fourth argument");

	module *m = world_find_module(WORLD, lua_tostring(L, 1));
	if(m == NULL) gen_error("module not found, or could not be created");
	
	object *o = world_create_object(WORLD, lua_tostring(L, 2));
	if(o == NULL) gen_error("object already exists, or could not be created");
	module_add_object(m, o);

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

int luaopen_process(lua_State *L)
{
	lua_register(L, "objectCreate", process_object_create);
	lua_register(L, "moduleAddInclude", process_module_add_include);
	lua_register(L, "moduleAddDep", process_module_add_depend);
	lua_register(L, "objectAddObject", process_object_add_object);
	lua_register(L, "objectAddType", process_object_add_type);
	lua_register(L, "objectAddPointer", process_object_add_pointer);
	lua_register(L, "objectAddArrayObject", process_object_add_array_object);
	lua_register(L, "objectAddArrayPointer", process_object_add_array_pointer);
	return 1;
}
