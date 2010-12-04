#include <generator.h>

using namespace generator;
using namespace std;

bool Element::genStruct(FILE *header)
{
	gen_error("A class has not declared a (struct) gen header method");
	return false;
}

bool Element::genFunctionDefs(FILE *header)
{
	gen_error("A class has not declared a (function) gen header method");
	return false;
}

bool Element::genLogic(FILE *logic)
{
	gen_error("A class has not declared a gen logic method");
	return false;
}

bool Element::genTemplate(FILE *templ)
{
	gen_error("A class has not declared a gen template method");
	return false;
}
