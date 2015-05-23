#pragma once

#include "ofMain.h"

class TotemDisplay
{
public:
	virtual ~TotemDisplay() {}

	void initTotemDisplay(int displayCount, int width, int height);
	void update();
	void draw();
	void setVideoSource(int totemDisplayId, ofPtr<ofBaseVideoDraws> videoSource);

private:
	bool isFirstTime = true;
	ofFbo _output;
	vector<ofPtr<ofBaseVideoDraws>> videoSources;
	float displayRatio;
	int displayWidth, displayHeight;
	int displayCount;
};