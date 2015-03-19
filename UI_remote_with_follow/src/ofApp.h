#pragma once 

#include "ofMain.h"
#include "ofTexture.h"
#include "ofxCv.h"
#include "ofxOsc.h"
#include "ofxSpout.h"
//#include <ofxNetwork.h>

//#define IMAGE
#define GRABBER

enum NAV_STATE {
	USER_CONTROL, SYSTEM_CONTROL
};

enum USER_DISPLAY_STATE {
	UDS_SHOW_FEED, UDS_SHOW_REMOTE_USER, UDS_SHOW_L_LOCAL_USER, UDS_SHOW_BOTH_LOCAL_USER
};

class ofApp : public ofBaseApp{
	
	public:
		
	//----------------------------------------
	/* standard openFrameworks app stuff */
	void setup();
	void update();
	void draw();
	void exit();
	
	void keyPressed  (int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
    
		
	//----------------------------------------
	/* Panoramic unwarp stuff */
	
    ofImage image;
	ofVideoPlayer player;
    
    ofVideoGrabber grabber;
	int	currentCodecId;
	string outputFileName;
	char *handyString;
	
	void computePanoramaProperties();
	void computeInversePolarTransform();
		 
	void drawTexturedCylinder();
	void drawPlayer();
	void drawUnwarpedVideo();
	
	void drawLeftCylinder();
	void drawRightCylinder();
	void drawRemoteUser();
	
	void findLeftFace();
	void findRightFace();

	void createCylinderPiece(ofMesh &m, float radius, float height, float degrees);
	void mapTexCoords(ofMesh &m, float u1, float v1, float u2, float v2);
	bool bAngularOffsetChanged;

	ofImage unwarpedImage;
    
	float currentLeftCylinder, currentRightCylinder, targetLeftCylinder, targetRightCylinder;
	ofVec2f leftCylinderBegin, rightCylinderBegin;
	//
	int   warpedW;
	int   warpedH;
	float angularOffset;

	ofTexture unwarpedTexture;
	int   cylinderRes;
	float *cylinderX;
	float *cylinderY;
	float cylinderWedgeAngle;
	float blurredMouseX;
	float blurredMouseY;
    
    bool drawCylinder;
    ofTrueTypeFont din;
    bool pixelsLoaded;
    
	ofImage gradient, overlay;
    ofCylinderPrimitive cylinder;
	ofxOscSender sender;
	ofSerial serial;

	NAV_STATE navState;
	float userControlCountdown;
	ofMesh leftCylinderPiece, rightCylinderPiece;

	USER_DISPLAY_STATE userDisplayState;

	ofxCv::ObjectFinder finder;
};
