#include <generator.h>

static char *to_upper_string(const char *str)
{
	if(str == NULL) return NULL;
	size_t len = strlen(str);
	char *ret = malloc(len + 1);
	for(int i = 0; i < len; i++)
	{
		ret[i] = toupper(str[i]);
	}
	ret[len] = '\0';
	return ret;
}

static bool generate_object_typedef(FILE *header, object *o)
{
	if(header == NULL || o == NULL) return false;
	
	int res = fprintf(header, "typedef struct %s_s *%s;\n", o->name, o->name);
	if(res <= 0)
		gen_error(strerror(errno));

	return true;
}

static bool generate_member_def(FILE *header, member *m)
{
	switch(m->type)
	{
		case member_type_object: {
			int res = fprintf(header, "%s %s", m->o->name, m->name);
			if(res <= 0)
				gen_error(strerror(errno));
		} break;
		case member_type_type: {
			int res = fprintf(header, "%s %s", m->type_name, m->name);
			if(res <= 0)
				gen_error(strerror(errno));
		} break;
		case member_type_pointer: {
			int res = fprintf(header, "%s *%s", m->type_name, m->name);
			if(res <= 0)
				gen_error(strerror(errno));
		} break;
		default: gen_error("Unknown member type\n");
	}
	
	return true;	
}

static bool generate_object_struct(FILE *header, object *o)
{
	if(header == NULL || o == NULL) return false;
	
	int res = fprintf(header, "struct %s_s {\n", o->name);
	if(res <= 0)
		gen_error(strerror(errno));

	if(o->locked)
	{
		// TODO: pthread_mutex_t might be the wrong type
		res = fprintf(header, "\tpthread_mutex_t lock;\n");
		if(res <= 0)
			gen_error(strerror(errno));
		
		if(o->refcount)
		{
			// TODO: configure the size of this
			res = fprintf(header, "\tunsigned int references;\n");
			if(res <= 0)
				gen_error(strerror(errno));
		}
	}
	
	for(int i = 0; i < o->member_count; i++)
	{
		member *m = o->members[i];
		res = fprintf(header, "\t");
		if(res <= 0)
			gen_error(strerror(errno));
		if(!generate_member_def(header, m)) return false;
		res = fprintf(header, ";\n");
		if(res <= 0)
			gen_error(strerror(errno));
	}
	
	res = fprintf(header, "};\n");
	
	return true;
}

static bool generate_object_function_definition_create(FILE *f, object *o, bool semi)
{
	if(f == NULL || o == NULL) return false;
	
	int res = fprintf(f, "%s %s_create(", o->name, o->name);
	if(res <= 0)
		gen_error(strerror(errno));
	
	int param_count = 0;
	for(int i = 0; i < o->member_count; i++)
	{
		if(o->members[i]->init)
		{
			if(o->members[i]->init_input)
			{
				if(param_count)
				{
					res = fprintf(f, ", ");
					if(res <= 0)
						gen_error(strerror(errno));
				}
				if(!generate_member_def(f, o->members[i])) return false;
				param_count++;
			}
		}
	}
	if(param_count == 0)
	{
		res = fprintf(f, "void");
		if(res <= 0)
			gen_error(strerror(errno));
	}
	
	res = fprintf(f, (semi) ? ");\n" : ")\n");
	if(res <= 0)
		gen_error(strerror(errno));

	return true;
}

static bool generate_object_function_definitions(FILE *header, object *o)
{
	if(header == NULL || o == NULL) return false;
	
	if(!generate_object_function_definition_create(header, o, true)) return false;
	
	int res = 0;

	if(o->locked && o->refcount)
	{
		res = fprintf(header, "bool %s_ref(%s o);\n", o->name, o->name);
		if(res <= 0)
			gen_error(strerror(errno));
		
		res = fprintf(header, "bool %s_unref(%s o);\n", o->name, o->name);
		if(res <= 0)
			gen_error(strerror(errno));
	} else {
		res = fprintf(header, "bool %s_destroy(%s o);\n", o->name, o->name);
		if(res <= 0)
			gen_error(strerror(errno));
	}
	
	return true;	
}

