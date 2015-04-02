#pragma once

#include "ofMain.h"
#include "ofxTween.h"

//#define IMAGE
#define GRABBER

#include "ofxSpout.h"
#include "ofxOsc.h"

class ofApp : public ofBaseApp{
	
	public:
		
		//----------------------------------------
		/* standard openFrameworks app stuff */
		void setup();
		void update();
		void draw();
		void exit();
		void keyPressed  (int key);
	
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
