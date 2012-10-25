#ifndef _TEST_APP
#define _TEST_APP

//#define USE_IR // Uncomment this to use infra red instead of RGB cam...

#include "ofxOpenNI.h"
#include "ofMain.h"
#include "time.h"

struct imagePixel
{
    int depth;
    bool changed;
    ofColor color;
    time_t initialTime;
};
struct color_pixel
{
    bool darkened;
    time_t darkened_time;
    bool firstChange;
    bool secondChange;
    bool thirdChange;
};
struct noiseDepth
{
    int depth;
    int frequency;
};

class testApp : public ofBaseApp{
    
    
public:
	void setup();
	void update();
	void draw();
	ofxOpenNIContext	depthContext;
	ofxDepthGenerator	depthValues;
    ofxImageGenerator   frameImage;
	ofxHardwareDriver	hardware;
    ofImage depthImage, color_image, snapshot;
    struct imagePixel previousDepth[640][480];
    int previousColorSum[640][480];
    void noiseReduction();
    void draw_squares();
    void keyPressed(int key);
    int column, rows, colorThreshold,depthThresholdValue;
    bool considerColor, considerDepth;
    float factor1,factor2;
    //stringstream msg;
    
    int lowerX;
    int upperX;
    int lowerY;
    int upperY;
    int imageTaken;
    struct color_pixel colorDarkenedValues[640][480];
    //int snapshot[640][480];
    int rectifiedDepthValues[640][480];
};


#endif
