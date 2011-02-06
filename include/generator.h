#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>
#include <list>
#include <ostream>
#include <fstream>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

extern "C" {
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
};

void gen_error_r(const char *, const char *, const char *, int) __attribute__((noreturn));
#define gen_error(M) gen_error_r(M, __FUNCTION__, __FILE__, __LINE__)

#define print_to_file(F,...) \
	do { \
		int res = fprintf(F, __VA_ARGS__); \
		if(res < 0) \
			gen_error_r(strerror(errno), __FUNCTION__, __FILE__, __LINE__); \
	} while(0)
		
//		fprintf(F, "/* %s:%i %s */", __FILE__, __LINE__, __FUNCTION__); \

#define LUA_SET_TABLE_TYPE(L,T) \
	lua_pushstring(L, #T); \
	lua_pushstring(L, "yes"); \
	lua_settable(L, -3); 

#define LUA_ADD_TABLE_FUNC(L,N,F) \
	lua_pushstring(L, N); \
	lua_pushcfunction(L, F); \
	lua_settable(L, -3);

#define CREATE_TABLE(L,D) \
	lua_newtable(L); \
	lua_pushstring(L, "data"); \
	lua_pushlightuserdata(L, D); \
	lua_settable(L, -3);

#define SET_TABLE_TYPE(L,T) \
	lua_pushstring(L, #T); \
	lua_pushstring(L, "yes"); \
	lua_settable(L, -3); 

#define LUA_ADD_TABLE_TABLE(L,N,D) \
	lua_pushstring(L, N); \
	CREATE_TABLE(L,D) \
	D->lua_table(L); \
	lua_settable(L, -3);

extern "C" {
	int li_type_memberAdd(lua_State *L);
	int li_type_functionCreate(lua_State *L);
	int li_function_paramAdd(lua_State *L);
	int li_module_type_create(lua_State *L);
	int li_module_include_add(lua_State *L);
};


namespace generator {
	class Element;
	class Type;
	class Object;
	class Function;
	class Element;
	class SystemType;
	class Pointer;
	class ObjectElement;
	class Array;
	class Module;
	class World;

	class Element {
		private:
		public:
			explicit Element() {};
			virtual bool genStruct(std::ostream& header, std::string name) = 0;
			virtual bool genType(std::ostream& header) = 0;
			virtual bool genDestruct(std::ostream& logic, std::string *name) = 0;
			virtual bool genFunctionDefs(std::ostream& header, std::string *name, Module *Mod, Type *t) = 0;
			virtual bool genLogic(std::ostream& logic, std::string *name, Module *Mod, Type *t) = 0;
			virtual std::string initValue() = 0;
			virtual bool genTemplate(std::ostream& templ) { return true; };
			virtual bool needs_connecting() { return false; };
			virtual bool needs_disconnecting() { return false; };
			virtual bool print_connect(std::ostream& logic) { return true; };
			virtual bool print_disconnect(std::ostream& logic) { return true; };
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Element) }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};
	
	template <class T>
	class Member {
		private:
			T* type;
			bool init;
			bool input;
		public:
			Member<T>(T* type, bool init = false, bool input = false) : type(type), init(init), input(input) {}
			T* get() { return this->type; }
			std::string Name() { return this->name; };
			bool isInit() { return this->init; };
			bool isInput() { return (this->init && this->input); }
			// functions for generator output
			virtual bool genStruct(std::ostream& of, std::string name)
			{
				return type->genStruct(of, name);
			}
			virtual bool genDestruct(std::ostream& of, std::string *name)
			{
				return type->genDestruct(of, name);
			}
			virtual bool genFunctionDefs(std::ostream& of, std::string *name, Module *Mod, Type *t)
			{
				return type->genFunctionDefs(of, name, Mod, t);
			}
			virtual bool genLogic(std::ostream& of, std::string *name, Module *Mod, Type *t)
			{
				return type->genLogic(of, name, Mod, t);
			}
	};

	class Type : public Element {
		private:
			typedef Element super;
			Module *Mod;
			std::string *Name;
			bool nolock;
			bool noref;
			std::map<std::string *, Member<Element> *> members;
			std::map<std::string *, Member<Element> *>::iterator memberIterBegin();
			std::map<std::string *, Member<Element> *>::iterator memberIterEnd();
			std::map<std::string *, Function *> functions;
			std::map<std::string *, Function *>::iterator functionIterBegin();
			std::map<std::string *, Function *>::iterator functionIterEnd();
			bool create_def_print(std::ostream& f);
			bool func_def_print(std::ostream& f, std::string fname);
			bool create_func_print(std::ostream& f);		
			bool destroy_func_print(std::ostream& f);
			bool ref_func_print(std::ostream& f);
			bool unref_func_print(std::ostream& f);
			bool destroy_lock_code_print(std::ostream& f);
		public:
			explicit Type(Module *mod, std::string *objName) : Name(objName), Mod(mod), nolock(false), noref(false), Element() {};
			bool memberAdd(Member<Element> *m, std::string *name);
			Element *memberFind(std::string *);
			bool functionAdd(Function *);
			Function *functionFind(std::string *);
			std::string *name() { return this->Name; };
			virtual bool genType(std::ostream& header);
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genSetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t);
			virtual bool genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t);
			virtual bool genFunctionDefs(std::ostream& header, std::string *, Module *Mod, Type *t);
			virtual bool genLogic(std::ostream& logic, std::string *name, Module *Mod, Type *t);
			virtual bool genDestruct(std::ostream& logic, std::string *name);
			virtual std::string initValue() { return "NULL"; };
			bool genStruct(std::ostream& header);
			bool genFunctionDefs(std::ostream& header, Module *Mod);
			bool genLogic(std::ostream& logic);
			bool genTemplate(std::ostream& templ);
			bool lock_code_print(std::ostream& f, bool null);
			bool unlock_code_print(std::ostream& f);
			bool haveFunctions();
			virtual bool print_disconnect(std::ostream& f);
			virtual bool needs_disconnecting() { return true; };
			virtual bool needs_connecting() { return (!nolock && !noref); };
			virtual bool needs_referencing() { return (!nolock && !noref); };
			virtual bool print_connect(std::ostream& f);
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Type)
						LUA_ADD_TABLE_FUNC(L, "memberAdd", li_type_memberAdd);
						LUA_ADD_TABLE_FUNC(L, "functionCreate", li_type_functionCreate);
						super::lua_table_r(L); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};
	
	class Function {
		private:
			std::map<std::string *, Member<Element> *> parameters;
			std::map<std::string *, Member<Element> *>::iterator paramsIterBegin();
			std::map<std::string *, Member<Element> *>::iterator paramsIterEnd();
			std::string *Name;
			Element *ReturnType;
			bool genFunctionDef(std::ostream& header, Module *Mod, Type *t, bool tpl, bool type);
			bool genFunctionCall(std::ostream& header, Module *Mod, Type *t, bool tpl, bool type);
		public:
			explicit Function(std::string *Name, Element *rt) : Name(Name), ReturnType(rt) {};
			bool paramAdd(std::string *, Member<Element> *mem);
			std::string *name() { return this->Name; };
			bool genFunctionDefs(std::ostream& header, Module *Mod, Type *t);
			bool genLogic(std::ostream& logic, Module *Mod, Type *t);
			bool genTemplate(std::ostream& templ, Module *Mod, Type *t);
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Function)
						LUA_ADD_TABLE_FUNC(L, "paramAdd", li_function_paramAdd); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};
	
	class SystemType : public Element {
		private:
			typedef Element super;
			std::string *TypeName;
		public:
			explicit SystemType(std::string *TypeName) : TypeName(TypeName), Element() {};
			virtual bool genType(std::ostream& header);
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genSetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t);
			virtual bool genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t);
			virtual bool genFunctionDefs(std::ostream& header, std::string *, Module *Mod, Type *t);
			virtual bool genLogic(std::ostream& logic, std::string *name, Module *Mod, Type *t);
			virtual bool genDestruct(std::ostream& logic, std::string *name);
			virtual std::string initValue() { return "0"; };
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,SystemType)
						super::lua_table_r(L); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};
	
	class Pointer : public Element {
		private:
			typedef Element super;
			Element *To;
			std::string *Destroy;
		public:
			explicit Pointer(Element *to, std::string *destroy) : To(to), Destroy(destroy), Element() { if(this->Destroy == NULL) { std::cerr << "Error Destroy is NULL" << std::endl; } };
			virtual bool genType(std::ostream& header);
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genSetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t);
			virtual bool genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t);
			virtual bool genFunctionDefs(std::ostream& header, std::string *, Module *Mod, Type *t);
			virtual bool genLogic(std::ostream& logic, std::string *name, Module *Mod, Type *t);
			virtual bool genDestruct(std::ostream& logic, std::string *name);
			virtual std::string initValue() { return "NULL"; };
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Pointer)
						super::lua_table_r(L); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};
	
	class Array : public Element {
		private:
			typedef Element super;
			Element *Of;
		public:
			explicit Array(Element *of) : Of(of), Element() {};
			virtual bool genType(std::ostream& header);
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genAddFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t);
			virtual bool genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t);
			virtual bool genDelFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t);
			virtual bool genSizeFunctionDef(std::ostream& header, std::string *name, Module *Mod, Type *t);
			virtual bool genFunctionDefs(std::ostream& header, std::string *, Module *Mod, Type *t);
			virtual bool genLogic(std::ostream& logic, std::string *name, Module *Mod, Type *t);
			virtual bool genDestruct(std::ostream& logic, std::string *name);
			virtual std::string initValue() { return "NULL"; };
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Array)
						super::lua_table_r(L); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};
	
	class Module {
		private:
			std::string *Name;
			std::string *Path;
			std::string *FilePrefix;
			std::string *FuncPrefix;
			std::list<std::string> headers;
			std::list<Module *> depends;
			std::map<std::string *, Type *> *Objects;
			std::map<std::string *, Type *>::iterator objectsIterBegin();
			std::map<std::string *, Type *>::iterator objectsIterEnd();
			std::list<std::string *> includes;
		public:
			explicit Module(std::string *name, std::string *Path, std::string *FilePrefix, std::string *FuncPrefix) :
			Name(name), Path(Path), FilePrefix(FilePrefix), FuncPrefix(FuncPrefix)
			{ this->Objects = new std::map<std::string *, Type *>(); };
			bool objectAdd(std::string *objName, Type *object);
			Type *objectFind(std::string *objName);
			bool generate(std::string *name);
			std::string *funcPrefix() { return this->FuncPrefix; };
			std::string *filePrefix() { return this->FilePrefix; };
			std::string *name() { return this->Name; };
			bool includeAdd(std::string *inc) { this->includes.push_back(inc); return true; };
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Module)
							LUA_ADD_TABLE_FUNC(L,"newType",li_module_type_create)
							LUA_ADD_TABLE_FUNC(L,"addInclude",li_module_include_add)
						}
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};
	
	class World {
		private:
			std::map<std::string *, Module *> *Modules;
			std::map<std::string *, Module *>::iterator modulesIterBegin();
			std::map<std::string *, Module *>::iterator modulesIterEnd();
		public:
			explicit World() { Modules = new std::map<std::string *, Module *>; };
			bool moduleAdd(std::string *modName, Module *mod);
			Module *moduleFind(std::string *modName);
			bool objectAdd(std::string *modName, std::string *objName, Type *object);
			bool objectFind(std::string *objName);
			bool generate();			
	};
	
	extern World *WORLD;
	void luaopen_process(lua_State *state);
}

std::ostream& operator<<(std::ostream& os, std::string *str);
