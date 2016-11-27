/*
 * Create annotation file for TME Dataset for using with Caffe-SSD
 * Author: SuperBo <supernbo@gmail.com>
 */
#include <iostream>
#include <fstream>
#include <vector>

#include "TMEMotorwayProcessor.h"

string data_path;
string anno_path;

using namespace std;

/*
 * Return label
 * 1: car
 * 2: truck
 */
int getLabel(const GTEntry& gt) {
    return (gt.isTruck()) ? 2 : 1;
}

bool createTxtAnno(const string& filename, const vector<GTEntry>& GroundTruths) {
    ofstream txtfile(filename);
    if (!txtfile.is_open()) {
        cerr << "Error while writing file " << filename << endl;
        return false;
    }

    // Loop through all of groundtruths boxes of image
    for (const GTEntry& gt : GroundTruths) {
        txtfile << getLabel(gt) << ' ';
        txtfile << gt.ip0.x << ' ' << gt.ip0.y << ' ';
        txtfile << gt.ip1.x << ' ' << gt.ip1.y << endl;
    }

    txtfile.close();

    return true;
}

int processData(TMEMotorwayProcessor& processor) {
    vector<string> sequences;
    processor.getSequences(sequences);

    ofstream listfile(data_path + "/TME_anno_list.txt");
    if (!listfile.is_open()) {
        cerr << "Error while create file " << data_path << "/TME_anno_list.txt";
        return -1;
    }

    vector<GTEntry> gts;
    CameraType cams[2] = {RIGHT, LEFT};
    for (const string& sequence: sequences) {
        for (int i = 0; i < 2; i++) {
            processor.initSequence(cams[i], sequence);
            if (!processor.isInitialized())
                continue;

            while (processor.hasNextFrame()) {
                processor.nextFrame();

                processor.getGroundTruths(gts);
                string imgfile = processor.getImageName();
                string txtfile = anno_path + '/' + getFilenameWithoutExtension(imgfile) + ".txt";

                cout << "Processing " << imgfile << endl;
                bool status = createTxtAnno(txtfile, gts);
                if (status) {
                    listfile << imgfile << ' ' << txtfile << endl;
                }
            }
        }
    }

    listfile.close();

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: create_txt_anno <path to TME dataset>" << endl;
        return 1;
    }

    data_path = string(argv[1]);
    anno_path = data_path + "/annotation";

    cout << "Data path: " << "  " << data_path << endl;

    // Initialize data processor
    TMEMotorwayProcessor processor(data_path);

    int status = processData(processor);
    if (status == 0)
        cout << "-------------------" << endl << "Finished Successfully!" << endl;

    return status;
}
