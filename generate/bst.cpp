#include <generator.h>

using namespace generator;

bool BST::create_def_print(std::ostream& f)
{
	this->genType(f);
	f << " " << this->module()->funcPrefix() << this->name() << "_create(void)";
	return true;
}

bool BST::func_def_print(std::ostream& f, std::string fname)
{
	f << "bool " << this->module()->funcPrefix() << this->name() << "_" << fname << "(";
	this->genStruct(f, "o");
	f << ")";
	return true;
}

bool BST::genInsertFunctionDef(std::ostream& header)
{
	header << "bool " << this->module()->funcPrefix() << this->name() << "_insert(";
	this->genStruct(header, "o");
	if(!this->ownfunc)
		header << ", unsigned long long k";
	header << ", ";
	this->Of->genStruct(header, "v");
	header << ")";
	return true;
}

bool BST::genFindFunctionDef(std::ostream& header)
{
	this->Of->genType(header);
	header << " " << this->module()->funcPrefix() << this->name() << "_find(";
	this->genStruct(header, "o");
	header << ", ";
	if(this->ownfunc)
		this->Of->genStruct(header, "v");
	else
		header << "unsigned long long k";
	header << ")";
	return true;
}

bool BST::genRemoveFunctionDef(std::ostream& header)
{
	this->Of->genType(header);
	header << " " << this->module()->funcPrefix() << this->name() << "_remove(";
	this->genStruct(header, "o");
	header << ", ";
	if(this->ownfunc)
		this->Of->genStruct(header, "v");
	else
		header << "unsigned long long k";
	header << ")";
	return true;
}

bool BST::genNodeStruct(std::ostream& header, std::string name)
{
	header << "struct " << this->module()->funcPrefix() << this->name() << "_node_s *" << name;
	return true;
}

bool BST::create_func_print(std::ostream& f)
{
	f << " /* Create a " << this->name() << " */" << std::endl;
	f << "{" << std::endl;
	f << "\t";
	this->genStruct(f, "o");
	f << " = calloc(1, sizeof(struct ";
	this->genType(f);
	f << "_s));" << std::endl;
	f << "\tif(o == NULL) goto ERROR_EARLY;\n" << std::endl;
	
	if(!nolock)
	{
		f << "\tpthread_mutexattr_t mutex_attr;\n\tif(pthread_mutexattr_init(&mutex_attr) != 0) goto ERROR_LOCK;" << std::endl;
		f << "\tif(pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK) != 0) goto ERROR_LOCK;" << std::endl;
		f << "\tif(pthread_mutex_init(&o->lock, &mutex_attr) != 0) goto ERROR_LOCK;\n" << std::endl;
		
		if(!noref)
		{
			f << "\to->references = 1;" << std::endl;
			f << std::endl;
		}
	}
	
	f << "\treturn o;" << std::endl;
	
	if(!nolock)
	{
		f << "ERROR_LOCK:" << std::endl;
		f << "\tfree(o);" << std::endl; 
	}
	f << "ERROR_EARLY:" << std::endl;
	f << "\treturn NULL;" << std::endl;
	f << "}" << std::endl;
	f << std::endl;
	return true;
}

bool BST::destroy_func_print(std::ostream& f)
{
	f << " /* Destroy a " << this->name() << " */" << std::endl;
	f << "{" << std::endl;
	f << "\tif(o == NULL) return false;" << std::endl;
	f << std::endl;
	
	f << "\tbool res = true;" << std::endl;

	if(noref)
	{
		if(!lock_code_print(f, false)) return false;
	}

	if(!unlock_code_print(f)) return false;
	if(!destroy_lock_code_print(f)) return false;
	
	f << "\tfree(o);" << std::endl;
	f << "\treturn res;" << std::endl;
	f << "}" << std::endl;
	f << std::endl;
	return true;
}

bool BST::ref_func_print(std::ostream& f)
{
	f << " /* Reference a " << this->name() << " */" << std::endl;
	f << "{" << std::endl;
	if(!lock_code_print(f, false)) return false;
	f << "\to->references++;" << std::endl;
	f << std::endl;
	if(!unlock_code_print(f)) return false;
	f << "\treturn true;" << std::endl;
	f << "}" << std::endl;
	f << std::endl;
	return true;
}

