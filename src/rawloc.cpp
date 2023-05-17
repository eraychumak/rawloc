#include "utils.h"

#include <iostream>
#include <filesystem>
#include <yyjson.h>
#include <fstream>
#include <set>
#include <map>

std::set<std::string> discoverLocationFields(yyjson_val* locations)
{
    std::set<std::string> headings;

    size_t idxLoc, maxLoc;
    yyjson_val* location;

    // discover all potential fields
    yyjson_arr_foreach(locations, idxLoc, maxLoc, location)
    {
        size_t idxKey, maxKey;
        yyjson_val* key, * val;
        
        yyjson_obj_foreach(location, idxKey, maxKey, key, val) {
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

    return headings;
}

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        std::cout << "[RAWLOC] Too many arguments given" << std::endl;
        return 0;
    }

    if (argc == 1)
    {
        std::cout << "[RAWLOC] Please specify a path to the records file." << std::endl;
        return 0;
    }

    char* recordsPathArg = argv[1];
    
    std::filesystem::path recordsPath = recordsPathArg;
    std::filesystem::path parentFolder = recordsPath.parent_path();

    std::cout << "[RAWLOC] Selected records at: " << recordsPathArg << std::endl;

    FILE* fp = nullptr;
    
    yyjson_read_err err;
    yyjson_doc* doc = yyjson_read_file(recordsPathArg, NULL, NULL, &err);

   
    if (doc)
    {
        std::cout << "[RAWLOC] Opening records file" << std::endl;

        yyjson_val *root = yyjson_doc_get_root(doc);
        yyjson_val *locations = yyjson_obj_get(root, "locations");
        size_t locationsCount = yyjson_arr_size(locations);
        
        std::cout << "[RAWLOC] " << locationsCount << " locations found" << std::endl;

        std::set<int> devices;
        std::map<std::string, std::ofstream> deviceYears;

        std::set<std::string> headings = discoverLocationFields(locations);

        std::cout << "[RAWLOC] Discovered " << headings.size() << " different fields for a location" << std::endl;
        std::string headingsRow;
        
        // format discovered fields into a csv heading string
        for (auto x = headings.begin(); x != headings.end(); ++x)
        {
            if (x != headings.begin())
            {
                headingsRow += ",";
            }
         
            headingsRow += *x;
        }

        size_t idx, max;
        yyjson_val* location;

        // iterate over each location and write to its relevant file.
        yyjson_arr_foreach(locations, idx, max, location)
        {
            std::string row = newLocRow(headings, location);

            int deviceTag = getIntVal(location, "deviceTag");
            std::string year;

            yyjson_val* timestampMs = yyjson_obj_get(location, "timestampMs");

            if (timestampMs)
            {
                std::string strVal = yyjson_get_str(timestampMs);
                year = getYear(strVal);
            }
            else
            {
                year = "noYear";
            }

            std::filesystem::path deviceFolderPath = parentFolder;
            deviceFolderPath /= "DevicesLocationHistoryCSVs";
            deviceFolderPath /= std::to_string(deviceTag);

            // create new folder for new device tags
            if (devices.find(deviceTag) == devices.end())
            {
                std::cout << "[RAWLOC] New device discovered, creating new folder for it at: " << deviceFolderPath.string() << std::endl;
                std::filesystem::create_directories(deviceFolderPath);
                devices.insert(deviceTag);
            }

            std::string deviceYearID = std::to_string(deviceTag) + "-" + year;

            // create new file for discovered year in the device folder
            if (deviceYears.find(deviceYearID) == deviceYears.end())
            {
                std::cout << "[RAWLOC] New year '" << year << "' discovered for device '" << deviceTag << "'" << std::endl;
                
                std::filesystem::path yearRecordsPath = deviceFolderPath;
                yearRecordsPath /= year + ".csv";

                deviceYears.emplace(deviceYearID, std::ofstream(yearRecordsPath));

                std::ofstream& recordsFile = deviceYears[deviceYearID];

                if (recordsFile.is_open())
                {
                    std::cout << "[RAWLOC] Created records file for device '" << deviceTag << "' at: " << yearRecordsPath.string() << std::endl;
                    recordsFile << headingsRow << std::endl;
                }
                else
                {
                    std::cout << "[RAWLOC] Could not create records file for deviceTag '" << deviceTag << "'" << std::endl;
                    return 0;
                }
            }

            std::ofstream& recordsFile = deviceYears[deviceYearID];
            recordsFile << row << std::endl;
        }

        // close all opened files for devices
        for (auto const& [deviceYearID, val] : deviceYears)
        {
            std::ofstream& recordsFile = deviceYears[deviceYearID];
            std::cout << "[RAWLOC] Closing file ID: '" << deviceYearID << "'" << std::endl;
            recordsFile.close();
        }
        
        yyjson_doc_free(doc);

        std::cout << "[RAWLOC] Done" << std::endl;
    }
    else {
        std::cout << "[RAWLOC] Could not find the file by the path specified" << std::endl;
    }

    return 0;
}
