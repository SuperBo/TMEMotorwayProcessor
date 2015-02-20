//
//  TMEMotorwayProcessor.h
//  TMEMotorwayProcessor
//
//  Created by Tom Runia on 16/02/15.
//

#ifndef __TMEMotorwayProcessor__TMEMotorwayProcessor__
#define __TMEMotorwayProcessor__TMEMotorwayProcessor__

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

// Only tested on OS X 10.9
#ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
#else
#ifdef _WIN32
  #include <windows.h>
#endif
  #include <GL/gl.h>
  #include <GL/glu.h>
#endif

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/objdetect/objdetect.hpp>

// Only used for string.trim() and string.split()
#include <boost/algorithm/string.hpp>

#include "Utilities.h"

using namespace std;
using namespace utilities;

static const double CAR_TRUCK_WIDTH_BOUNDARY_TH = 2.1;
static const double CAR_HEIGHT = 1.5;
static const double TRUCK_HEIGHT = 2.1;

static const double TRUNCATED_TH = 0.75;
static const double OVERLAP_TH = 0.2;
static const double OCCLUSION_TH = 0.2;

enum SequenceType
{
    DAYLIGHT,
    SUNSET
};

struct CalibrationParameters
{
    double x;
    double y;
    double z;
    double roll_deg;
    double yaw_deg;
    double pitch_deg;
    double h_fov_deg;
    int image_width;
    int image_height;
    inline double Roll_rad() const { return roll_deg *M_PI/180; }
    inline double Yaw_rad() const { return yaw_deg*M_PI/180; }
    inline double Pitch_rad() const { return pitch_deg*M_PI/180; }
    inline double H_fov_rad() const { return h_fov_deg*M_PI/180; }
};

struct GTEntry
{
    unsigned long ID;
    cv::Point2d wp0;        // World coordinate
    cv::Point2d wp1;        // World coordinate
    
    bool reconstructed;     // Sample was not detected at this frame, but interpolated from previous and successive detection
    double estimatedWidth;  // Estimated across all the observation time of this target
    
    // Variables computed from the static calibration
    cv::Point2d ip0;        // Screen coordinates
    cv::Point2d ip1;        // Screen coordinates
    
    bool truncated;
    bool occluded;
    bool truck;
    
    inline double assignedHeight()  const { return (estimatedWidth > CAR_TRUCK_WIDTH_BOUNDARY_TH) ? TRUCK_HEIGHT : CAR_HEIGHT; }
    inline double meterWidth()      const { return (wp1.y - wp0.y); }
    inline double pixelWidth()      const { return (ip1.x - ip0.x); }
    inline double pixelHeight()     const { return (ip1.y - ip0.y); }
    inline double ImageArea()       const { return pixelWidth() * pixelHeight(); }
    inline bool isTruck ()          const { return (estimatedWidth > CAR_TRUCK_WIDTH_BOUNDARY_TH) ? true : false; }
    
    inline double meterToPixel() const
    {
        if (meterWidth())
            return pixelWidth() / meterWidth();
        else
            return 0.0;
    }
    
    inline double PixelToMeter() const
    {
        if (pixelWidth())
            return meterWidth() / (pixelWidth());
        else
            return 0.0;
    }
    
    void print ()
    {
        printf("ID = %i\n", (int)ID);
        printf("wp0 = (%2.2f, %2.2f)\n", wp0.x, wp0.y);
        printf("wp1 = (%2.2f, %2.2f)\n", wp1.x, wp1.y);
        printf("reconsructed = %i\n", reconstructed);
        printf("estimated width = %3.2f\n", estimatedWidth);
    }
};

class TMEMotorwayProcessor
{
public:
    TMEMotorwayProcessor ();
    TMEMotorwayProcessor(const string& _datasetPath);
    
    void initSequence (const SequenceType& _sequenceType, const string& _sequence);
    void readCalibrationParameters (const string& calibrationFile = "calibration.ini");
    void readGroundTruths ();
    void getGroundTruths (int frame, vector<GTEntry>& groundTruths);
    
    string getGrountTruthFilename ();
    string getImageDirectory ();
    int getImageIndex (const string& imFilename);
    
    void show(int delay = 5);
    void readFrame(cv::Mat& image, vector<GTEntry>& gts);
    void jumpToFrame (int frame);
    bool hasNextFrame();
    bool isInitialized();
    
    void computeScreenCoordinates ();
    double computeBoundingBoxOverlap (const GTEntry& gt1, const GTEntry& gt2);
    bool isOccluded (const GTEntry& gt, const vector<GTEntry>& otherGts);
    
    void convertImagesToRGB (const string& directory);
    
private:
    string datasetPath;
    string sequence;
    SequenceType sequenceType;
    CalibrationParameters calibration;
    
    // This map stores ground truths for each frame (key = frame number)
    map<int, vector<GTEntry>> groundTruths;
    
    // List of images in the stream
    vector<string> imageFiles;
    int currentFrame;
    
};

#endif /* defined(__TMEMotorwayProcessor__TMEMotorwayProcessor__) */