static bool generate_object_function_lock(FILE *code, object *o, bool null)
{
	if(code == NULL || o == NULL) return false;
	
	int res = fprintf(code, "\t{\n\t\tint lock_res = pthread_mutex_lock(&o->lock);\n");
	if(res <= 0)
		gen_error(strerror(errno));
	
	res = fprintf(code, "\t\tif(lock_res != 0) return %s;\n\t}\n\n", null ? "NULL" : "false");
	if(res <= 0)
		gen_error(strerror(errno));
	return true;
}

static bool generate_object_function_unlock(FILE *code, object *o)
{
	if(code == NULL || o == NULL) return false;
	
	int res = fprintf(code, "\t{\n\t\tint lock_res = pthread_mutex_unlock(&o->lock);\n");
	if(res <= 0)
		gen_error(strerror(errno));
	
	res = fprintf(code, "\t}\n\n");
	if(res <= 0)
		gen_error(strerror(errno));
	return true;
}

static bool generate_object_function_create(FILE *code, object *o)
{
	if(code == NULL || o == NULL) return false;
	
	int res = fprintf(code, "\n\n/* Create a %s */\n", o->name);
	if(res <= 0)
		gen_error(strerror(errno));
	
	if(!generate_object_function_definition_create(code, o, false)) return false;
	
	res = fprintf(code, "{\n");
	if(res <= 0)
		gen_error(strerror(errno));
	
	res = fprintf(code, "\t%s o = calloc(1, sizeof(struct %s_s));\n", o->name, o->name);
	if(res <= 0)
		gen_error(strerror(errno));
	res = fprintf(code, "\tif(o == NULL) goto ERROR_EARLY;\n\n");
	if(res <= 0)
		gen_error(strerror(errno));

	struct error_s {
		char *name;
		char *undo;
	} *errors = NULL;
	size_t error_count = 0;
	error_count++;
	errors = realloc(errors, sizeof(struct error_s) * error_count);
	if(errors == NULL)
		gen_error("Reallocing errors failed");
	errors[error_count-1].name = strdup("EARLY");
	errors[error_count-1].undo = strdup("free(o);");

	if(o->locked)
	{
		// TODO: maybe pthread is the wrong implementation of lock ?
		res = fprintf(code, "\tpthread_mutexattr_t mutex_attr;\n\tif(pthread_mutexattr_init(&mutex_attr) != 0) goto ERROR_LOCK;\n");
		if(res <= 0)
			gen_error(strerror(errno));

		res = fprintf(code, "\tif(pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK) != 0) goto ERROR_LOCK;\n");
		if(res <= 0)
			gen_error(strerror(errno));
		
		res = fprintf(code, "\tif(pthread_mutex_init(&o->lock, &mutex_attr) != 0) goto ERROR_LOCK;\n\n");
		if(res <= 0)
			gen_error(strerror(errno));
		
		error_count++;
		errors = realloc(errors, sizeof(struct error_s) * error_count);
		if(errors == NULL)
			gen_error("Reallocing errors failed");
		
		errors[error_count-1].name = strdup("LOCK");
		errors[error_count-1].undo = strdup("pthread_mutex_destroy(&o->lock)");
			
		if(o->refcount)
		{
			res = fprintf(code, "\to->references = 1;\n\n");
			if(res <= 0)
				gen_error(strerror(errno));
		}
	}

	for(int i = 0; i < o->member_count; i++)
	{
		member *m = o->members[i];
		if(m->init && m->init_input)
		{
			res = fprintf(code, "\to->%s = %s;\n\n", m->name, m->name);
			if(res <= 0)
				gen_error(strerror(errno));
		}
		if(m->init && !m->init_input)
		{
			switch(m->type)
			{
				case member_type_object: {
					error_count++;
					errors = realloc(errors, sizeof(struct error_s) * error_count);
					if(errors == NULL)
						gen_error("Reallocing errors failed");
			
					errors[error_count-1].name = to_upper_string(m->name);
					errors[error_count-1].undo = NULL;
					res = asprintf(&errors[error_count-1].undo, "%s_%s(o->%s)", m->o->name, (m->o->locked && m->o->refcount) ? "unref" : "destroy", m->name);
					if(res <= 0)
						gen_error(strerror(errno));
		
					res = fprintf(code, "\to->%s = %s_create();\n", m->name, m->o->name);
					if(res <= 0)
						gen_error(strerror(errno));
					res = fprintf(code, "\tif(o->%s == NULL) goto ERROR_%s;\n\n", m->name, errors[error_count-1].name);
					if(res <= 0)
						gen_error(strerror(errno));
				} break;
				case member_type_type: {
					res = fprintf(code, "\to->%s = %s;\n\n", m->name, m->init_params[0]);
					if(res <= 0)
						gen_error(strerror(errno));
				} break;
				case member_type_pointer: {
					error_count++;
					errors = realloc(errors, sizeof(struct error_s) * error_count);
					if(errors == NULL)
						gen_error("Reallocing errors failed");
			
					errors[error_count-1].name = to_upper_string(m->name);
					errors[error_count-1].undo = NULL;
					res = asprintf(&errors[error_count-1].undo, "%s(o->%s)", m->destruct, m->name);
					if(res <= 0)
						gen_error(strerror(errno));
					res = fprintf(code, "\to->%s = %s;\n", m->name, m->init_params[0]);
					if(res <= 0)
						gen_error(strerror(errno));
					res = fprintf(code, "\tif(o->%s == NULL) goto ERROR_%s;\n\n", m->name, errors[error_count-1].name);
				} break;
				default: {
					gen_error("unknown member type");
				} break;
			}
		}
	}
	
	res = fprintf(code, "\treturn o;\n\n");
	if(res <= 0)
		gen_error(strerror(errno));
	
	// errors
	int ecnt = 0;
	for(int i = error_count - 1; i >= 0; i--)
	{
		if(i != error_count-1)
		{
			res = fprintf(code, "\t%s;\n", errors[i].undo);
			if(res <= 0)
				gen_error(strerror(errno));
		}
		res = fprintf(code, "ERROR_%s:\n", errors[i].name);
		if(res <= 0)
			gen_error(strerror(errno));
	}
	res = fprintf(code, "\treturn NULL;\n");
	if(res <= 0)
		gen_error(strerror(errno));
		
	res = fprintf(code, "}\n");
	if(res <= 0)
		gen_error(strerror(errno));
	
	return true;
}


