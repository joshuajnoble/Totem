#pragma once

#include "ofMain.h"
#include "ofTexture.h"
#include "ofxCv.h"
#include "ofxOsc.h"
#include "ofxGui\src\ofxGui.h"
#include "ofxPlaylist\src\ofxPlaylist.h"

class CylinderDisplay
{
private:
	ofPtr<ofBaseVideoDraws> totemVideoSource;
	float scale = 1.0;
	int windowWidth;
	int windowHeight;

	float viewRotationAngle = 0;

	void drawTexturedCylinder();
	void drawLeftCylinder();
	void drawRightCylinder();

	void findLeftFace();
	void findRightFace();

	void createCylinderPiece(ofMesh &m, float radius, float height, float degrees);
	void mapTexCoords(ofMesh &m, float u1, float v1, float u2, float v2);
	float getClickAngle(int x);

	float currentLeftCylinder, currentRightCylinder, targetLeftCylinder, targetRightCylinder;
	int warpedW;
	int warpedH;
	float cylinderCircumference;
	float cylinderWedgeAngle;

	ofPoint dragStart;
	float startDragAngle;
	float startDragAngleOffset;
	bool isDragging = false;

	ofMesh leftCylinderPiece, rightCylinderPiece;
	ofVec4f prevTcoordLCP, prevTcoordRCP;
	ofPixels cropped;

	ofxCv::ObjectFinder finder;
	ofCylinderPrimitive cylinder;
	ofxPlaylist introPlaylist;

public:
	virtual ~CylinderDisplay() {}

	void initCylinderDisplay(int width, int height);
	void allocateBuffers(int width, int height);
	void update();
	void draw();
	void setTotemVideoSource(ofPtr<ofBaseVideoDraws> videoSource, int width, int height);
	ofPtr<ofBaseVideoDraws> getTotemVideoSource();
	void DoWelcome(const string& eventName);

	bool isDrawingLeftCylinder, isDrawingRightCylinder, isDrawingSecondRemote;
	float GetViewAngle() const;
	void SetViewAngle(float angle, bool animate = true);
	static float NormalizeAngle(float);

	void DragStart(ofPoint screenPosition);
	void DragMove(ofPoint screenPosition);
	void DragEnd(ofPoint screenPosition);
	bool IsDragging() const { return this->isDragging; }
};