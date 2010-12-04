#include <generator.h>

using namespace generator;
using namespace std;

World *generator::WORLD = NULL;

void gen_error_r(const char *mesg, const char *func, const char *file, int line)
{
	fprintf(stderr, "ERROR: %s:%i %s(...): %s\n", file, line, func, mesg);
	exit(EXIT_FAILURE);
}

static void print_usage(const char *name)
{
	fprintf(stderr, "Usage: %s objectDefinition.lua\n", name);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	if(argc <= 1)
	{
		print_usage(argv[0]);
	}
	
	WORLD = new World();
	
	lua_State *state = luaL_newstate();
	if(state == NULL) gen_error("creating lua state");
	
	luaL_openlibs(state);
	luaopen_process(state);

	for(int i = 1; i < argc; i++)
	{
		int res = luaL_dofile(state, argv[i]);
	
		if(res != 0)
		{
			gen_error(lua_tostring(state, -1));
		}
	}
	
	lua_close(state);

	// Create a "contrived" example
#if 0
	{
		std::string *mName = new std::string("scott");
		std::string *mPath = new std::string("server");
		std::string *fPrefix = new std::string("us_");
		std::string *funcPrefix = new std::string("");
		Module *m = new Module(mName, mPath, fPrefix, funcPrefix);
		WORLD->moduleAdd(mName, m);
		
		std::string *tName = new std::string("test");
		Type *t = new Type(m, tName);
		m->objectAdd(tName, t);
	}
#endif	
		
	if(!WORLD->generate())
		gen_error("generating world");

	return EXIT_SUCCESS;
}
