#pragma once

#include "ofMain.h"

class TotemDisplay
{
public:
	virtual ~TotemDisplay() {}

	void initTotemDisplay(int displayCount, int width, int height);
	void allocateBuffers();
	void update();
	void draw();
	void drawCloned(); // Draw the first frame buffer to all displays
	//void setVideoSource(int totemDisplayId, ofPtr<ofBaseVideoDraws> videoSource);
	ofFbo& getDisplay(int totemDisplayId);
	int windowWidth() const;
	int windowHeight() const;

	bool drawTestPattern;

private:
	//bool isFirstTime = true;
	std::vector<ofFbo> _output;
	//std::vector<ofPtr<ofBaseVideoDraws>> videoSources;
	float displayRatio;
	int displayWidth, displayHeight, displayCount;
	float scale = 1.0;
	bool displayVertical;

	void LoadXmlOverrideSettings();
};