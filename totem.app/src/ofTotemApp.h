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

	TotemDisplay totemDisplay;
	StreamManager streamManager;
	ofPtr<ofImage> remoteImage;
	std::vector<ofPtr<ofFbo>> remoteVideoSources;
	
	void newClient(string& args);
	ofPtr<ofBaseVideoDraws> netImpersonate;
	
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
	ofPtr<ofBaseVideoDraws> rawSource;
	bool showUnwrapped = false;

	void ImporsonateRemoteClient(ofPtr<ofBaseVideoDraws> source);
};
