#pragma once

#include "ofMain.h"
#include "ofxSpout\src\ofxSpout.h"
#include "ofxOsc\src\ofxOsc.h"
#include "ofxPlaylist\src\ofxPlaylist.h"

class ofApp : public ofBaseApp
{
private:
	bool isInitialized = false;

	ofxPlaylist mainPlaylist;
	ofVec2f remotePosition, remoteScale;
	ofVec2f mainPosition, mainScale;

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
	ofPtr<ofBaseVideoDraws> videoSource;

	ofImage image;
	ofPtr<ofBaseVideoDraws> player;
	ofImage small3, small1, small2;

	bool drawCylinder;
	ofTrueTypeFont din;
	bool pixelsLoaded;
    
	ofCylinderPrimitive cylinder;

	ofxOscReceiver rec;
	ofFbo fbo;

	bool drawSecondRemote;
};