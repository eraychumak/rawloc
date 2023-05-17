#include "utils.h"

#include <iostream>
#include <filesystem>
#include <yyjson.h>
#include <fstream>
#include <set>
#include <map>

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        std::cout << "[RAWLOC] Too many arguments.";
        return 1;
    }

    char* recordsPathArg = argv[1];
    
    std::filesystem::path recordsPath = recordsPathArg;
    std::filesystem::path parentFolder = recordsPath.parent_path();

    std::cout << "[RAWLOC] Selected records path: " << recordsPathArg << std::endl;

    FILE* fp = nullptr;
    
    yyjson_read_err err;
    yyjson_doc* doc = yyjson_read_file(recordsPathArg, NULL, NULL, &err);

   
    if (doc)
    {
        std::string csvHeadings = "timestampMs, latitudeE7, longitudeE7, accuracy, source, deviceTag";
        
        std::cout << "[RAWLOC] File opened." << std::endl;

        yyjson_val *root = yyjson_doc_get_root(doc);
        yyjson_val *locations = yyjson_obj_get(root, "locations");
        size_t locationsCount = yyjson_arr_size(locations);
        
        std::cout << "[RAWLOC] " << locationsCount << " locations found" << std::endl;

        std::map<int, std::ofstream> devices;

        size_t idx, max;
        yyjson_val* location;

        std::set<std::string> headings;

        yyjson_arr_foreach(locations, idx, max, location)
        {
            size_t idx, max;
            yyjson_val* key, * val;
            yyjson_obj_foreach(location, idx, max, key, val) {
                std::string sKey = yyjson_get_str(key);
                
                if (headings.contains(sKey))
                {
                    continue;
                }
                else
                {
                    headings.insert(sKey);
                }
            }
        }

        for (std::string heading : headings)
        {
            std::cout << heading;
        }


        yyjson_arr_foreach(locations, idx, max, location)
        {
            std::string row = newLocRow(location);
                
            int deviceTag = getIntVal(location, "deviceTag");
                
            std::filesystem::path deviceFolderPath = parentFolder;
            deviceFolderPath /= "out";
            deviceFolderPath /= std::to_string(deviceTag);

            if (devices.find(deviceTag) == devices.end())
            {
                std::cout << "[RAWLOC] New device detected, creating new folder at: " << deviceFolderPath.string() << std::endl;
                std::filesystem::create_directories(deviceFolderPath);

                std::filesystem::path recordsPath = deviceFolderPath;
                recordsPath /= "records.csv";

                devices.emplace(deviceTag, std::ofstream(recordsPath));

                std::ofstream& recordsFile = devices[deviceTag];

                if (recordsFile.is_open())
                {
                    std::cout << "[RAWLOC] Created records file for: " << deviceTag << std::endl;
                    recordsFile << csvHeadings << std::endl;
                }
                else
                {
                    std::cout << "[RAWLOC] Could not create new file." << std::endl;
                    return 0;
                }
            }

            std::ofstream& recordsFile = devices[deviceTag];
            recordsFile << row << std::endl;
        }

        for (auto const& [deviceTag, val] : devices)
        {
            std::ofstream& recordsFile = devices[deviceTag];
            std::cout << "[RAWLOC] Closing records file for device: " << deviceTag << std::endl;
            recordsFile.close();
        }
    }

    yyjson_doc_free(doc);

    return 0;
}
