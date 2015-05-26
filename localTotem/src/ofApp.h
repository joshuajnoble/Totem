#pragma once

#include "ofMain.h"
#include "ofxOsc\src\ofxOsc.h"
#include "ofxPlaylist\src\ofxPlaylist.h"
#include "ofxCv\src\ofxCv.h"
#include "ThreeSixtyUnwrap.h"
#include "..\..\sharedCode\StreamManager.h"
#include "TotemDisplay.h"

class ofApp : public ofBaseApp
{
private:
	bool isInitialized = false;

	ofxPlaylist mainPlaylist;
	ofVec2f remotePosition, remoteScale;
	ofVec2f mainPosition, mainScale;
	double doubleM = 2560;
	//ThreeSixtyUnwrap unwrapper;

	TotemDisplay totemDisplay;
	StreamManager streamManager;
	ofPtr<ofImage> remoteImage;
	void newClient(string& args);

public:
	//----------------------------------------
	/* standard openFrameworks app stuff */
	void setup();
	void update();
	void draw();
	void exit();
	void keyPressed  (int key);
	void onKeyframe(ofxPlaylistEventArgs& args);

	static ofPtr<ofBaseVideoDraws> InitializeVideoPresenterFromFile(std::string path);
	static ofPtr<ofBaseVideoDraws> InitializePlayerFromCamera(int deviceId, int width, int height);

	// Public Config
	bool showInput = false;
	bool showUnwrapped = false;
	ofPtr<ofBaseVideoDraws> videoSource;
	float unwrapMultiplier = 1.5f;
	float unwrapAspectRatio = 0.25f;

	// Legacy stuff
	ofImage image;
	ofPtr<ofBaseVideoDraws> player;
	ofPtr<ofBaseVideoDraws> processedVideo;
	ofImage small3, small1, small2;
	std::vector<ofPtr<ofFbo>> remoteVideoSources;

	bool drawCylinder;
	ofTrueTypeFont din;
	bool pixelsLoaded;
    
	ofCylinderPrimitive cylinder;

	ofxOscReceiver rec;
	ofFbo fbo;

	bool passthroughVideo = false;
	bool drawSecondRemote;
};
