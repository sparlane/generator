#include <generator.h>

using namespace generator;

bool Heap::genRemoveFunctionDef(std::ostream& header)
{
	this->of()->genType(header);
	header << " " << this->module()->funcPrefix() << this->name() << "_remove(";
	this->genStruct(header, "o");
	header << ")";
	return true;
}

bool Heap::genFunctionDefs(std::ostream& header, Module *Mod)
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
	if(!this->genRemoveFunctionDef(header)) return false;
	header << ";" << std::endl;
	
	header << std::endl;
	return true;
}

bool Heap::genLogic(std::ostream& logic)
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
	if(this->haveFunctions()) logic << "#include <" << this->module()->filePrefix() << this->module()->name() << "_" << this->name() << "_logic.c>" << std::endl << std::endl;

	// Create the merge function (code)
	logic << "static ";
	this->genNodeStruct(logic, " ");
	logic << this->module()->funcPrefix() << this->name() << "_merge_r(";
	this->genNodeStruct(logic, "root");
	logic << ", ";
	this->genNodeStruct(logic, "node");
	logic << ")" << std::endl;
	logic << "{" << std::endl;
	logic << "\tif(root == NULL) return node;" << std::endl;
	logic << "\tif(node == NULL) return root;" << std::endl;
	
	if(this->haveFunctions())
	{
		logic << "\tbool cmp = " << this->module()->funcPrefix() << this->name() << "_compare_s(root->data, node->data);" << std::endl;
	}
	logic << "\t// check if it already exists" << std::endl;
	if(this->haveFunctions())
		logic << "\tif(!cmp)" << std::endl;
	else
	{
		if(this->max)
			logic << "\tif(root->idx > node->idx)" << std::endl;
		else
			logic << "\tif(root->idx > node->idx)" << std::endl;			
	}

	logic << "\t{" << std::endl;
	logic << "\t\t";
	this->genNodeStruct(logic, "t");
	logic << " = root;" << std::endl;
	logic << "\t\troot = node;" << std::endl;
	logic << "\t\tnode = t;" << std::endl;
	logic << "\t}" << std::endl;

	logic << "\troot->right = " << this->module()->funcPrefix() << this->name() << "_merge_r(root->right, node);" << std::endl;
	
	logic << "\t";
	this->genNodeStruct(logic, "t");
	logic << " = root->right;" << std::endl;
	logic << "\troot->right = root->left;" << std::endl;
	logic << "\troot->left = t;" << std::endl;
	
	logic << "\treturn root;" << std::endl;

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
	
	if(this->of()->needs_connecting())
	{
		logic << "\tif(res)" << std::endl;
		logic << "\t{" << std::endl;
		logic << "\t\tres = ";
		this->of()->print_connect(logic);
		logic << "(v);" << std::endl;
		logic << "\t}" << std::endl;
	}
	
	logic << "\tif(res)" << std::endl;
	logic << "\t\{" << std::endl;
	logic << "\t\tnN->data = v;" << std::endl;
	if(!this->haveFunctions()) logic << "\t\tnN->idx = k;" << std::endl;
	logic << "\t\tnN->parent = nN->left = nN->right = NULL;" << std::endl;
	logic << "\t}" << std::endl;
	logic << std::endl;

	logic << "\tif(res)" << std::endl;
	logic << "\t\{" << std::endl;
	logic << "\t\to->root = " << this->module()->funcPrefix() << this->name() << "_merge_r(o->root, nN);" << std::endl;
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

	if(!this->genRemoveFunctionDef(logic)) return false;
	logic << std::endl;
	logic << "{" << std::endl;
	logic << "\tif(o == NULL) return " << this->of()->initValue() << ";" << std::endl;
	logic << std::endl;
	
	if(!this->lock_code_print(logic, false)) return false;

	logic << "\t";
	this->genNodeStruct(logic, "node");
	logic << " = o->root;" << std::endl;
	logic << "\t";
	this->of()->genStruct(logic, "data");
	logic << " = " << this->of()->initValue() << ";" << std::endl;
	
	logic << "\t// check empty condition ..." << std::endl;
	logic << "\tif(node)" << std::endl;
	logic << "\t{" << std::endl;
	logic << "\t\to->root = " << this->module()->funcPrefix() << this->name() << "_merge_r(node->left, node->right);" << std::endl;
	logic << "\t\tdata = node->data;" << std::endl;
		
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

bool Heap::genTemplate(std::ostream& templ)
{
	if(this->haveFunctions())
	{
		templ << "bool " << this->module()->funcPrefix() << this->name() << "_compare_s(";
		this->of()->genStruct(templ, "first");
		templ << ", ";
		this->of()->genStruct(templ, "second");
		templ << ")" << std::endl;
		templ << "{" << std::endl;
		templ << "\tbool res;" << std::endl;
		templ << "\t//Return true if first is "  << (this->max ? " greater" : "less" ) << " than second" << std::endl;
		templ << "\t//Return false otherwise" << std::endl;
		templ << "\treturn res;" << std::endl;
		templ << "}" << std::endl;
	}
	return true;
}
