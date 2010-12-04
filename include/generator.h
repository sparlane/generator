#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <sstream>

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

namespace generator {
	class Element;
	class Object;
	class Function;
	class Member;
	class RType;
	class Pointer;
	class ObjectMember;
	class Array;
	class Module;
	class World;

	class Element {
		public:
			explicit Element() {};
			virtual bool genStruct(FILE *header);
			virtual bool genFunctionDefs(FILE *header);
			virtual bool genLogic(FILE *logic);
			virtual bool genTemplate(FILE *templ);
	};

	class Type : public Element {
		private:
			Module *Mod;
			std::string *Name;
			bool nolock;
			bool noref;
			std::map<std::string *, Member *> members;
			std::map<std::string *, Member *>::iterator memberIterBegin();
			std::map<std::string *, Member *>::iterator memberIterEnd();
			std::map<std::string *, Function *> functions;
			std::map<std::string *, Function *>::iterator functionIterBegin();
			std::map<std::string *, Function *>::iterator functionIterEnd();
			bool create_def_print(FILE *f);		
			bool destroy_def_print(FILE *f);
			bool ref_def_print(FILE *f);
			bool unref_def_print(FILE *f);
			bool create_func_print(FILE *f);		
			bool destroy_func_print(FILE *f);
			bool ref_func_print(FILE *f);
			bool unref_func_print(FILE *f);
			bool destroy_lock_code_print(FILE *f);
		public:
			explicit Type(Module *mod, std::string *objName) : Name(objName), Mod(mod), nolock(false), noref(false), Element() {};
			bool memberAdd(Member *);
			Element *memberFind(std::string *);
			bool functionAdd(Function *);
			Function *functionFind(std::string *);
			std::string *name() { return this->Name; };
			virtual bool genStruct(FILE *header);
			virtual bool genFunctionDefs(FILE *header, Module *Mod);
			virtual bool genLogic(FILE *logic);
			virtual bool genTemplate(FILE *templ);
			bool lock_code_print(FILE *f, bool null);
			bool unlock_code_print(FILE *f);
			bool print_disconnect_function(FILE *f);
			bool needs_referencing() { return (!nolock && !noref); };
			bool print_connect_function(FILE *f);
	};
	
	class Member : public Element {
		private:
			bool init;
			bool input;
			std::string *Name;
			std::string *InitValue;
		public:
			explicit Member(bool init, bool input, std::string *name, std::string *initValue) : init(init), input(input), Name(name), InitValue(initValue), Element() {};
			std::string *name() { return this->Name; };
			std::string *initValue() { return this->InitValue; };
			bool isInit() { return this->init; };
			bool isInput() { return this->input; };
			virtual bool genStruct(FILE *header);
			virtual bool genType(FILE *header);
			virtual bool genFunctionDefs(FILE *header, Module *Mod, Type *t);
			virtual bool genDestruct(FILE *logic);
			virtual bool genLogic(FILE *logic, Module *Mod, Type *t);
			virtual bool genTemplate(FILE *templ);
			virtual bool needs_connecting() { return false; };
			virtual bool needs_disconnecting() { return false; };
			virtual bool print_connect(FILE *logic);
			virtual bool print_disconnect(FILE *logic);
	};
	
	class Function : public Element {
		private:
			std::map<std::string *, Member *> parameters;
			std::map<std::string *, Member *>::iterator paramsIterBegin();
			std::map<std::string *, Member *>::iterator paramsIterEnd();
			std::string *Name;
			Member *ReturnType;
			bool genFunctionDef(FILE *header, Module *Mod, Type *t, bool tpl, bool type);
		public:
			explicit Function(std::string *Name, Member *rt) : Name(Name), ReturnType(rt), Element() {};
			bool paramAdd(Member *mem);
			std::string *name() { return this->Name; };
			virtual bool genFunctionDefs(FILE *header, Module *Mod, Type *t);
			virtual bool genLogic(FILE *logic, Module *Mod, Type *t);
			virtual bool genTemplate(FILE *templ, Module *Mod, Type *t);
	};
	
	class RType : public Member {
		private:
			std::string *TypeName;
		public:
			explicit RType(std::string *TypeName, bool init, bool input, std::string *Name, std::string *InitValue) : TypeName(TypeName), Member(init, input, Name, InitValue) {};
			virtual bool genType(FILE *header);
			virtual bool genSetFunctionDef(FILE *header, Module *Mod, Type *t);
			virtual bool genFunctionDefs(FILE *header, Module *Mod, Type *t);
			virtual bool genLogic(FILE *logic, Module *Mod, Type *t);
			virtual bool genDestruct(FILE *logic);
	};
	
	class Pointer : public Member {
		private:
			Member *To;
			std::string *Destroy;
		public:
			explicit Pointer(Member *to, std::string *Name, std::string *destroy, bool init, bool input, std::string *InitValue) : To(to), Destroy(destroy), Member(init, input, Name, InitValue) { if(this->Destroy == NULL) { std::cerr << "Error Destroy is NULL" << std::endl; } };
			virtual bool genType(FILE *header);
			virtual bool genSetFunctionDef(FILE *header, Module *Mod, Type *t);
			virtual bool genFunctionDefs(FILE *header, Module *Mod, Type *t);
			virtual bool genDestruct(FILE *logic);
			virtual bool genLogic(FILE *logic, Module *Mod, Type *t);
	};
	
	class ObjectMember : public Member {
		private:
			Type *Object;
		public:
			explicit ObjectMember(Type *object, bool init, bool input, std::string *Name, std::string *InitValue) : Object(object), Member(init, input, Name, InitValue) {};
			virtual bool genType(FILE *header);
			virtual bool genSetFunctionDef(FILE *header, Module *Mod, Type *t);
			virtual bool genFunctionDefs(FILE *header, Module *Mod, Type *t);
			virtual bool genDestruct(FILE *logic);		
			virtual bool genLogic(FILE *logic, Module *Mod, Type *t);
	};
	
	class Array : public Member {
		private:
			Member *Of;
		public:
			explicit Array(Member *of, std::string *Name) : Of(of), Member(false, false, Name, NULL) {};
			virtual bool genStruct(FILE *header);
			virtual bool genType(FILE *header);
			virtual bool genAddFunctionDef(FILE *header, Module *Mod, Type *t);
			virtual bool genFunctionDefs(FILE *header, Module *Mod, Type *t);
			virtual bool genDestruct(FILE *logic);		
			virtual bool genLogic(FILE *logic, Module *Mod, Type *t);
	};
	
	class Module {
		private:
			std::string *Name;
			std::string *Path;
			std::string *FilePrefix;
			std::string *FuncPrefix;
			std::map<std::string *, Type *> *Objects;
			std::map<std::string *, Type *>::iterator objectsIterBegin();
			std::map<std::string *, Type *>::iterator objectsIterEnd();
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
