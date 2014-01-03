#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <list>
#include <ostream>
#include <fstream>
#include <set>

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
	int li_enum_valueAdd(lua_State *L);
	int li_module_type_create(lua_State *L);
	int li_module_enum_create(lua_State *L);
	int li_module_function_pointer_create(lua_State *L);
	int li_module_queue_create(lua_State *L);
	int li_module_bst_create(lua_State *L);
	int li_module_heap_create(lua_State *L);
	int li_module_conditional_create(lua_State *L);
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
			std::string *Name;
		protected:
			Module *mod;
		public:
			explicit Element(std::string *name) : Name(name), mod(NULL) {};
			explicit Element(std::string *name, Module *m) : Name(name), mod(m) {};
			virtual Module *module() { return this->mod; };
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
			std::string *name() { return this->Name; };
			virtual std::string include() = 0;
			virtual bool populate_dependencies(std::set<Module *>& deps) { if(this->mod != NULL) deps.insert(this->mod); return true; };
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
			std::string *getInit() { return (this->initString == NULL) ? new std::string(type->initValue()) : this->initString; };
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
			virtual std::string include()
			{
				return type->include();
			}
			virtual std::string *name()
			{
				return type->name();
			}
			virtual Module *module()
			{
				return type->module();
			}

	};
	
	typedef std::list< std::pair<std::string *, Member<Element> *> > MemberList;
	typedef std::list< std::pair<std::string *, Element *> > ElementList;

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
			std::map<std::string *, Object *> *FunctionPointers;
			std::map<std::string *, Object *>::iterator fpIterBegin();
			std::map<std::string *, Object *>::iterator fpIterEnd();
			std::set<std::string> includes;
		public:
			explicit Module(std::string *name, std::string *Path, std::string *FilePrefix, std::string *FuncPrefix) :
			Name(name), Path(Path), FilePrefix(FilePrefix), FuncPrefix(FuncPrefix)
			{ this->Objects = new std::map<std::string *, Object *>(); this->FunctionPointers = new std::map<std::string *, Object *>(); };
			bool fpAdd(std::string *objName, Object *object);
			bool objectAdd(std::string *objName, Object *object);
			Object *objectFind(std::string *objName);
			bool generate(std::string *name, const char *output_dir);
			std::string *funcPrefix() { return this->FuncPrefix; };
			std::string *filePrefix() { return this->FilePrefix; };
			std::string *name() { return this->Name; };
			std::string *path() { return this->Path; };
			bool includeAdd(std::string *inc) { this->includes.insert(*inc); return true; };
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Module)
							LUA_ADD_TABLE_FUNC(L,"newType",li_module_type_create)
							LUA_ADD_TABLE_FUNC(L,"newEnum",li_module_enum_create)
							LUA_ADD_TABLE_FUNC(L,"newFunctionPointer",li_module_function_pointer_create)
							LUA_ADD_TABLE_FUNC(L,"newQueue",li_module_queue_create)
							LUA_ADD_TABLE_FUNC(L,"newBST",li_module_bst_create)
							LUA_ADD_TABLE_FUNC(L,"newHeap",li_module_heap_create)
							LUA_ADD_TABLE_FUNC(L,"newConditional",li_module_conditional_create)
							LUA_ADD_TABLE_FUNC(L,"addInclude",li_module_include_add)
						}
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};
	

	class Object : public Element {
		private:
			typedef Element super;
			std::string *Name;
			virtual bool create_def_print(std::ostream& f) = 0;
			virtual bool func_def_print(std::ostream& f, std::string fname) = 0;
			virtual bool create_func_print(std::ostream& f) = 0;
			virtual bool destroy_func_print(std::ostream& f) = 0;
			virtual bool ref_func_print(std::ostream& f) = 0;
			virtual bool unref_func_print(std::ostream& f) = 0;
		protected:
			bool nolock;
			bool noref;
			virtual bool destroy_lock_code_print(std::ostream& f);
		public:
			explicit Object(Module *mod, std::string *objName, bool nl, bool nr) : Element(objName, mod), Name(objName), nolock(nl), noref(nr) {};
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
			virtual std::string include();
			virtual bool lock_code_print(std::ostream& f, bool null);
			virtual bool unlock_code_print(std::ostream& f);
			virtual bool needs_connecting() { return (!nolock && !noref); };
			virtual bool needs_referencing() { return (!nolock && !noref); };
			virtual bool populate_dependencies(std::set<Module *>& deps) = 0;
		};

	class Type : public Object {
		private:
			typedef Object super;
			MemberList members;
			MemberList::iterator memberIterBegin();
			MemberList::iterator memberIterEnd();
			std::map<std::string *, Function *> functions;
			virtual bool create_def_print(std::ostream& f);
			virtual bool func_def_print(std::ostream& f, std::string fname);
			virtual bool create_func_print(std::ostream& f);		
			virtual bool destroy_func_print(std::ostream& f);
			virtual bool ref_func_print(std::ostream& f);
			virtual bool unref_func_print(std::ostream& f);
		protected:
			std::map<std::string *, Function *>::iterator functionIterBegin();
			std::map<std::string *, Function *>::iterator functionIterEnd();
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
			virtual bool genDestruct(std::ostream& logic, std::string *name);
			virtual bool haveFunctions();
			virtual bool haveLogic() { return true; };
			virtual bool genLogic(std::ostream& logic);
			virtual bool genTemplate(std::ostream& templ);
			virtual bool print_connect(std::ostream& f);
			virtual bool print_disconnect(std::ostream& f);
			virtual bool needs_disconnecting() { return true; };
			virtual bool populate_dependencies(std::set<Module *>& deps);
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Type)
						LUA_ADD_TABLE_FUNC(L, "memberAdd", li_type_memberAdd);
						LUA_ADD_TABLE_FUNC(L, "functionCreate", li_type_functionCreate);
						super::lua_table_r(L); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};
	
	class FunctionPointer : public Object {
		private:
			typedef Object super;
			ElementList parameters;
			ElementList::iterator paramsIterBegin();
			ElementList::iterator paramsIterEnd();
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
			virtual bool populate_dependencies(std::set<Module *>& deps) { return true; };
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,FunctionPointer)
							LUA_ADD_TABLE_FUNC(L, "paramAdd", li_fp_memberAdd);
							super::lua_table_r(L); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};
	

	class Function {
		private:
			MemberList parameters;
			MemberList::iterator paramsIterBegin();
			MemberList::iterator paramsIterEnd();
			std::string *Name;
			std::list<std::string *> myIncludes;
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
			std::list<std::string *> includes();
			Element *retType() { return this->ReturnType; };
			virtual bool populate_dependencies(std::set<Module *>& deps);
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Function)
						LUA_ADD_TABLE_FUNC(L, "paramAdd", li_function_paramAdd); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};
	
	class SystemType : public Element {
		private:
			typedef Element super;
			std::string *TypeName;
		public:
			explicit SystemType(std::string *TypeName) : Element(TypeName), TypeName(TypeName) {};
			virtual bool genType(std::ostream& header);
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genSetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t);
			virtual bool genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t);
			virtual bool genFunctionDefs(std::ostream& header, std::string *, Module *Mod, Object *t);
			virtual bool genLogic(std::ostream& logic, std::string *name, Module *Mod, Object *t);
			virtual bool genDestruct(std::ostream& logic, std::string *name);
			virtual std::string initValue() { return "0"; };
			virtual std::string include() { return ""; };
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
			explicit Pointer(Element *to, std::string *destroy) : Element(new std::string("PTR"), to->module()), To(to), Destroy(destroy) { if(this->Destroy == NULL) { std::cerr << "Error Destroy is NULL" << std::endl; } };
			virtual bool genType(std::ostream& header);
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genSetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t);
			virtual bool genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t);
			virtual bool genFunctionDefs(std::ostream& header, std::string *, Module *Mod, Object *t);
			virtual bool genLogic(std::ostream& logic, std::string *name, Module *Mod, Object *t);
			virtual bool genDestruct(std::ostream& logic, std::string *name);
			virtual std::string initValue() { return "NULL"; };
			virtual std::string include() { return ""; };
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Pointer)
						super::lua_table_r(L); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};

	class Array : public Element {
		private:
			typedef Element super;
			Element *Of;
		public:
			explicit Array(Element *of) : Element(new std::string("ARRAY"), of->module()),  Of(of) {};
			virtual bool genType(std::ostream& header);
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genAddFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t, bool, bool);
			virtual bool genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t, bool, bool);
			virtual bool genDelFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t, bool, bool);
			virtual bool genSizeFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t, bool, bool);
			virtual bool genFunctionDefs(std::ostream& header, std::string *, Module *Mod, Object *t);
			virtual bool genLogic(std::ostream& logic, std::string *name, Module *Mod, Object *t);
			virtual bool genDestruct(std::ostream& logic, std::string *name);
			virtual std::string initValue() { return std::string("NULL"); };
			virtual std::string include() { return Of->include(); };
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Array)
						super::lua_table_r(L); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};

	
	typedef std::list< std::pair< std::string *, int > > string_int_set;

	class Enum : public Object {
		private:
			typedef Object super;
			std::string *EnumName;
			string_int_set members;
		public:
			explicit Enum(Module *mod, std::string *EnumName) : Object(mod, EnumName, true, true), EnumName(EnumName) {};

			virtual bool valueAdd(std::string *s, int v) { this->members.push_back(std::make_pair(s,v)); return true; };

			virtual bool genType(std::ostream& header);
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genSetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t) { return true; };
			virtual bool genGetFunctionDef(std::ostream& header, std::string *name, Module *Mod, Object *t) { return true; };
			//virtual bool genFunctionDefs(std::ostream& header, std::string *, Module *Mod, Object *t) { return true; };
			virtual bool genLogic(std::ostream& logic, std::string *name, Module *Mod, Object *t) { return true; };
			virtual bool genDestruct(std::ostream& logic, std::string *name) { return true; };
			virtual std::string initValue() { return "-1"; };
			virtual std::string include() { return ""; };
			
			virtual bool genTemplate(std::ostream&) { return true; };
			virtual bool populate_dependencies(std::set<generator::Module*>&) { return true; };
			virtual bool create_def_print(std::ostream&) { return true; };
		 	virtual bool func_def_print(std::ostream&, std::string) { return true; };
		 	virtual bool create_func_print(std::ostream&) { return true; };
		 	virtual bool destroy_func_print(std::ostream&) { return true; };
		 	virtual bool ref_func_print(std::ostream&) { return true; };
		 	virtual bool unref_func_print(std::ostream&) { return true; };
		 	virtual bool genStruct(std::ostream&) { return true; };
		 	virtual bool genTypeDef(std::ostream&);
		 	virtual bool genFunctionDefs(std::ostream&, generator::Module*) { return true; };
		 	virtual bool genLogic(std::ostream&) { return true; };
		 	virtual bool haveLogic() { return false; };

			
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Enum)
								LUA_ADD_TABLE_FUNC(L, "valueAdd", li_enum_valueAdd);
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
			virtual bool populate_dependencies(std::set<Module *>& deps) { return true; };

			// Queue specific
			virtual bool genPushFunctionDef(std::ostream& header);
			virtual bool genPopFunctionDef(std::ostream& header);
			virtual bool genSizeFunctionDef(std::ostream& header);
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Queue)
						super::lua_table_r(L); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};
	
	class Tree : public Type {
		private:
			typedef Type super;
			bool ownfunc;
			Element *Of;
		protected:
			virtual bool create_def_print(std::ostream& f);
			virtual bool func_def_print(std::ostream& f, std::string fname);
			virtual bool create_func_print(std::ostream& f);		
			virtual bool destroy_func_print(std::ostream& f);
			virtual bool ref_func_print(std::ostream& f);
			virtual bool unref_func_print(std::ostream& f);
		public:
			explicit Tree(Element *of, Module *mod, std::string *objName, bool func) : Type(mod, objName, false, false), ownfunc(func), Of(of) {};
			virtual bool ownLogic() { return ownfunc; };
			virtual bool haveFunctions() { return this->ownLogic() || this->functionIterBegin() != this->functionIterEnd(); };
			virtual Element *of() { return this->Of; };
			
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genType(std::ostream& header);
			virtual bool genTypeDef(std::ostream& header);
			virtual bool genStruct(std::ostream& header);
			virtual bool genFunctionDefs(std::ostream& header, Module *Mod);
			virtual bool haveLogic() { return true; };

			virtual bool populate_dependencies(std::set<Module *>& deps) { return true; };
			
			virtual bool genInsertFunctionDef(std::ostream& header);
			virtual bool genFindFunctionDef(std::ostream& header);
			virtual bool genRemoveFunctionDef(std::ostream& header);
			virtual bool genNodeStruct(std::ostream& header, std::string name);
			
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Tree)
						super::lua_table_r(L); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};
	
	class BST : public Tree {
		private:
			typedef Tree super;
		protected:
			virtual bool destroy_func_print(std::ostream& f);
		public:
			explicit BST(Element *of, Module *mod, std::string *objName, bool func) : Tree(of, mod, objName, func) {};
			

			virtual bool genLogic(std::ostream& logic);
			virtual bool genTemplate(std::ostream& templ);

			// BST specific
			
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,BST)
						LUA_ADD_TABLE_FUNC(L, "functionCreate", li_type_functionCreate);
						super::lua_table_r(L); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};
	
	class Heap : public Tree {
		private:
			typedef Tree super;
			bool max;
		public:
			explicit Heap(Element *of, Module *mod, std::string *objName, bool func, bool max) : Tree(of, mod, objName, func), max(max) {};

			virtual bool genLogic(std::ostream& logic);
			virtual bool genTemplate(std::ostream& templ);

			virtual bool genRemoveFunctionDef(std::ostream& header);
			virtual bool genFunctionDefs(std::ostream& header, Module *Mod);
			
			// HEAP specific
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,HEAP)
						super::lua_table_r(L); }
			virtual void lua_table(lua_State *L) { lua_table_r(L); };
	};

	class Conditional : public Object {
		private:
			typedef Object super;
			virtual bool genLockFunctionDef(std::ostream& header, bool unlock);
			virtual bool genWaitFunctionDef(std::ostream& header);
			virtual bool genSignalFunctionDef(std::ostream& header);
			virtual bool create_def_print(std::ostream& f);
			virtual bool func_def_print(std::ostream& f, std::string fname);
			virtual bool create_func_print(std::ostream& f);		
			virtual bool destroy_func_print(std::ostream& f);
			virtual bool ref_func_print(std::ostream& f);
			virtual bool unref_func_print(std::ostream& f);
		public:
			explicit Conditional(Module *mod, std::string *objName) : Object(mod, objName, false, false) { };
			virtual bool genType(std::ostream& header);
			virtual bool genTypeDef(std::ostream& header);
			virtual bool genStruct(std::ostream& header, std::string name);
			virtual bool genTemplate(std::ostream& templ);
			virtual bool genStruct(std::ostream& header);
			virtual bool genFunctionDefs(std::ostream& header, Module *Mod);
			virtual bool genLogic(std::ostream& logic);
			virtual bool haveLogic() { return true; };
			virtual bool populate_dependencies(std::set<Module *>& deps) { return true; };

			virtual std::string initValue() { return "NULL"; };
			static void lua_table_r(lua_State *L) { LUA_SET_TABLE_TYPE(L,Conditional)
						super::lua_table_r(L); }
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
			bool generate(const char *output_dir);			
	};
	
	extern World *WORLD;
	void luaopen_process(lua_State *state);
}

std::ostream& operator<<(std::ostream& os, std::string *str);