static bool generate_object_function_destroy(FILE *code, object *o, bool local)
{
	if(code == NULL || o == NULL) return false;
	
	int res = fprintf(code, "\n\n/* Destroy a %s */\n", o->name);
	if(res <= 0)
		gen_error(strerror(errno));

	res = fprintf(code, "%s %s_destroy(%s o)\n", (local) ? "static bool" : "bool", o->name, o->name);
	if(res <= 0)
		gen_error(strerror(errno));

	res = fprintf(code, "{\n");
	if(res <= 0)
		gen_error(strerror(errno));
	
	res = fprintf(code, "\tif(o == NULL) return false;\n\n");
	if(res <= 0)
		gen_error(strerror(errno));

	if(o->locked)
		generate_object_function_lock(code, o, false);

	for(int i = 0; i < o->member_count; i++)
	{
		member *m = o->members[i];
		switch(m->type)
		{
			case member_type_object: {
				res = fprintf(code, "\tif(o->%s != NULL) %s_%s(o->%s);\n\n", m->name, m->o->name, (m->o->locked && m->o->refcount) ? "unref" : "destroy", m->name);
				if(res <= 0)
					gen_error(strerror(errno));
			} break;
			case member_type_type: {
				// do nothing
			} break;
			case member_type_pointer: {
				res = fprintf(code, "\tif(o->%s != NULL) ", m->name);
				if(res <= 0)
					gen_error(strerror(errno));
				res = fprintf(code, "%s(o->%s)", m->destruct, m->name);
				if(res <= 0)
					gen_error(strerror(errno));
				res = fprintf(code, ";\n\n");
				if(res <= 0)
					gen_error(strerror(errno));
			} break;
			default: {
				fprintf(stderr, "type = %i\n", m->type);
				gen_error("unknown member type");
			}
		}
	}

	if(o->locked)
	{
		generate_object_function_unlock(code, o);
		res = fprintf(code, "\tpthread_mutex_destroy(&o->lock);\n");
		if(res <= 0)
			gen_error(strerror(errno));
	}
	res = fprintf(code, "\tfree(o);\n");
	if(res <= 0)
		gen_error(strerror(errno));

	res = fprintf(code, "\treturn true;\n");
	if(res <= 0)
		gen_error(strerror(errno));

	res = fprintf(code, "}\n");
	if(res <= 0)
		gen_error(strerror(errno));

	return true;
}

