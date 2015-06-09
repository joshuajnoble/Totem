#pragma once

#include "ofMain.h"
#include "VideoCaptureAppBase.h"
#include "ofTexture.h"
#include "ofxCv.h"
#include "ofxOsc.h"
#include "ofxGui\src\ofxGui.h"
#include "ofxPlaylist\src\ofxPlaylist.h"
#include "CylinderDisplay.h"

class ofRemoteApp : public VideoCaptureAppBase
{
private:
	float scale = 1.0f;
	int width = 1920, height = 1080;

	ofxPlaylist mainPlaylist;
	float rotateToPosition;

	void DrawSelfie();
	ofPtr<ofBaseVideoDraws> remoteTotemSource;

public:
	//----------------------------------------
	/* standard openFrameworks app stuff */
	void earlyinit();
	void setup();
	void update();
	void draw();
	void exit();
	//void keyPressed(int key);
	//void keyReleased(int key);
	//void mouseMoved(int x, int y);
	//void mouseDragged(int x, int y, int button);
	//void mousePressed(int x, int y, int button);
	//void mouseReleased(int x, int y, int button);
	//void windowResized(int w, int h);

	void RegisterTotemVideoSource(ofPtr<ofBaseVideoDraws> source);

	virtual int displayWidth() const;
	virtual int displayHeight() const;

	//----------------------------------------
	/* Panoramic unwarp stuff */
	CylinderDisplay cylinderDisplay;

	//ofVideoPlayer remotePlayer;
	//ofImage image;

	//ofVideoGrabber grabber;
	//int	currentCodecId;
	//string outputFileName;
	//char *handyString;

	//void computePanoramaProperties();
	//void computeInversePolarTransform();

	//void drawTexturedCylinder();
	//void drawPlayer();
	//void drawLeftCylinder();
	//void drawRightCylinder();
	//void drawRemoteUser();

	//void findLeftFace();
	//void findRightFace();

	//void createCylinderPiece(ofMesh &m, float radius, float height, float degrees);
	//void mapTexCoords(ofMesh &m, float u1, float v1, float u2, float v2);
	//bool bAngularOffsetChanged;

	//ofImage unwarpedImage;

	//float currentLeftCylinder, currentRightCylinder, targetLeftCylinder, targetRightCylinder;
	//ofVec2f leftCylinderBegin, rightCylinderBegin;
	////
	//int   warpedW;
	//int   warpedH;
	//float angularOffset;

	//ofTexture unwarpedTexture;
	//int   cylinderRes;
	//float *cylinderX;
	//float *cylinderY;
	//float cylinderWedgeAngle;
	//float blurredMouseX;
	//float blurredMouseY;

	//bool drawCylinder, isDrawingLeftCylinder, isDrawingRightCylinder, isDrawingSecondRemote;
	//ofTrueTypeFont din;
	//bool pixelsLoaded;

	//ofImage gradient, overlay;
	//ofCylinderPrimitive cylinder;
	//ofxOscSender sender;
	//ofSerial serial;

	//NAV_STATE navState;
	//float userControlCountdown;
	//ofMesh leftCylinderPiece, rightCylinderPiece;
	//ofVec4f prevTcoordLCP, prevTcoordRCP;
	//ofPixels cropped;

	//USER_DISPLAY_STATE userDisplayState;

	//ofxCv::ObjectFinder finder;

	//ofEasyCam cam;

	//ofxPanel gui;
	//ofxFloatSlider xPosition, yPosition, zPosition, rotation;

	//int detectedMicrophone[6];
	//bool isDoingMicDetection;

	//ofImage panoView, remoteCaller, remoteCaller1, remoteCaller2;

	//bool showInstructions;
};
