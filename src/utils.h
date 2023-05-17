#include <iostream>
#include <yyjson.h>
#include <set>

std::string getYear(std::string);

int getIntVal(yyjson_val*, const char*);

std::string getStrVal(yyjson_val*, const char*);

std::string newLocRow(std::set<std::string>, yyjson_val*);