static bool generate_object_function_refunref(FILE *code, object *o)
{
	if(code == NULL || o == NULL) return false;
	
	int res = fprintf(code, "\n\n/* Reference a %s */\n", o->name);
	if(res <= 0)
		gen_error(strerror(errno));
	
	res = fprintf(code, "bool %s_ref(%s o)\n", o->name, o->name);
	if(res <= 0)
		gen_error(strerror(errno));
		
	res = fprintf(code, "{\n");
	if(res <= 0)
		gen_error(strerror(errno));
	
	if(o->locked)
		if(!generate_object_function_lock(code, o, false))
			gen_error("generating locking code");
	
	res = fprintf(code, "\to->references++;\n\n");
	if(res <= 0)
		gen_error(strerror(errno));
	
	if(o->locked)
		if(!generate_object_function_unlock(code, o))
			gen_error("generating unlocking code");

	res = fprintf(code, "\treturn true;\n");
	if(res <= 0)
		gen_error(strerror(errno));

	res = fprintf(code, "}\n");
	if(res <= 0)
		gen_error(strerror(errno));
	
	
	res = fprintf(code, "\n\n/* Unreference a %s */\n", o->name);
	if(res <= 0)
		gen_error(strerror(errno));
	
	res = fprintf(code, "bool %s_unref(%s o)\n", o->name, o->name);
	if(res <= 0)
		gen_error(strerror(errno));
		
	res = fprintf(code, "{\n");
	if(res <= 0)
		gen_error(strerror(errno));
	
	if(o->locked)
		if(!generate_object_function_lock(code, o, false))
			gen_error("generating locking code");
	
	res = fprintf(code, "\to->references--;\n\n");
	if(res <= 0)
		gen_error(strerror(errno));
	
	res = fprintf(code, "\tbool done = (o->references == 0);\n\n");
	if(res <= 0)
		gen_error(strerror(errno));
	
	
	if(o->locked)
		if(!generate_object_function_unlock(code, o))
			gen_error("generating unlocking code");
	
	res = fprintf(code, "\tif(done) %s_destroy(o);\n\n", o->name);
	if(res <= 0)
		gen_error(strerror(errno));
	
	res = fprintf(code, "\treturn true;\n");
	if(res <= 0)
		gen_error(strerror(errno));

	res = fprintf(code, "}\n");
	if(res <= 0)
		gen_error(strerror(errno));
	return true;
}

