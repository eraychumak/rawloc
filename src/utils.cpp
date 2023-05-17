#include "utils.h"

#include <iostream>
#include <time.h>
#include <filesystem>
#include <yyjson.h>
#include <fstream>

int getIntVal(yyjson_val* obj, const char* key) {
    yyjson_val* val = yyjson_obj_get(obj, key);
    return yyjson_get_int(val);
}

std::string getStrVal(yyjson_val* obj, const char* key) {
    yyjson_val* val = yyjson_obj_get(obj, key);
    return yyjson_get_str(val);
}

std::string newLocRow(yyjson_val* location) {
    std::string row;

    const char* timestampMs = "timestampMs";
    std::string col1 = getStrVal(location, timestampMs);
    row.append(col1);

    uint64_t secs = std::atoi(col1.c_str()) / 1000;

    std::cout << col1 << std::endl;
    std::cout << std::chrono::sys_seconds{ std::chrono::seconds(secs) } << std::endl;

    /*uint64_t i = std::stoi(col1);
    std::cout << std::chrono::sys_seconds{ std::chrono::seconds(i) } << '\n';*/

    const char* latitudeE7 = "latitudeE7";
    std::string col2 = std::to_string(getIntVal(location, latitudeE7));
    row.append("," + col2);

    const char* longitudeE7 = "longitudeE7";
    std::string col3 = std::to_string(getIntVal(location, longitudeE7));
    row.append("," + col3);

    const char* accuracy = "accuracy";
    std::string col4 = std::to_string(getIntVal(location, accuracy));
    row.append("," + col4);

    const char* source = "source";
    std::string col5 = getStrVal(location, source);
    row.append("," + col5);

    const char* deviceTag = "deviceTag";
    std::string col6 = std::to_string(getIntVal(location, deviceTag));
    row.append("," + col6);

    return row;
}