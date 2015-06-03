#pragma once

#include "ofMain.h"
#include "VideoCaptureAppBase.h"

class ofRemoteApp : public VideoCaptureAppBase
{
private:
	float scale = 1.0f;
	int width = 1920, height = 1080;

public:
	//----------------------------------------
	/* standard openFrameworks app stuff */
	void earlyinit();
	void setup();
	void update();
	void draw();
	void exit();
	void keyPressed(int key);
	virtual int displayWidth() const;
	virtual int displayHeight() const;
};
