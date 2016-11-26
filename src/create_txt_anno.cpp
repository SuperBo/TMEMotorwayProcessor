/*
 * Create annotation file for TME Dataset for using with Caffe-SSD
 * Author: SuperBo <supernbo@gmail.com>
 */
#include <iostream>
#include <vector>

#include "TMEMotorwayProcessor.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: create_txt_anno <path to TME dataset>" << endl;
        return 1;
    }

    string data_path = string(argv[1]);
    cout << "Data path: " << "  " << data_path << endl;

    string anno_path = data_path + "/annotaion";
    string sequence = "tme08";

    // Initialize data processor
    TMEMotorwayProcessor processor(data_path);
    processor.initSequence(sequence);

    cout << processor.getImageDirectory() << endl;

    if (! processor.isInitialized()) {
        cout << "Error while initializing processor" << endl;
        return 1;
    }
    /*
    vector<GTEntry> gts;

    while (processor.hasNextFrame()) {
        processor.nextFrame();

        string filename = processor.getImageName();
        processor.getGroundTruths(gts);

        for (auto const& gt : gts) {
            cout << processor.getImageName() << " " << gt.isTruck() << " " << gt.ip0 << " " << gt.ip1 << endl;
        }
    }
    */

    return 0;
}