bool BST::unref_func_print(std::ostream& f)
{
	f << " /* Unreference a " << this->name() << " */" << std::endl;
	f << "{" << std::endl;
	if(!lock_code_print(f, false)) return false;
	f << "\to->references--;" << std::endl;
	f << std::endl;
	f << "\tif(o->references == 0) return " << this->module()->funcPrefix() << this->name() << "_destroy(o);" << std::endl;
	f << std::endl;
	if(!unlock_code_print(f)) return false;
	f << "\treturn true;" << std::endl;
	f << "}" << std::endl;
	f << std::endl;
	return true;
}

bool BST::genStruct(std::ostream& header)
{
	header << "struct " << this->module()->funcPrefix() << this->name() << "_node_s {" << std::endl;
	header << "\t";
	if(!this->Of->genStruct(header, "data")) return false;
	header << ";" << std::endl;
	if(!this->ownfunc)
		header << "\tunsigned long long idx;" << std::endl;
	header << "\tstruct " << this->module()->funcPrefix() << this->name() << "_node_s *parent;" << std::endl;
	header << "\tstruct " << this->module()->funcPrefix() << this->name() << "_node_s *left;" << std::endl;
	header << "\tstruct " << this->module()->funcPrefix() << this->name() << "_node_s *right;" << std::endl;
	header << "};" << std::endl << std::endl;

	header << "struct " << this->module()->funcPrefix() << this->name() << "_s {" << std::endl;

	header << "\tbool destroyed;" << std::endl;
	if(!this->nolock)
	{
		header << "\tpthread_mutex_t lock;" << std::endl;
		if(!noref)
		{
			header << "\tunsigned int references;" << std::endl;
		}
	}
	
	header << "\tstruct " << this->module()->funcPrefix() << this->name() << "_node_s *root;" << std::endl;
	header << "};" << std::endl << std::endl;

	return true;
}

bool BST::genFunctionDefs(std::ostream& header, Module *Mod)
{
	// Constructor
	if(!create_def_print(header)) return false;
	header << " __attribute__((__warn_unused_result__));" << std::endl;
	if(!nolock && !noref)
	{ // Reference counters
		if(!func_def_print(header, "ref")) return false;
		header << " __attribute__((__warn_unused_result__));" << std::endl;
		if(!func_def_print(header, "unref")) return false;
		header << " __attribute__((__warn_unused_result__));" << std::endl;
	} else { // Destructor
		if(!func_def_print(header, "destroy")) return false;
		header << " __attribute__((__warn_unused_result__));" << std::endl;
	}
	if(!this->genInsertFunctionDef(header)) return false;
	header << ";" << std::endl;
	if(!this->genFindFunctionDef(header)) return false;
	header << ";" << std::endl;
	if(!this->genRemoveFunctionDef(header)) return false;
	header << ";" << std::endl;
	
	header << std::endl;
	return true;
}

