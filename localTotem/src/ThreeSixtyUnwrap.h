#pragma once

#include "ofMain.h"
//#include "ofxOsc\src\ofxOsc.h"
//#include "ofxPlaylist\src\ofxPlaylist.h"
#include "ofxCv\src\ofxCv.h"
#include "ofxOpenCv\src\ofxCvColorImage.h"
#include "ofxOpenCv\src\ofxCvFloatImage.h"
#include "ofxXmlSettings\src\ofxXmlSettings.h"

class ThreeSixtyUnwrap
{
private:
	void computePanoramaProperties();
	void computeInversePolarTransform();

	int   warpedW;
	int   warpedH;
	float unwarpedW;
	float unwarpedH;
	float warpedCx;
	float warpedCy;
	float savedWarpedCx;
	float savedWarpedCy;
	float savedAngularOffset;
	float angularOffset;

	float maxR;
	float minR;
	float maxR_factor;
	float minR_factor;
	int   interpMethod;
	float playerScaleFactor;

	float *xocvdata;
	float *yocvdata;

	float yWarpA; // for parabolic fit for Y unwarping
	float yWarpB;
	float yWarpC;

	unsigned char *blackColor;
	CvScalar	blackOpenCV;
	IplImage	*warpedIplImage;
	IplImage	*unwarpedIplImage;
	ofxCvColorImage	warpedImageOpenCV;
	ofxCvColorImage unwarpedImageOpenCV;
	ofxCvFloatImage srcxArrayOpenCV;
	ofxCvFloatImage srcyArrayOpenCV;

	ofPixels conversionPixels;
	ofPixels inputPixels;

	unsigned char *warpedPixels;
	ofPixels unwarpedPixels;
	ofxXmlSettings XML;

	bool _isFramNew = false;
	bool _bCenterChanged = false;
	bool _bAngularOffsetChanged = false;

public:
	void setup();
	void update();
	bool isFrameNew();

	ofPtr<ofBaseVideoDraws> videoSource;
	ofImage unwarpedImage;
};

