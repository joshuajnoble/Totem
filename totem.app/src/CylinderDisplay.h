#pragma once

#include "ofMain.h"
#include "ofTexture.h"
#include "ofxCv.h"
#include "ofxOsc.h"
#include "ofxGui\src\ofxGui.h"
#include "ofxPlaylist\src\ofxPlaylist.h"

enum NAV_STATE {
	USER_CONTROL, SYSTEM_CONTROL
};

enum USER_DISPLAY_STATE {
	UDS_SHOW_FEED, UDS_SHOW_REMOTE_USER, UDS_SHOW_L_LOCAL_USER, UDS_SHOW_BOTH_LOCAL_USER
};

class CylinderDisplay
{
private:
	ofFbo fboOutput;
	ofPtr<ofBaseVideoDraws> totemVideoSource;
	float scale = 1.0;
	int windowWidth;
	int windowHeight;


public:
	virtual ~CylinderDisplay() {}

	void initCylinderDisplay(int width, int height);
	void allocateBuffers();
	void update();
	void draw();
	void setTotemVideoSource(ofPtr<ofBaseVideoDraws> videoSource);

	ofVideoPlayer remotePlayer;

	void drawTexturedCylinder();
	void drawLeftCylinder();
	void drawRightCylinder();

	void findLeftFace();
	void findRightFace();

	void createCylinderPiece(ofMesh &m, float radius, float height, float degrees);
	void mapTexCoords(ofMesh &m, float u1, float v1, float u2, float v2);

	float currentLeftCylinder, currentRightCylinder, targetLeftCylinder, targetRightCylinder;
	int   warpedW;
	int   warpedH;
	int   cylinderRes;
	float *cylinderX;
	float *cylinderY;
	float cylinderWedgeAngle;

	bool isDrawingLeftCylinder, isDrawingRightCylinder, isDrawingSecondRemote;

	ofCylinderPrimitive cylinder;

	ofMesh leftCylinderPiece, rightCylinderPiece;
	ofVec4f prevTcoordLCP, prevTcoordRCP;
	ofPixels cropped;

	ofxCv::ObjectFinder finder;
};