ml = moduleCreate('lua', 'lib', 'us_', 'us_')

ml:addInclude('lua.h')
ml:addInclude('lauxlib.h')

lua = ml:newType('lua')

lua_State = newPointer(newSystemType('lua_State'),'free')
lua:memberAdd(lua_State,'state',false,false)

f_do_string = lua:functionCreate('do_string',bool)
f_do_string:paramAdd(pchar,'script')

f_do_file = lua:functionCreate('do_file',bool)
f_do_file:paramAdd(pchar,'fname')

lua_init_fp = ml:newFunctionPointer('lualib_init',bool)
lua_init_fp:paramAdd(lua_State,'state')

lua_open_lib = lua:functionCreate('open_lib',bool)
lua_open_lib:paramAdd(lua_init_fp, 'lib')

f_load_file = lua:functionCreate('load_file',bool)
f_load_file:paramAdd(pchar,'fname')

lua_run = lua:functionCreate('run',bool)
