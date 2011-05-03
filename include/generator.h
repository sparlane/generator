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
	int li_module_function_pointer_create(lua_State *L);
	int li_module_queue_create(lua_State *L);
	int li_module_bst_create(lua_State *L);
	int li_module_include_add(lua_State *L);
	int li_fp_memberAdd(lua_State *L);
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
			virtual bool genFunctionDefs(std::ostream& header, std::string *name, Module *Mod, Object *t) = 0;
			virtual bool genLogic(std::ostream& logic, std::string *name, Module *Mod, Object *t) = 0;
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
			std::string *initString;
		public:
			Member<T>(T* type, bool init = false, bool input = false, std::string *iString = NULL) : type(type), init(init), input(input), initString(iString) {}
			T* get() { return this->type; }
			std::string Name() { return this->name; };
			bool isInit() { return this->init; };
			bool isInput() { return (this->init && this->input); }
			std::string *getInit() { return this->initString; };
			// functions for generator output
			virtual bool genStruct(std::ostream& of, std::string name)
			{
				return type->genStruct(of, name);
			}
			virtual bool genDestruct(std::ostream& of, std::string *name)
			{
				return type->genDestruct(of, name);
			}
			virtual bool genFunctionDefs(std::ostream& of, std::string *name, Module *Mod, Object *t)
			{
				return type->genFunctionDefs(of, name, Mod, t);
			}
			virtual bool genLogic(std::ostream& of, std::string *name, Module *Mod, Object *t)
			{
				return type->genLogic(of, name, Mod, t);
			}
	};
	
	class Object : public Element {
		private:
			typedef Element super;
			Module *Mod;
			std::string *Name;
			virtual bool create_def_print(std::ostream& f) = 0;
			virtual bool func_def_print(std::ostream& f, std::string fname) = 0;
			virtual bool create_func_print(std::ostream& f) = 0;
			virtual bool destroy_func_print(std::ostream& f) = 0;
			virtual bool ref_func_print(std::ostream& f) = 0;
			virtual bool unref_func_print(std::ostream& f) = 0;
		protected:
			Module *module() { return this->Mod; };
			bool nolock;
			bool noref;
			virtual bool destroy_lock_code_print(std::ostream& f);
		public:
			explicit Object(Module *mod, std::string *objName, bool nl, bool nr) : Element(), Mod(mod), Name(objName), nolock(nl), noref(nr) {};
			virtual bool genStruct(std::ostream& header) = 0; // this is the version that module calls
			virtual bool genStruct(std::ostream& header, std::string name) = 0; // this is the version that other genStructs call
			virtual bool genType(std::ostream& header) = 0;
			virtual bool genTypeDef(std::ostream& header) = 0;
			virtual bool genDestruct(std::ostream& logic, std::string *name);
			virtual bool genSetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t);
			virtual bool genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t);
			virtual bool genFunctionDefs(std::ostream& header, std::string *name, Module *Mod, Object *t);
			virtual bool genLogic(std::ostream& logic, std::string *name, Module *Mod, Object *t);
			virtual bool genFunctionDefs(std::ostream& header, Module *Mod) = 0;
			virtual bool genLogic(std::ostream& logic) = 0;
			virtual bool genTemplate(std::ostream& templ) = 0;
			virtual bool haveFunctions() { return false; };
			virtual bool haveLogic() = 0;
			virtual std::string initValue() { return "NULL"; };			
			virtual bool lock_code_print(std::ostream& f, bool null);
			virtual bool unlock_code_print(std::ostream& f);
			virtual bool needs_connecting() { return (!nolock && !noref); };
			virtual bool needs_referencing() { return (!nolock && !noref); };
			std::string *name() { return this->Name; };
		};

	class Type : public Object {
		private:
			typedef Object super;
			std::map<std::string *, Member<Element> *> members;
			std::map<std::string *, Member<Element> *>::iterator memberIterBegin();
			std::map<std::string *, Member<Element> *>::iterator memberIterEnd();
			std::map<std::string *, Function *> functions;
			std::map<std::string *, Function *>::iterator functionIterBegin();
			std::map<std::string *, Function *>::iterator functionIterEnd();
			virtual bool create_def_print(std::ostream& f);
			virtual bool func_def_print(std::ostream& f, std::string fname);
			virtual bool create_func_print(std::ostream& f);		
			virtual bool destroy_func_print(std::ostream& f);
			virtual bool ref_func_print(std::ostream& f);
			virtual bool unref_func_print(std::ostream& f);
		public:
			explicit Type(Module *mod, std::string *objName, bool nl = false, bool nr = false) : Object(mod, objName, nl, nr) {};
			bool memberAdd(Member<Element> *m, std::string *name);
			Element *memberFind(std::string *);
			bool functionAdd(Function *);
			Function *functionFind(std::string *);
			virtual bool genType(std::ostream& header);
			virtual bool genTypeDef(std::ostream& header);
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genStruct(std::ostream& header);
			virtual bool genFunctionDefs(std::ostream& header, Module *Mod);
			virtual bool haveFunctions();
			virtual bool haveLogic() { return true; };
			virtual bool genLogic(std::ostream& logic);
			virtual bool genTemplate(std::ostream& templ);
			virtual bool print_connect(std::ostream& f);
			virtual bool print_disconnect(std::ostream& f);
			virtual bool needs_disconnecting() { return true; };
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Type)
						LUA_ADD_TABLE_FUNC(L, "memberAdd", li_type_memberAdd);
						LUA_ADD_TABLE_FUNC(L, "functionCreate", li_type_functionCreate);
						super::lua_table_r(L); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};
	
	class FunctionPointer : public Object {
		private:
			typedef Object super;
			std::map<std::string *, Element *> parameters;
			std::map<std::string *, Element *>::iterator paramsIterBegin();
			std::map<std::string *, Element *>::iterator paramsIterEnd();
			Element *ReturnType;
			virtual bool create_def_print(std::ostream& f);
			virtual bool func_def_print(std::ostream& f, std::string fname);
			virtual bool create_func_print(std::ostream& f);		
			virtual bool destroy_func_print(std::ostream& f);
			virtual bool ref_func_print(std::ostream& f);
			virtual bool unref_func_print(std::ostream& f);
		public:
			explicit FunctionPointer(Module *mod, std::string *fpName, Element *rt) : Object(mod, fpName, false, false), ReturnType(rt) {};
			bool memberAdd(Element *e, std::string *name);
			Element *memberFind(std::string *);
			virtual bool genType(std::ostream& header);
			virtual bool genTypeDef(std::ostream& header);
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genStruct(std::ostream& header);
			virtual bool genFunctionDefs(std::ostream& header, Module *Mod);
			virtual bool haveFunctions() { return false; };
			virtual bool genLogic(std::ostream& logic) { return false; };
			virtual bool haveLogic() { return false; };
			virtual bool genTemplate(std::ostream& templ);
			virtual bool needs_disconnecting() { return false; };
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,FunctionPointer)
							LUA_ADD_TABLE_FUNC(L, "memberAdd", li_fp_memberAdd);
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
			explicit SystemType(std::string *TypeName) : Element(), TypeName(TypeName) {};
			virtual bool genType(std::ostream& header);
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genSetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t);
			virtual bool genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t);
			virtual bool genFunctionDefs(std::ostream& header, std::string *, Module *Mod, Object *t);
			virtual bool genLogic(std::ostream& logic, std::string *name, Module *Mod, Object *t);
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
			explicit Pointer(Element *to, std::string *destroy) : Element(), To(to), Destroy(destroy) { if(this->Destroy == NULL) { std::cerr << "Error Destroy is NULL" << std::endl; } };
			virtual bool genType(std::ostream& header);
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genSetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t);
			virtual bool genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t);
			virtual bool genFunctionDefs(std::ostream& header, std::string *, Module *Mod, Object *t);
			virtual bool genLogic(std::ostream& logic, std::string *name, Module *Mod, Object *t);
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
			explicit Array(Element *of) : Element(),  Of(of) {};
			virtual bool genType(std::ostream& header);
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genAddFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t, bool, bool);
			virtual bool genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t, bool, bool);
			virtual bool genDelFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t, bool, bool);
			virtual bool genSizeFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t, bool, bool);
			virtual bool genFunctionDefs(std::ostream& header, std::string *, Module *Mod, Object *t);
			virtual bool genLogic(std::ostream& logic, std::string *name, Module *Mod, Object *t);
			virtual bool genDestruct(std::ostream& logic, std::string *name);
			virtual std::string initValue() { return "NULL"; };
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Array)
						super::lua_table_r(L); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};

	class Queue : public Object {
		private:
			typedef Object super;
			Element *Of;
			virtual bool create_def_print(std::ostream& f);
			virtual bool func_def_print(std::ostream& f, std::string fname);
			virtual bool create_func_print(std::ostream& f);		
			virtual bool destroy_func_print(std::ostream& f);
			virtual bool ref_func_print(std::ostream& f);
			virtual bool unref_func_print(std::ostream& f);
		public:
			explicit Queue(Element *of, Module *mod, std::string *objName) : Object(mod, objName, true, true),  Of(of) {};
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genType(std::ostream& header);
			virtual bool genTypeDef(std::ostream& header);
			virtual bool genStruct(std::ostream& header);
			virtual bool genFunctionDefs(std::ostream& header, Module *Mod);
			virtual bool genLogic(std::ostream& logic);
			virtual bool genTemplate(std::ostream& templ);
			virtual bool haveLogic() { return true; };

			// Queue specific
			virtual bool genPushFunctionDef(std::ostream& header);
			virtual bool genPopFunctionDef(std::ostream& header);
			virtual bool genSizeFunctionDef(std::ostream& header);
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Queue)
						super::lua_table_r(L); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};
	
	class BST : public Object {
		private:
			typedef Object super;
			bool ownfunc;
			Element *Of;
			virtual bool create_def_print(std::ostream& f);
			virtual bool func_def_print(std::ostream& f, std::string fname);
			virtual bool create_func_print(std::ostream& f);		
			virtual bool destroy_func_print(std::ostream& f);
			virtual bool ref_func_print(std::ostream& f);
			virtual bool unref_func_print(std::ostream& f);
		public:
			explicit BST(Element *of, Module *mod, std::string *objName, bool func) : Object(mod, objName, false, false),  ownfunc(func), Of(of) {};
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genType(std::ostream& header);
			virtual bool genTypeDef(std::ostream& header);
			virtual bool genStruct(std::ostream& header);
			virtual bool genFunctionDefs(std::ostream& header, Module *Mod);
			virtual bool genLogic(std::ostream& logic);
			virtual bool genTemplate(std::ostream& templ);
			virtual bool haveFunctions() { return ownfunc; };
			virtual bool haveLogic() { return true; };
			// BST specific
			virtual bool genInsertFunctionDef(std::ostream& header);
			virtual bool genFindFunctionDef(std::ostream& header);
			virtual bool genRemoveFunctionDef(std::ostream& header);
			virtual bool genNodeStruct(std::ostream& header, std::string name);
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,BST)
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
			std::map<std::string *, Object *> *Objects;
			std::map<std::string *, Object *>::iterator objectsIterBegin();
			std::map<std::string *, Object *>::iterator objectsIterEnd();
			std::list<std::string *> includes;
		public:
			explicit Module(std::string *name, std::string *Path, std::string *FilePrefix, std::string *FuncPrefix) :
			Name(name), Path(Path), FilePrefix(FilePrefix), FuncPrefix(FuncPrefix)
			{ this->Objects = new std::map<std::string *, Object *>(); };
			bool objectAdd(std::string *objName, Object *object);
			Object *objectFind(std::string *objName);
			bool generate(std::string *name);
			std::string *funcPrefix() { return this->FuncPrefix; };
			std::string *filePrefix() { return this->FilePrefix; };
			std::string *name() { return this->Name; };
			bool includeAdd(std::string *inc) { this->includes.push_back(inc); return true; };
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Module)
							LUA_ADD_TABLE_FUNC(L,"newType",li_module_type_create)
							LUA_ADD_TABLE_FUNC(L,"newFunctionPointer",li_module_function_pointer_create)
							LUA_ADD_TABLE_FUNC(L,"newQueue",li_module_queue_create)
							LUA_ADD_TABLE_FUNC(L,"newBST",li_module_bst_create)
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
