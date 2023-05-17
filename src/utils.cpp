#include "utils.h"

#include <iostream>
#include <time.h>
#include <filesystem>
#include <yyjson.h>
#include <fstream>
#include <set>
#include <vector>
#include <chrono>

std::string getYear(std::string ms)
{
    unsigned long long int secs = std::strtoull(ms.c_str(), nullptr, 0) / 1000;

    std::chrono::time_point tp = std::chrono::sys_seconds{
        std::chrono::seconds(secs)
    };

    std::string s = std::format("{:%Y}", tp);

    return s;
}

int getIntVal(yyjson_val* obj, const char* key) {
    yyjson_val* val = yyjson_obj_get(obj, key);
    return yyjson_get_int(val);
}

std::string getStrVal(yyjson_val* obj, const char* key) {
    yyjson_val* val = yyjson_obj_get(obj, key);
    return yyjson_get_str(val);
}

std::string newLocRow(std::set<std::string> headings, yyjson_val* location) {
    std::vector<std::string> row;

    for (std::string heading : headings)
    {
        yyjson_val* col = yyjson_obj_get(location, heading.c_str());

        if (!col)
        {
            row.push_back("");
            continue;
        }

        if (yyjson_is_str(col))
        {
            row.push_back(yyjson_get_str(col));
        }
        else if (yyjson_is_int(col))
        {
            row.push_back(std::to_string(yyjson_get_int(col)));
        }
        else
        {
            row.push_back("Field Not Supported");
        }
    }

    std::string csvRow;

    for (auto x = row.begin(); x != row.end(); ++x)
    {
        if (x != row.begin())
        {
            csvRow += ",";
        }

        csvRow += *x;
    }

    return csvRow;
}