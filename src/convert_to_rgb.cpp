#include <iostream>
#include "TMEMotorwayProcessor.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: convert_to_rgb <path to TME dataset>" << endl;
        return 1;
    }

    string data_path = string(argv[1]);

    TMEMotorwayProcessor processor (data_path);
    cout << "TME Dataset path: data_path" << endl;

    vector<string> seqs;
    processor.getSequences(seqs);

    CameraType cams[2] = {RIGHT, LEFT};

    for (const string& seq : seqs) {
        for (int i = 0; i < 2; i++) {
            processor.initSequence(cams[i], seq);
            if (! processor.isInitialized())
                continue;

            string imgDir = processor.getImageDirectory();
            cout << "Converting " << imgDir << " to RGB" << endl;
            processor.convertImagesToRGB(imgDir);
        }
    }

    return 0;
}
