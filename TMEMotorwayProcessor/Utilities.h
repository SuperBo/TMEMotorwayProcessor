//
//  Utilities.h
//  TMEMotorwayProcessor
//
//  Created by Tom Runia on 07/02/15.
//

#ifndef __TMEMotorwayProcessor__Utilities__
#define __TMEMotorwayProcessor__Utilities__

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <dirent.h>

#include <vector>
#include <map>
#include <list>
#include <string>
#include <unistd.h>

using namespace std;

namespace utilities
{
    //*************************************************************//
    //****************** GENERAL HELPER METHODS *******************//
    //*************************************************************//
    string toLowerCase(const string& inputString);
    string getFilename (const string& fullPath);
    string getFilenameWithoutExtension(const string& fullPath);
    void getFilesInDirectory(const string& directory, vector<string>& fileNames, const vector<string>& validExtensions);
    void getFilesInDirectory(const string& directory, vector<string>& fileNames, const string& validExtension);
}

#endif
