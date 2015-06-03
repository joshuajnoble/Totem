#pragma once

#include "ofMain.h"

class VideoCaptureAppBase : public ofBaseApp
{
public:
	virtual ~VideoCaptureAppBase() {};

	virtual int displayWidth() const = 0;
	virtual int displayHeight() const = 0;

	ofPtr<ofBaseVideoDraws> videoSource;
};