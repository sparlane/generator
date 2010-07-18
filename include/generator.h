#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define gen_error(M) gen_error_r(M, __FUNCTION__, __FILE__, __LINE__)

typedef struct world_s world;
typedef struct module_s module;
typedef struct object_s object;
typedef struct member_s member;
typedef struct function_s function;
typedef struct operation_s operation;

typedef enum { member_type_none , member_type_object , member_type_type, member_type_pointer, member_type_array } member_type;

struct world_s
{
	module **modules;
	size_t module_count;
	object **objects;
	size_t object_count;
};

struct module_s
{
	world *w;
	char *name;
	char **includes;
	size_t include_count;
	object **objects;
	size_t object_count;
};

struct object_s
{
	world *w;
	module *m;	
	
	char *name;
	bool locked;
	bool refcount;
	member **members;
	size_t member_count;
};

struct member_s
{
	char *name;
	member_type type;
	object *o;
	char *type_name;
	size_t pointer_depth;
	member *array_of;
	bool init;
	bool init_input;
	char **init_params;
	size_t param_count;
	char *destruct;
};

struct function_s
{
	object *o;
	member **params;
	size_t param_count;
	operation **operations;
	size_t operation_count;	
};

struct operation_s
{
	// dont know yet
	// operation type
	// other things ??	
	char *name;
};


// main.c
extern world *WORLD;
void gen_error_r(const char *, const char *, const char *, int) __attribute__((noreturn));


// process.c
int luaopen_process(lua_State *L);

// generate.c
bool generate(world *w);

// world.c
world *world_create(void);
module *world_find_module(world *w, const char *name);
object *world_create_object(world *w, const char *name);
object *world_find_object(world *w, const char *name);

// module.c
module *module_create(const char *name);
bool module_add_object(module *m, object *o);
bool module_add_include(module *m, const char *inc);

// object.c
object *object_create(const char *name);
bool object_add_member(object *o, member *m);

