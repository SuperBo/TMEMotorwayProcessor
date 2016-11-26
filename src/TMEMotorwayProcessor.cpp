//
//  TMEMotorwayProcessor.cpp
//  TMEMotorwayProcessor
//
//  Created by Tom Runia on 16/02/15.
//

#include "TMEMotorwayProcessor.h"

TMEMotorwayProcessor::TMEMotorwayProcessor(const string& _datasetPath)
{
    datasetPath = _datasetPath + "/";
}

void TMEMotorwayProcessor::initSequence(const string& _sequence)
{
    initSequence(DAYLIGHT, _sequence);
}

void TMEMotorwayProcessor::initSequence(const SequenceType &_sequenceType, const string &_sequence)
{
    sequenceType =  _sequenceType;
    sequence =      _sequence;

    string imageDirectory = getImageDirectory();
    getFilesInDirectory(imageDirectory, imageFiles, "png");

    if (imageFiles.empty()) {
        cout << "Unable to read sequence...EXIT" << endl;
        return;
    }

    // Read calibration parameters (now hardcoded, but can be read from calibration.ini)
    readCalibrationParameters();

    // Read ground truth annotations from file and convert the bounding boxes to screen coordinates
    readGroundTruths();

    currentFrame = -1; // Ground truths are available from frame 80
}

void TMEMotorwayProcessor::readCalibrationParameters(const string &calibrationFile)
{
    // Since there is only one calibration file, for now just use hard coded values.
    calibration.x = -1.70; // POS X_M
    calibration.y = -0.60; // POS Y_M
    calibration.z =  1.28; // POS Z_M

    calibration.h_fov_deg = 32.1; // H_FOV_DEF

    calibration.pitch_deg =  0.2; // POS PITCH_DEG
    calibration.yaw_deg =   -4.0; // POS YAW_DEF
    calibration.roll_deg =   0.0; // POS ROLL_DEG

    calibration.image_width =  1024;
    calibration.image_height = 768;
}

void TMEMotorwayProcessor::readGroundTruths()
{
    groundTruths.clear();
    string line;
    int gtCount = 0;

    // There are 2 GT files: Daylight and Sunset
    string gtFilename = getGroundTruthFilename();
    ifstream gtFile (gtFilename);

    if (!gtFile.is_open()) {
        cout << "Unable to read ground truth file." << endl;
        return;
    }

    while (getline(gtFile, line)) {

        // We only need to read out ground truths for the current sequence
        if (line.substr(0,5) != sequence) continue;

        // Use the frame number as key for saving ground truths
        string frameNr = line.substr(6, 6);
        frameNr.erase(0, frameNr.find_first_not_of('0')); // remove preceding zeros

        // Rest of the line containing ground truth boxes
        line = line.substr(13);
        boost::trim(line);

        // Split line on ; character to read all ground truths in the image
        vector<string> gtLineParts;
        boost::split(gtLineParts, line, boost::is_any_of(";"), boost::token_compress_on);

        // Frame ground truths
        vector<GTEntry> frameGts;

        for (auto& gtLinePart : gtLineParts) {
            // Split part of line on space to read out detection values

            vector<string> gtValues;
            boost::trim(gtLinePart);
            boost::split(gtValues, gtLinePart, boost::is_any_of("\t "), boost::token_compress_on);

            // Ground truth detection contains seven values
            if (gtValues.size() != 7) continue;

            // Fill values of GroundTruth object
            GTEntry gt;
            gt.ID =     stoi(gtValues[0]);
            gt.wp0.x =  stof(gtValues[1]);
            gt.wp0.y =  stof(gtValues[2]);
            gt.wp1.x =  stof(gtValues[3]);
            gt.wp1.y =  stof(gtValues[4]);
            gt.reconstructed =  stoi(gtValues[5]) == 1 ? true : false;
            gt.estimatedWidth = stof(gtValues[6]);

            // Save ground truth for this frame
            frameGts.push_back(gt);

            gtCount++;
        }

        groundTruths[stoi(frameNr)] = frameGts;
    }

    // Conversion of world coordinates to screen coordinates
    // This conversion is done using parameters from calibration.ini
    computeScreenCoordinates();

    gtFile.close();
}