static bool generate_module(module *m)
{
	if(m == NULL) return false;
	// create the directories for this module
	char *path = NULL;
	int res = asprintf(&path, "output/lib/%s", m->name);
	if(res <= 0)
		gen_error(strerror(errno));
	res = mkdir(path, 0755);
	if(res != 0 && errno != EEXIST)
		gen_error(strerror(errno));
	free(path);
	path = NULL;
	res = asprintf(&path, "output/lib/%s/include", m->name);
	if(res <= 0)
		gen_error(strerror(errno));
	res = mkdir(path, 0755);
	if(res != 0 && errno != EEXIST)
		gen_error(strerror(errno));
	free(path);
	path = NULL;
	// generate the header file first
	res = asprintf(&path, "output/lib/%s/include/%s.h", m->name, m->name);
	if(res <= 0)
		gen_error(strerror(errno));
	FILE *header = fopen(path, "w");
	if(header == NULL)
		gen_error(strerror(errno));
	
	char *modUpperName = to_upper_string(m->name);
	
	res = fprintf(header, "#ifndef %s_H\n#define %s_H\n", modUpperName, modUpperName);
	if(res <= 0)
		gen_error(strerror(errno));
	
	
	// print the includes
	res = fprintf(header, "#include <stdbool.h>\n");
	if(res <= 0)
		gen_error(strerror(errno));	
	res = fprintf(header, "#include <stdlib.h>\n");
	if(res <= 0)
		gen_error(strerror(errno));	
	
	bool use_lock = false;
	for(int i = 0; i < m->object_count; i++)
	{
		if(m->objects[i]->locked)
		{
			use_lock = true;
			break;
		}
	}
	if(use_lock)
	{
		res = fprintf(header, "#include <pthread.h>\n");
		if(res <= 0)
			gen_error(strerror(errno));	
	}
	for(int i = 0; i < m->include_count; i++)
	{
		res = fprintf(header, "#include <%s>\n", m->includes[i]);
		if(res <= 0)
			gen_error(strerror(errno));
	}
	// print the typedefs
	for(int i = 0; i < m->object_count; i++)
	{
		if(!generate_object_typedef(header, m->objects[i]))
			return false;
	}
	// print the structs
	for(int i = 0; i < m->object_count; i++)
	{
		if(!generate_object_struct(header, m->objects[i]))
			return false;
	}
	
	// print the functions definitions
	for(int i = 0; i < m->object_count; i++)
	{
		if(!generate_object_function_definitions(header, m->objects[i]))
			return false;
	}

	res = fprintf(header, "#endif /* %s_H */\n", modUpperName);
	if(res <= 0)
		gen_error(strerror(errno));
	
	free(modUpperName);
	modUpperName = NULL;

	fflush(header);
	fclose(header);
	
	free(path);
	path = NULL;
	
	// generate the implementation
	for(int i = 0; i < m->object_count; i++)
	{
		object *o = m->objects[i];
		res = asprintf(&path, "output/lib/%s/%s_%s.c", m->name, m->name, o->name);
		if(res <= 0)
			gen_error(strerror(errno));
	
		FILE *code = fopen(path, "w");
		if(code == NULL)
			gen_error(strerror(errno));
		
		res = fprintf(code, "#include <%s.h>\n", m->name);
		if(res <= 0)
			gen_error(strerror(errno));
		
		// generate the create function
		if(!generate_object_function_create(code, o)) return false;
		// generate the destroy function
		if(!generate_object_function_destroy(code, o, (o->locked && o->refcount))) return false;
		
		// generate the ref/unref functions
		if(o->locked && o->refcount)
			if(!generate_object_function_refunref(code, o)) return false;

		fflush(code);
		fclose(code);
		
		free(path);
		path = NULL;
	}
	
	return true;
}

bool generate(world *w)
{
	if(w == NULL) return false;
	
	// create the directories we need to make
	int res = mkdir("output", 0755);
	if(res != 0 && errno != EEXIST)
		gen_error(strerror(errno));
	
	res = mkdir("output/lib", 0755);
	if(res != 0 && errno != EEXIST)
		gen_error(strerror(errno));
	
	res = mkdir("output/lb", 0755);
	if(res != 0 && errno != EEXIST)
		gen_error(strerror(errno));
	
	for(int i = 0; i < w->module_count; i++)
	{
		if(!generate_module(w->modules[i])) return false;
	}
	return true;
}
