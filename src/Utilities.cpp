//
//  Utilities.cpp
//  TMEMotorwayProcessor
//
//  Created by Tom Runia on 07/02/15.
//

#include "Utilities.h"

namespace utilities
{
    //*************************************************************//
    //****************** GENERAL HELPER METHODS *******************//
    //*************************************************************//

    // Get current date/time "YYYY-MM-DD HH:mm:ss"
    string currentDateTime()
    {
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d_%X", &tstruct);
        return string(buf);
    }

    string toLowerCase(const string& inputString)
    {
        string t;
        for (string::const_iterator i = inputString.begin(); i != inputString.end(); ++i) {
            t += tolower(*i);
        }
        return t;
    }

    string getFilename(const string& fullPath)
    {
        unsigned pos = (unsigned)fullPath.find_last_of("/\\");
        string filename = fullPath.substr(pos+1);
        return filename;
    }

    string getFilenameWithoutExtension(const string& fullPath)
    {
        string withExtension = getFilename(fullPath);
        int lastDot = withExtension.find_last_of(".");
        if (lastDot == string::npos) return fullPath;
        return withExtension.substr(0, lastDot);
    }

    void getFilesInDirectory(const string& directory, vector<string>& fileNames, const vector<string>& validExtensions)
    {
        struct dirent* ep;
        size_t extensionLocation;
        DIR* dp = opendir(directory.c_str());

        if (dp != NULL) {
            while ((ep = readdir(dp))) {
                // Ignore (sub-)directories like
                if (ep->d_type & DT_DIR) {
                    continue;
                }
                extensionLocation = string(ep->d_name).find_last_of("."); // Assume the last point marks beginning of extension like file.ext
                // Check if extension is matching the wanted ones
                string tempExt = toLowerCase(string(ep->d_name).substr(extensionLocation + 1));

                // Check for matching file
                if (find(validExtensions.begin(), validExtensions.end(), tempExt) != validExtensions.end()) {
                    fileNames.push_back((string)directory + ep->d_name);
                }
            }
            (void)closedir(dp);
        }
        // Make sture the filenames are sorted
        sort(fileNames.begin(), fileNames.end());
    }

    void getFilesInDirectory(const string& directory, vector<string>& fileNames, const string& validExtension)
    {
        getFilesInDirectory(directory, fileNames, vector<string>{validExtension});
    }

    void getFoldersInDirectory(const string& directory, vector<string>& folderNames, const string& prefix)
    {
        int pre_len = prefix.length();
        const char* pre_str = prefix.c_str();

        struct dirent* entry;
        DIR* dir = opendir(directory.c_str());

        folderNames.clear();

        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && entry->d_name[0] != '.' &&
                    strncmp(entry->d_name, pre_str, pre_len) == 0) {
                folderNames.push_back(string(entry->d_name));
            }
        }
        closedir(dir);

        // Sort Folder Name
        sort(folderNames.begin(), folderNames.end());
    }
}