void TMEMotorwayProcessor::computeScreenCoordinates()
{
    static const float near = 1.0;
    static const float far =  100;

    GLint m_viewPort[4];
    GLdouble m_modelViewMatrix[16], m_projMatrix[16];

    // Create hidden OpenGL window (just for its context)
    cv::namedWindow("OpenGL Context", cv::WINDOW_OPENGL);

    glViewport(0, 0, calibration.image_width, calibration.image_height);
    glGetIntegerv(GL_VIEWPORT, m_viewPort);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(calibration.h_fov_deg, double(calibration.image_width) / double(calibration.image_height), near, far);
    glGetDoublev(GL_PROJECTION_MATRIX, m_projMatrix);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0,0.0,0.0, 1.0,0.0,0.0, 0.0,0.0,1.0);

    // This is the only order that works correctly.
    glRotated(-calibration.roll_deg, 1.0, 0.0, 0.0);
    glRotated(-calibration.pitch_deg, 0.0, 1.0, 0.0);
    glRotated(-calibration.yaw_deg, 0.0, 0.0, 1.0);

    glTranslated(-calibration.x, -calibration.y, -calibration.z); // z = height
    glGetDoublev(GL_MODELVIEW_MATRIX, m_modelViewMatrix);

    double z;

    for(auto &it : groundTruths) {
        for (auto& gt : it.second) { // 'it.second' is vector<GTEntry> containing GTs for one frame
            cv::Point2d pointLeft, pointRight;

            // Convert first (LEFT) point to screen coordinates
            gluProject(gt.wp0.x, gt.wp1.y, 0.0, m_modelViewMatrix, m_projMatrix, m_viewPort, &(pointLeft.x), &(pointLeft.y), &z);
            pointLeft.y = calibration.image_height - pointLeft.y;

            // Convert second (RIGHT) point to screen coordinates
            gluProject(gt.wp0.x, gt.wp0.y, 0.0, m_modelViewMatrix, m_projMatrix, m_viewPort, &(pointRight.x), &(pointRight.y), &z);
            pointRight.y = calibration.image_height - pointRight.y;

            gt.ip0 = pointLeft;
            gt.ip1 = pointRight;

            const double pixelHeight = gt.assignedHeight() * gt.meterToPixel();
            gt.ip0.y -= pixelHeight;

            // Check whether bounding box is truncated (out-of-screen)
            gt.truncated = ((gt.ip0.x < 0 && 1.0 - abs(gt.ip0.x) / gt.pixelWidth() < TRUNCATED_TH) ||
                            (gt.ip1.x > calibration.image_width && 1.0 - (gt.ip1.x - calibration.image_width) / gt.pixelWidth() < TRUNCATED_TH) );

            // Check whether bounding box is occluded
            gt.occluded = (isOccluded(gt, it.second)) ? true : false;

        }
    }

    cv::destroyWindow("OpenGL Context");
}

bool TMEMotorwayProcessor::isOccluded (const GTEntry& gt, const vector<GTEntry>& otherGts)
{
    for (auto& other : otherGts) {
        if (gt.ID == other.ID) continue;
        if (gt.pixelWidth() >= other.pixelWidth()) continue;

        double overlapScore = computeBoundingBoxOverlap(gt, other);
        if (overlapScore > 0) return true;
    }

    return false;
}

double TMEMotorwayProcessor::computeBoundingBoxOverlap (const GTEntry& gt1, const GTEntry& gt2)
{
    // get overlapping area
    double x1 = max(gt1.ip0.x, gt2.ip0.x);
    double y1 = max(gt1.ip0.y, gt2.ip0.y);
    double x2 = min(gt1.ip1.x, gt2.ip1.x);
    double y2 = min(gt1.ip1.y, gt2.ip1.y);

    // compute width and height of overlapping area
    double w = x2-x1;
    double h = y2-y1;

    // Set invalid entries to 0 overlap
    if(w <= 0 || h <= 0)
        return 0;

    // get overlapping areas
    double inter = w*h;
    double b_area = (gt2.ip1.x-gt2.ip0.x) * (gt2.ip1.y-gt2.ip0.y);

    return (inter / b_area);
}

