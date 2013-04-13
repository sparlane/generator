#include <generator.h>

using namespace generator;

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
	if(this->haveFunctions()) logic << "#include <" << this->module()->filePrefix() << this->module()->name() << "_" << this->name() << "_logic.c>" << std::endl << std::endl;

	
	// Create the add function
	logic << "static bool " << this->module()->funcPrefix() << this->name() << "_add_r(";
	this->genNodeStruct(logic, "root");
	logic << ", ";
	this->genNodeStruct(logic, "node");
	logic << ");" << std::endl;
	logic << "static bool " << this->module()->funcPrefix() << this->name() << "_add_r(";
	this->genNodeStruct(logic, "root");
	logic << ", ";
	this->genNodeStruct(logic, "node");
	logic << ")" << std::endl;
	logic << "{" << std::endl;
	logic << "\tif(root == NULL || node == NULL) return false;" << std::endl;
	
	if(this->haveFunctions())
	{
		logic << "\tint cmp = " << this->module()->funcPrefix() << this->name() << "_compare_s(root->data, node->data);" << std::endl;
	}
	logic << "\t// check if it already exists" << std::endl;
	if(this->haveFunctions())
		logic << "\tif(cmp == 0) return false;" << std::endl;
	else
		logic << "\tif(root->idx == node->idx) return false;" << std::endl;
	
	if(this->haveFunctions())
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
	if(this->haveFunctions())
		this->of()->genStruct(logic, "v");
	else
		logic << "unsigned long long k";
	logic << ")" << std::endl;

	logic << "{" << std::endl;
	logic << "\t// input checking ..." << std::endl;
	logic << "\tif(node == NULL) return NULL;" << std::endl;
	
	if(this->haveFunctions())
	{
		logic << "\tint cmp = " << this->module()->funcPrefix() << this->name() << "_compare_s(node->data, v);" << std::endl;
	}
	logic << "\t// is this it ?" << std::endl;
	if(this->haveFunctions())
		logic << "\tif(cmp == 0) return node;" << std::endl;
	else
		logic << "\tif(node->idx == k) return node;" << std::endl;
	
	logic << "\t// search the children ..." << std::endl;
	logic << "\treturn " << this->module()->funcPrefix() << this->name() << "_find_r(";
	if(this->haveFunctions())
		logic << "(cmp < 0)";
	else
		logic << "(k < node->idx)";
	logic << "? node->left : node->right, " << (this->haveFunctions() ? "v" : "k") << ");" << std::endl;
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
	this->of()->genStruct(logic, "data");
	logic << " = " << this->of()->initValue() << ";" << std::endl;
	logic << std::endl;

	logic << "\t// check empty condition ..." << std::endl;
	logic << "\tif(o->root != NULL) node = " << this->module()->funcPrefix()  << this->name() << "_find_r(o->root, " << (this->haveFunctions() ? "v" : "k" ) << ");" << std::endl;
	logic << "\tif(node != NULL) data = node->data;" << std::endl;

	logic << std::endl;

	if(this->of()->needs_connecting())
	{
		logic << "\tif(data != " << this->of()->initValue() << ")" << std::endl;
		logic << "\t\tif(!";
		this->of()->print_connect(logic);
		logic << "(data)) data = " << this->of()->initValue() << ";" << std::endl;
	}

	if(!this->unlock_code_print(logic)) return false;
	
	logic << "\treturn data;" << std::endl;

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
	logic << " = NULL;" << std::endl;
	logic << "\t";
	this->of()->genStruct(logic, "data");
	logic << " = " << this->of()->initValue() << ";" << std::endl;
	
	logic << "\t// check empty condition ..." << std::endl;
	logic << "\tif(o->root != NULL) node = " << this->module()->funcPrefix() << this->name() << "_find_r(o->root, " << (this->haveFunctions() ? "v" : "k") << ");" << std::endl;
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
	if(this->haveFunctions())
	{
		templ << "int " << this->module()->funcPrefix() << this->name() << "_compare_s(";
		this->of()->genStruct(templ, "first");
		templ << ", ";
		this->of()->genStruct(templ, "second");
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
