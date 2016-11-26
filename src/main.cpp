//
//  main.cpp
//  TMEMotorwayProcessor
//
//  Created by Tom Runia on 20/02/15.
//  Copyright (c) 2015 TUDelft. All rights reserved.
//

#include <iostream>

#include "Utilities.h"
#include "TMEMotorwayProcessor.h"

void convertImagesToRGB (TMEMotorwayProcessor& processor, const string& datasetPath)
{
    // Daylight: 08 11 12 16 17 18 32 35 42
    // Sunset:   12 16 20 46

    processor.convertImagesToRGB(datasetPath + "tme08/Right/");
    // ... add other directories if you want.
}

void showFrames (TMEMotorwayProcessor& processor)
{
    if (!processor.isInitialized()) return;

    // Ground truth boxes
    vector<GTEntry> gts;

    cv::namedWindow("TME Motorway Sequence", cv::WINDOW_AUTOSIZE);
    cv::Mat image;

    while (processor.hasNextFrame()) {

        // Read next frame from sequence
        processor.readNextFrame(image, gts);

        // Draw ground truth bounding boxes
        // Note that the first ~100 frames are not annotated.
        for (auto& gt : gts) {

            // Optionally filter occluded objects, trucks or cars too far away
            //if (gt.isTruck() || gt.occluded || gt.pixelWidth() < 40) continue;

            cv::rectangle(image, gt.ip0, gt.ip1, cv::Scalar(0, 255, 255), 2);
        }

        cv::imshow("TME Motorway Sequence", image);
        cv::waitKey(5); // Set the delay between frames (ms)

        image.release();
        gts.clear();
    }
}

int main(int argc, const char * argv[]) {

    // Full path to the location of your TME Motorway Dataset
    string datasetPath = "data/Daylight";

    // Choose a sequence type (Daylight/Sunset) and a sequence number
    string sequence = "tme08";

    // Initialize data processor
    TMEMotorwayProcessor processor(datasetPath);
    processor.initSequence(sequence);

    // Show the images + ground truth bounding boxes
    showFrames(processor);

}
