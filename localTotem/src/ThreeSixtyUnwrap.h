#pragma once

#include "ofMain.h"
#include "ofxCv\src\ofxCv.h"
#include "ofxOpenCv\src\ofxCvColorImage.h"
#include "ofxOpenCv\src\ofxCvFloatImage.h"
#include "ofxXmlSettings\src\ofxXmlSettings.h"

class ThreeSixtyUnwrap : public ofBaseVideoDraws
{
public:
	virtual ~ThreeSixtyUnwrap(){}
	
	void initUnwrapper(ofPtr<ofBaseVideoDraws> videoSource, int outputWidth, int outputHeight);

	void update();
	void close();

	// ofBaseHasPixles implementation
	unsigned char* getPixels() { return this->unwrappedImage.getPixels(); }
	ofPixelsRef getPixelsRef() { return this->unwrappedImage.getPixelsRef(); }
	void draw(float x, float y, float w, float h) { this->unwrappedImage.draw(x, y, w, h); }
	void draw(float x, float y) { this->unwrappedImage.draw(x, y); }

	// ofBaseVideoDraws implementation
	float getHeight() { return this->unwrappedImage.getHeight(); }
	float getWidth() { return this->unwrappedImage.getWidth(); }
	bool isFrameNew() { return this->videoSource->isFrameNew(); }
	ofTexture & getTextureReference() { return this->unwrappedImage.getTextureReference(); }
	void setUseTexture(bool bUseTex) { this->unwrappedImage.setUseTexture(bUseTex); }

	void setVideoSource(ofPtr<ofBaseVideoDraws> videoSource) { this->videoSource = videoSource; }
	ofPtr<ofBaseVideoDraws> getVideoSource() { return this->videoSource; }

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

	CvScalar	blackOpenCV;
	IplImage	*warpedIplImage;
	IplImage	*unwarpedIplImage;
	ofxCvColorImage	warpedImageOpenCV;
	ofxCvColorImage unwrappedImageOpenCV;
	ofxCvFloatImage srcxArrayOpenCV;
	ofxCvFloatImage srcyArrayOpenCV;

	ofPixels conversionPixels;
	ofPixels inputPixels;

	ofPtr<unsigned char> warpedPixels;
	ofPixels unwarpedPixels;
	ofxXmlSettings XML;

	bool _bCenterChanged = false;
	bool _bAngularOffsetChanged = false;

	ofPtr<ofBaseVideoDraws> videoSource;
	ofImage unwrappedImage;
};