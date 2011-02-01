#include <generator.h>

#define CHECK_COUNT(F,C)		if(lua_gettop(L) != C) gen_error(#F "() requires exactly " #C " arguments");
#define CHECK_COUNT_MIN(F,C)		if(lua_gettop(L) < C) gen_error(#F "() requires at least  " #C " arguments");
#define CHECK_ARGUMENT(F,N,T)		if(!lua_is##T(L, N)) gen_error(#F "() requires a " #T " as argument " #N );
#define CHECK_ARGUMENT_IF_GIVEN(F,N,T)	if(lua_gettop(L) >= N && !lua_is##T(L, N)) gen_error(#F "() requires a " #T " as argument " #N " if present");
#define GET_ARGUMENT_IF_GIVEN(N,T,D)	((lua_gettop(L) >= N && lua_is##T(L, N)) ? lua_to##T(L, N) : D)
#define GET_ARGUMENT_IF_GIVEN_STR(N)	((lua_gettop(L) >= N && lua_isstring(L, N)) ? new std::string(lua_tostring(L, N)) : NULL)

#define CHECK_ARGUMENT_TYPE(F,N,T,V) CHECK_ARGUMENT(F,N,table) \
	lua_pushstring(L, #T); \
	lua_gettable(L, N); \
	if(!lua_isstring(L, -1) || strcmp(lua_tostring(L, -1), "yes") != 0) gen_error("" #F "() requires an object of type " #T " as argument " #N); \
	lua_pop(L, 1); \
	lua_pushstring(L, "data"); \
	lua_gettable(L, N); \
	if(!lua_islightuserdata(L, -1)) gen_error("" #F "() requires data of argument " #N " to contain something of type " #T ""); \
	T * V = (T *)lua_topointer(L, -1); \
	lua_pop(L, 1);

using namespace generator;

// bool memberAdd(Type, Member)
int li_type_memberAdd(lua_State *L)
{
	CHECK_COUNT("memberAdd",2)	
	CHECK_ARGUMENT_TYPE("memberAdd",1,Type,t)
	CHECK_ARGUMENT_TYPE("memberAdd",2,Member,m)

	lua_pushboolean(L, t->memberAdd(m));
	return 1;
}

// bool paramAdd(Function, Member)
int li_function_paramAdd(lua_State *L)
{
	CHECK_COUNT("paramAdd",2)	
	CHECK_ARGUMENT_TYPE("paramAdd",1,Function,f)
	CHECK_ARGUMENT_TYPE("paramAdd",2,Member,m)

	lua_pushboolean(L, f->paramAdd(m));
	return 1;
}

// function * functionCreate(Type, name, Member)
int li_type_functionCreate(lua_State *L)
{
	CHECK_COUNT("functionCreate",3)	
	CHECK_ARGUMENT_TYPE("functionCreate",1,Type,t)
	CHECK_ARGUMENT_TYPE("functionCreate",3,Member,rt)

	Function *f = new Function(GET_ARGUMENT_IF_GIVEN_STR(2), rt);

	t->functionAdd(f);

	CREATE_TABLE(L, f);
	f->lua_table(L);
	
	// return the table
	return 1;
}

// RType * = externTypeCreate(tname, name, init, input, value)
int li_extern_type_create(lua_State *L)
{
	CHECK_COUNT_MIN("externTypeCreate", 2)
	CHECK_ARGUMENT("externTypeCreate", 1, string)
	CHECK_ARGUMENT("externTypeCreate", 2, string)
	CHECK_ARGUMENT_IF_GIVEN("externTypeCreate", 3, boolean)
	CHECK_ARGUMENT_IF_GIVEN("externTypeCreate", 4, boolean)
	CHECK_ARGUMENT_IF_GIVEN("externTypeCreate", 5, string)

	RType *rt = new RType(	new std::string(lua_tostring(L, 1)),
				GET_ARGUMENT_IF_GIVEN(3,boolean,false),
				GET_ARGUMENT_IF_GIVEN(4,boolean,GET_ARGUMENT_IF_GIVEN(3,boolean,false)),
				new std::string(lua_tostring(L, 2)),
				GET_ARGUMENT_IF_GIVEN_STR(5) );
	if(rt == NULL) gen_error("external type could not be created");
	
	CREATE_TABLE(L, rt);
	rt->lua_table(L);
	
	// return the table
	return 1;
}

// Pointer * = pointerCreate(Member, name, destruct, init, input, value)
int li_pointer_create(lua_State *L)
{
	CHECK_COUNT_MIN("pointerCreate", 3)
	CHECK_ARGUMENT_TYPE("pointerCreate", 1, Member, m)
	CHECK_ARGUMENT("pointerCreate", 2, string)
	CHECK_ARGUMENT("pointerCreate", 3, string)
	CHECK_ARGUMENT_IF_GIVEN("pointerCreate", 4, boolean)
	CHECK_ARGUMENT_IF_GIVEN("pointerCreate", 5, boolean)
	CHECK_ARGUMENT_IF_GIVEN("pointerCreate", 6, string)
	
	Pointer *p = new Pointer(m,
				GET_ARGUMENT_IF_GIVEN_STR(2),
				GET_ARGUMENT_IF_GIVEN_STR(3),
				GET_ARGUMENT_IF_GIVEN(4,boolean,false),
				GET_ARGUMENT_IF_GIVEN(5,boolean,GET_ARGUMENT_IF_GIVEN(4,boolean,false)),
				GET_ARGUMENT_IF_GIVEN_STR(6));
	if(p == NULL) gen_error("pointer could not be created");
	
	CREATE_TABLE(L, p);
	p->lua_table(L);
	
	// return the table
	return 1;
}

// ObjectMember * = objectCreate(Type, name, init, input, value)
int li_object_create(lua_State *L)
{
	CHECK_COUNT("objectCreate", 2)
	CHECK_ARGUMENT_TYPE("objectCreate", 1, Type, t)
	CHECK_ARGUMENT("objectCreate", 2, string)
	CHECK_ARGUMENT_IF_GIVEN("objectCreate", 3, boolean)
	CHECK_ARGUMENT_IF_GIVEN("objectCreate", 4, boolean)
	CHECK_ARGUMENT_IF_GIVEN("objectCreate", 5, string)
	
	ObjectMember *om = new ObjectMember(t,
				GET_ARGUMENT_IF_GIVEN(3,boolean,false),
				GET_ARGUMENT_IF_GIVEN(4,boolean,GET_ARGUMENT_IF_GIVEN(3,boolean,false)),
				GET_ARGUMENT_IF_GIVEN_STR(2),
				GET_ARGUMENT_IF_GIVEN_STR(5));
	if(om == NULL) gen_error("object could not be created");
	
	CREATE_TABLE(L, om);
	om->lua_table(L);
	
	// return the table
	return 1;
}

// Array * = arrayCreate(Member, init, input, name)
int li_array_create(lua_State *L)
{
	CHECK_COUNT("arrayCreate", 2)
	CHECK_ARGUMENT_TYPE("arrayCreate", 1, Member, m)
	CHECK_ARGUMENT("arrayCreate", 2, string)
	
	Array *a = new Array(m, GET_ARGUMENT_IF_GIVEN_STR(2));
	if(a == NULL) gen_error("array could not be created");
	
	CREATE_TABLE(L, a);
	a->lua_table(L);
	
	// return the table
	return 1;
}

// type *t = typeCreate(Module, name)
int li_type_create(lua_State *L)
{
	CHECK_COUNT("typeCreate",2)
	CHECK_ARGUMENT("typeCreate",2,string)
	
	// check we have a module, then convert it to a Module
	CHECK_ARGUMENT_TYPE("typeCreate",1,Module,m)

	// now create the Type
	
	Type *t = new Type(m, new std::string(luaL_checkstring(L, 2)));
	if(t == NULL) gen_error("type could not be created");
	
	// now add this type to the module
	m->objectAdd(new std::string(luaL_checkstring(L, 2)), t);
	
	CREATE_TABLE(L, t);
	t->lua_table(L);
		
	// now we just return the table :)
	return 1;
}

// module *m = moduleCreate(name, path, file_prefix, function_prefix)
int li_module_create(lua_State *L)
{
	if(lua_gettop(L) != 4) gen_error("moduleCreate() requires exactly 4 arguments");

	Module *m = new Module(	new std::string(luaL_checkstring(L, 1)),
				new std::string(luaL_checkstring(L, 2)),
				new std::string(luaL_checkstring(L, 3)),
				new std::string(luaL_checkstring(L, 4)));
	if(m == NULL) gen_error("module could not be created");

	// register the module in the world
	WORLD->moduleAdd(new std::string(luaL_checkstring(L, 1)), m);
	
	CREATE_TABLE(L, m);
	m->lua_table(L);

	// now we just return the table :)
	return 1;
}

void generator::luaopen_process(lua_State *state)
{
	lua_register(state, "moduleCreate", li_module_create);
	lua_register(state, "typeCreate", li_type_create);
	
	// RType * = externTypeCreate(tname, name, init, input)
	lua_register(state, "externTypeCreate", li_extern_type_create);
	// Pointer * = pointerCreate(Member, name, init, input)
	lua_register(state, "pointerCreate", li_pointer_create);
	// ObjectMember * = objectCreate(Type, name, init, input)
	lua_register(state, "objectCreate", li_object_create);
	// Array * = arrayCreate(Member, name, init, input)
	lua_register(state, "arrayCreate", li_array_create);
}