void TMEMotorwayProcessor::show (int delay)
{
    string imageDirectory = getImageDirectory();
    vector<string> imageFiles;
    getFilesInDirectory(imageDirectory, imageFiles, "png");

    cv::Mat image;
    cv::namedWindow("TME Motorway Sequence", cv::WINDOW_AUTOSIZE);

    vector<GTEntry> gts; // ground truths

    for (int i = 100; i < imageFiles.size(); i++) {
        // Read ground truths from cache
        int frameNr = getImageIndex(imageFiles[i]);
        getGroundTruths(frameNr, gts);

        image = cv::imread(imageFiles[i], cv::IMREAD_COLOR);

        for (auto& gt : gts) {
            cv::Scalar color = (gt.occluded) ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 255);
            cv::rectangle(image, gt.ip0, gt.ip1, color, 2);
        }

        cv::imshow("TME Motorway Sequence", image);
        cv::waitKey(delay); // recorded @ 20 Hz
        image.release();
        gts.clear();
    }
}

void TMEMotorwayProcessor::readFrame(cv::Mat &image, vector<GTEntry> &gts)
{
    assert(currentFrame >= 0);

    image = cv::imread(imageFiles[currentFrame], cv::IMREAD_COLOR);
    getGroundTruths(getImageIndex(imageFiles[currentFrame]), gts);
}

void TMEMotorwayProcessor::readNextFrame(cv::Mat &image, vector<GTEntry> &gts)
{
    currentFrame++;

    TMEMotorwayProcessor::readFrame(image, gts);
}

bool TMEMotorwayProcessor::isInitialized()
{
    if (imageFiles.empty() || groundTruths.empty()) return false;
    if (calibration.x == 0 || calibration.y == 0)   return false;
    return true;
}

bool TMEMotorwayProcessor::hasNextFrame ()
{
    return (currentFrame < (int) imageFiles.size() - 1);
}

void TMEMotorwayProcessor::nextFrame()
{
    currentFrame++;
}

void TMEMotorwayProcessor::resetFrame()
{
    currentFrame = -1;
}

void TMEMotorwayProcessor::jumpToFrame(int frame)
{
    currentFrame = frame;
}

string TMEMotorwayProcessor::getImageName()
{
    return imageFiles[currentFrame];
}

void TMEMotorwayProcessor::getGroundTruths(vector<GTEntry>& gts)
{
    getGroundTruths(getImageIndex(), gts);
}

void TMEMotorwayProcessor::getGroundTruths (int frameNr, vector<GTEntry>& gts)
{
    gts.clear();
    if (groundTruths.empty()) return; // Map with ground truths filled ?
    if (groundTruths.find(frameNr) == groundTruths.end()) return; // check key not found
    gts = groundTruths.at(frameNr);
}

string TMEMotorwayProcessor::getGroundTruthFilename ()
{
    //string lightingSubset = (sequenceType == DAYLIGHT) ? "Daylight" : "Sunset";
    //string gtFilename = datasetPath + "ITSC2012gt-" + lightingSubset + ".txt";
    //return gtFilename;
    return (datasetPath + "ITSC2012gt.txt");
}

string TMEMotorwayProcessor::getImageDirectory ()
{
    //string lightingSubset = (sequenceType == DAYLIGHT) ? "Daylight" : "Sunset";
    //string imagePath = datasetPath + lightingSubset + "/" + sequence + "/Right/";
    string imagePath = datasetPath + sequence + "/Right/";
    return imagePath;
}

int TMEMotorwayProcessor::getImageIndex()
{
    return getImageIndex(getImageName());
}

int TMEMotorwayProcessor::getImageIndex (const string& imFilename)
{
    string tmp = getFilenameWithoutExtension(imFilename); // 022562-R
    tmp.erase(0, tmp.find_first_not_of('0'));             // remove preceding zeros
    tmp.erase(tmp.find_first_of('-'), 2);                 // remove '-R'
    return stoi(tmp);
}

void TMEMotorwayProcessor::convertImagesToRGB (const string& directory)
{
    vector<string> filenames;
    getFilesInDirectory(directory, filenames, "png");

    for (auto& s : filenames) {
        cv::Mat image = cv::imread(s, cv::IMREAD_UNCHANGED);

        if (image.type() == CV_8UC3) continue;

        cout << "Processing " << getFilename(s) << endl;
        cv::cvtColor(image, image, cv::COLOR_BayerGB2BGR);
        cv::imwrite(s, image);
    }
}
