#pragma once
#include <vector>
#include "StickyTimer.h"
#include "ofxGstRTP/src/ofxGstRTPClient.h"

class RemoteVideoInfo
{
public:
	std::string clientId = "";
	ofPtr<ofFbo> source;
	ofPtr<ofBaseVideoDraws> videoSource;
	StickyTimer activityTimer;
	ofPtr<ofxGstRTPClient> netClient;

	bool isTotem = false;

	RemoteVideoInfo() : activityTimer(10.0f)
	{
	}

	bool IsActive() { return true; }// return this->activityTimer.IsActive();
};