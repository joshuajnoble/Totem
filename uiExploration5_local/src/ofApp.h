#pragma once

#include "ofMain.h"

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
		//void keyReleased(int key);
		//void mouseMoved(int x, int y );
		//void mouseDragged(int x, int y, int button);
		//void mousePressed(int x, int y, int button);
		//void mouseReleased(int x, int y, int button);
		//void windowResized(int w, int h);
  //  
		
		//----------------------------------------
		/* Panoramic unwarp stuff */
	
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