bool BST::genLogic(std::ostream& logic)
{
	// Generate the constructor
	if(!create_def_print(logic)) return false;
	if(!create_func_print(logic)) return false;
	
	// Generate the destructor
	if(!nolock && !noref)
		logic << "static ";
	if(!func_def_print(logic, "destroy")) return false;
	if(!destroy_func_print(logic)) return false;
	if(!nolock && !noref)
	{
		if(!func_def_print(logic, "ref")) return false;
		if(!ref_func_print(logic)) return false;
	
		if(!func_def_print(logic, "unref")) return false;
		if(!unref_func_print(logic)) return false;
	}
	
	// include the logic file they need to generate
	if(ownfunc) logic << "#include <" << this->module()->filePrefix() << this->module()->name() << "_" << this->name() << "_logic.c>" << std::endl << std::endl;

	
	// Create the add function
	logic << "static bool " << this->module()->funcPrefix() << this->name() << "_add_r(";
	this->genNodeStruct(logic, "root");
	logic << ", ";
	this->genNodeStruct(logic, "node");
	logic << ") __attribute((ownership_holds(malloc, 2)));" << std::endl;
	logic << "static bool " << this->module()->funcPrefix() << this->name() << "_add_r(";
	this->genNodeStruct(logic, "root");
	logic << ", ";
	this->genNodeStruct(logic, "node");
	logic << ")" << std::endl;
	logic << "{" << std::endl;
	logic << "\tif(root == NULL || node == NULL) return false;" << std::endl;
	
	if(ownfunc)
	{
		logic << "\tint cmp = " << this->module()->funcPrefix() << this->name() << "_compare_s(root->data, node->data);" << std::endl;
	}
	logic << "\t// check if it already exists" << std::endl;
	if(ownfunc)
		logic << "\tif(cmp == 0) return false;" << std::endl;
	else
		logic << "\tif(root->idx == node->idx) return false;" << std::endl;
	
	if(ownfunc)
		logic << "\tif(cmp < 0)" << std::endl;
	else
		logic << "\tif(node->idx < root->idx)" << std::endl;
	
	logic << "\t{" << std::endl;
	logic << "\t\t// go left" << std::endl;
	logic << "\t\tif(root->left) return " << this->module()->funcPrefix() << this->name() << "_add_r(root->left, node);" << std::endl;
	logic << "\t\troot->left = node;" << std::endl;
	logic << "\t\tnode->parent = root;" << std::endl;
	logic << "\t\treturn true;" << std::endl;
	logic << "\t} else {" << std::endl;
	logic << "\t\t// go right" << std::endl;
	logic << "\t\tif(root->right) return " << this->module()->funcPrefix() << this->name() << "_add_r(root->right, node);" << std::endl;
	logic << "\t\troot->right = node;" << std::endl;
	logic << "\t\tnode->parent = root;" << std::endl;
	logic << "\t\treturn true;" << std::endl;
	logic << "\t}" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;
	
	
	logic << "static struct " << this->module()->funcPrefix() << this->name() << "_node_s *"  << this->module()->funcPrefix() << this->name() << "_find_r(";
	this->genNodeStruct(logic, "node");
	logic << ", ";
	if(this->ownfunc)
		this->Of->genStruct(logic, "v");
	else
		logic << "unsigned long long k";
	logic << ")" << std::endl;

	logic << "{" << std::endl;
	logic << "\t// input checking ..." << std::endl;
	logic << "\tif(node == NULL) return NULL;" << std::endl;
	
	if(ownfunc)
	{
		logic << "\tint cmp = " << this->module()->funcPrefix() << this->name() << "_compare_s(node->data, v);" << std::endl;
	}
	logic << "\t// is this it ?" << std::endl;
	if(ownfunc)
		logic << "\tif(cmp == 0) return node;" << std::endl;
	else
		logic << "\tif(node->idx == k) return node;" << std::endl;
	
	logic << "\t// search the children ..." << std::endl;
	logic << "\treturn " << this->module()->funcPrefix() << this->name() << "_find_r(";
	if(ownfunc)
		logic << "(cmp < 0)";
	else
		logic << "(k < node->idx)";
	logic << "? node->left : node->right, " << (this->ownfunc ? "v" : "k") << ");" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;
	
	logic << "static void " << this->module()->funcPrefix() << this->name() << "_add_t(";
	this->genStruct(logic, "bst");
	logic << ", ";
	this->genNodeStruct(logic, "node");
	logic << ")" << std::endl;
	logic << "{" << std::endl;
	logic << "\t// input checking ..." << std::endl;
	logic << "\tif(bst == NULL || node == NULL) return;" << std::endl;
	logic << std::endl;
	logic << "\t// grab the left and right children" << std::endl;
	logic << "\t";
	this->genNodeStruct(logic, "left");
	logic << " = node->left;" << std::endl;
	logic << "\t";
	this->genNodeStruct(logic, "right");
	logic << " = node->right;" << std::endl;
	
	logic << "\tnode->left = node->right = node->parent = NULL;" << std::endl;
	
	logic << "\t// check if the root is empty" << std::endl;
	logic << "\tif(!bst->root) bst->root = node;" << std::endl;
	logic << "\telse " << this->module()->funcPrefix() << this->name() << "_add_r(bst->root, node);" << std::endl;
	logic << std::endl;
	logic << "\t// do the same for the children (trees)" << std::endl;
	logic << "\tif(left != NULL) " << this->module()->funcPrefix() << this->name() << "_add_t(bst, left);" << std::endl;
	logic << "\tif(right != NULL) " << this->module()->funcPrefix() << this->name() << "_add_t(bst, right);" << std::endl;
	logic << "}" << std::endl;
	logic << std::endl;

	
	// Generate each of our (externally accessible) functions
	if(!this->genInsertFunctionDef(logic)) return false;
	logic << std::endl;
	logic << "{" << std::endl;
	logic << "\tif(o == NULL) return false;" << std::endl;
	logic << std::endl;
	
	logic << "\tbool res = true;" << std::endl;

	if(!this->lock_code_print(logic, false)) return false;

	logic << "\t";
	this->genNodeStruct(logic, "nN");
	logic << " = malloc(sizeof(struct " << this->module()->funcPrefix() << this->name() << "_node_s";
	logic << "));" << std::endl;
	logic << "\tif(nN == NULL) res = false;" << std::endl;
	logic << std::endl;
	
	if(this->Of->needs_connecting())
	{
		logic << "\tif(res)" << std::endl;
		logic << "\t{" << std::endl;
		logic << "\t\tres = ";
		this->Of->print_connect(logic);
		logic << "(v);" << std::endl;
		logic << "\t}" << std::endl;
	}
	
	logic << "\tif(res)" << std::endl;
	logic << "\t\{" << std::endl;
	logic << "\t\tnN->data = v;" << std::endl;
	if(!ownfunc) logic << "\t\tnN->idx = k;" << std::endl;
	logic << "\t\tnN->parent = nN->left = nN->right = NULL;" << std::endl;
	logic << "\t}" << std::endl;
	logic << std::endl;

	logic << "\tif(res)" << std::endl;
	logic << "\t\{" << std::endl;
	logic << "\t\tif(o->root == NULL)" << std::endl;
	logic << "\t\t{" << std::endl;
	logic << "\t\t\to->root = nN;" << std::endl;
	logic << "\t\t} else {" << std::endl;
	logic << "\t\t\tres = " << this->module()->funcPrefix() << this->name() << "_add_r(o->root, nN);" << std::endl;
	logic << "\t\t}" << std::endl;
	logic << "\t}" << std::endl;
	logic << std::endl;

	logic << "\tif(!res)" << std::endl;
	logic << "\t{" << std::endl;
	logic << "\t\tif(nN != NULL) free(nN);" << std::endl;
	logic << "\t}" << std::endl;
	logic << std::endl;

	if(!this->unlock_code_print(logic)) return false;
	
	logic << "\treturn res;" << std::endl;

	logic << "}" << std::endl;
	logic << std::endl;

	if(!this->genFindFunctionDef(logic)) return false;
	logic << std::endl;
	logic << "{" << std::endl;
	logic << "\tif(o == NULL) return false;" << std::endl;
	logic << std::endl;
	
	if(!this->lock_code_print(logic, false)) return false;

	logic << "\t";
	this->genNodeStruct(logic, "node");
	logic << " = NULL;" << std::endl;
	logic << "\t";
	this->Of->genStruct(logic, "data");
	logic << " = " << this->Of->initValue() << ";" << std::endl;
	logic << std::endl;

	logic << "\t// check empty condition ..." << std::endl;
	logic << "\tif(o->root != NULL) node = " << this->module()->funcPrefix()  << this->name() << "_find_r(o->root, " << (this->ownfunc ? "v" : "k" ) << ");" << std::endl;
	logic << "\tif(node != NULL) data = node->data;" << std::endl;

	logic << std::endl;

	if(this->Of->needs_connecting())
	{
		logic << "\tif(data != " << this->Of->initValue() << ")" << std::endl;
		logic << "\t\tif(!";
		this->Of->print_connect(logic);
		logic << "(data)) data = " << this->Of->initValue() << ";" << std::endl;
	}

	if(!this->unlock_code_print(logic)) return false;
	
	logic << "\treturn data;" << std::endl;

	logic << "}" << std::endl;
	logic << std::endl;
	
	if(!this->genRemoveFunctionDef(logic)) return false;
	logic << std::endl;
	logic << "{" << std::endl;
	logic << "\tif(o == NULL) return " << this->Of->initValue() << ";" << std::endl;
	logic << std::endl;
	
	if(!this->lock_code_print(logic, false)) return false;

	logic << "\t";
	this->genNodeStruct(logic, "node");
	logic << " = NULL;" << std::endl;
	logic << "\t";
	this->Of->genStruct(logic, "data");
	logic << " = " << this->Of->initValue() << ";" << std::endl;
	
	logic << "\t// check empty condition ..." << std::endl;
	logic << "\tif(o->root != NULL) node = " << this->module()->funcPrefix() << this->name() << "_find_r(o->root, " << (this->ownfunc ? "v" : "k") << ");" << std::endl;
	logic << "\tif(node)" << std::endl;
	logic << "\t{" << std::endl;
	logic << "\t\tbool doLeft = true;" << std::endl;
	logic << "\t\tbool doRight = true;" << std::endl;
	logic << "\t\tdata = node->data;" << std::endl;
	logic << "\t\t// remove the node from the tree, this is tricky ..." << std::endl;
	logic << "\t\t// check if its the root node ..." << std::endl;
	logic << "\t\tif(node->parent == NULL)" << std::endl;
	logic << "\t\t{" << std::endl;
	logic << "\t\t\to->root = NULL;" << std::endl;
	logic << "\t\t\tif(node->left)" << std::endl;
	logic << "\t\t\t{" << std::endl;
	logic << "\t\t\t\t// has a left child, so make that the new root" << std::endl;
	logic << "\t\t\t\to->root = node->left;" << std::endl;
	logic << "\t\t\t\to->root->parent = NULL;" << std::endl;
	logic << "\t\t\t\t// now we only need to re-add the right" << std::endl;
	logic << "\t\t\t\tdoLeft = false;" << std::endl;
	logic << "\t\t\t}" << std::endl;
	logic << "\t\t\telse if(node->right)" << std::endl;
	logic << "\t\t\t{" << std::endl;
	logic << "\t\t\t\t// has only a right child, so make that the new root" << std::endl;
	logic << "\t\t\t\to->root = node->right;" << std::endl;
	logic << "\t\t\t\to->root->parent = NULL;" << std::endl;
	logic << "\t\t\t\t// now we don't have to re-add anything" << std::endl;
	logic << "\t\t\t\tdoLeft = doRight = false;" << std::endl;
	logic << "\t\t\t}" << std::endl;
	logic << "\t\t} else {" << std::endl;
	logic << "\t\t\t// figure out which child it is" << std::endl;
	logic << "\t\t\tif(node->parent->left == node)" << std::endl;
	logic << "\t\t\t{" << std::endl;
	logic << "\t\t\t\t// left" << std::endl;
	logic << "\t\t\t\t// automatically connect the left sub-tree here" << std::endl;
	logic << "\t\t\t\tnode->parent->left = node->left;" << std::endl;
	logic << "\t\t\t\tif(node->left) node->left->parent = node->parent;" << std::endl;
	logic << "\t\t\t\tdoLeft = false;" << std::endl;
	logic << "\t\t\t} else {" << std::endl;
	logic << "\t\t\t\t// right" << std::endl;
	logic << "\t\t\t\t// automatically connect the right sub-tree here" << std::endl;
	logic << "\t\t\t\tnode->parent->right = node->right;" << std::endl;
	logic << "\t\t\t\tif(node->right) node->right->parent = node->parent;" << std::endl;
	logic << "\t\t\t\tdoRight = false;" << std::endl;
	logic << "\t\t\t}" << std::endl;
	logic << "\t\t}" << std::endl;
	logic << "\t\t// now add any children back into the tree." << std::endl;
	logic << "\t\tif(doLeft) " << this->module()->funcPrefix() << this->name() << "_add_t(o, node->left);" << std::endl;
	logic << "\t\tif(doRight) " << this->module()->funcPrefix() << this->name() << "_add_t(o, node->right);" << std::endl;
		
	logic << "\t\t// now free the node" << std::endl;
	logic << "\t\tfree(node);" << std::endl;
	logic << "\t}" << std::endl;
	logic << std::endl;

	if(!this->unlock_code_print(logic)) return false;
	
	logic << "\treturn data;" << std::endl;

	logic << "}" << std::endl;
	logic << std::endl;


	return true;
}

bool BST::genTemplate(std::ostream& templ)
{
	if(ownfunc)
	{
		templ << "int " << this->module()->funcPrefix() << this->name() << "_compare_s(";
		this->Of->genStruct(templ, "first");
		templ << ", ";
		this->Of->genStruct(templ, "second");
		templ << ")" << std::endl;
		templ << "{" << std::endl;
		templ << "\tint res;" << std::endl;
		templ << "\t//Return 0 if these are the same" << std::endl;
		templ << "\t//Return a negative value if first comes before second" << std::endl;
		templ << "\t//Return a postive value if first comes after second" << std::endl;
		templ << "\treturn res;" << std::endl;
		templ << "}" << std::endl;
	}
	return true;
}

bool BST::genType(std::ostream& header)
{
	header << this->module()->funcPrefix() << this->name();
	return true;
}

bool BST::genTypeDef(std::ostream& header)
{
	header << "typedef struct " << this->module()->funcPrefix() << this->name() << "_s *" << this->module()->funcPrefix() << this->name() << ";" << std::endl;
	return true;
}

bool BST::genStruct(std::ostream& header, std::string name)
{
	header << this->module()->funcPrefix() << this->name() << " " << name;
	return true;
}
