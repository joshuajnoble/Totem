#pragma once

#include "ofMain.h"
#include "VideoCaptureAppBase.h"
#include "ofxPlaylist\src\ofxPlaylist.h"
#include "ofxCv\src\ofxCv.h"
#include "ThreeSixtyUnwrap.h"
#include "..\..\sharedCode\StreamManager.h"
#include "TotemDisplay.h"

class ofTotemApp : public VideoCaptureAppBase
{
private:
	bool isInitialized = false;

	ofVec2f remotePosition, remoteScale;
	ofVec2f mainPosition, mainScale;
	double doubleM = 2560;
	ofPtr<ofBaseVideoDraws> processedVideo;

	TotemDisplay totemDisplay;
	StreamManager streamManager;
	ofPtr<ofImage> remoteImage;
	std::vector<ofPtr<ofFbo>> remoteVideoSources;
	
	void newClient(string& args);
	
public:
	//----------------------------------------
	/* standard openFrameworks app stuff */
	void earlyinit();
	void setup();
	void update();
	void draw();
	void exit();
	void keyPressed  (int key);
	void onKeyframe(ofxPlaylistEventArgs& args);
	virtual int displayWidth() const;
	virtual int displayHeight() const;

	// Public Config
	bool showInput = false;
	bool showUnwrapped = false;
	float unwrapMultiplier = 1.5f;
	float unwrapAspectRatio = 0.25f;
	bool passthroughVideo = false;

	// Legacy stuff
	//ofxPlaylist mainPlaylist;
	//ofImage image;
	//ofPtr<ofBaseVideoDraws> player;
	//ofImage small3, small1, small2;

	//bool drawCylinder;
	//ofTrueTypeFont din;
	//bool pixelsLoaded;
 //   
	//ofCylinderPrimitive cylinder;

	//ofxOscReceiver rec;
	//ofFbo fbo;
	//bool drawSecondRemote;
};
