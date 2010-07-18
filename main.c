#include <generator.h>

world *WORLD = NULL;

void gen_error_r(const char *mesg, const char *func, const char *file, int line)
{
	fprintf(stderr, "ERROR: %s:%i %s(...): %s\n", file, line, func, mesg);
	exit(EXIT_FAILURE);
}

static void print_usage(const char *name)
{
	fprintf(stderr, "Usage: %s object.def\n", name);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	if(argc <= 1)
	{
		print_usage(argv[0]);
	}
	
	WORLD = world_create();
	
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
	
	if(!generate(WORLD))
		gen_error("generating world");
	
	return EXIT_SUCCESS;
}
