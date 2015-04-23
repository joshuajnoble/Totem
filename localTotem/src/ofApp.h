#pragma once

#include "ofMain.h"

//#define IMAGE
#define GRABBER

#include "ofxSpout.h"
#include "ofxOsc.h"
#include "ofxPlaylist.h"

class ofApp : public ofBaseApp{
	

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
	
        ofImage image;
		ofVideoPlayer player;
		ofImage small3, small1, small2;

		bool drawCylinder;
		ofTrueTypeFont din;
		bool pixelsLoaded;
    
		ofCylinderPrimitive cylinder;

		ofxOscReceiver rec;
		ofFbo fbo;

		bool drawSecondRemote;
		
};
