#pragma once
#include <vector>
#include "StickyTimer.h"

class RemoteVideoInfo
{
public:
	std::string clientId;
	ofPtr<ofBaseVideoDraws> source;
	StickyTimer activityTimer;

	RemoteVideoInfo() : activityTimer(10.0f)
	{
	}

	bool IsActive() { return true; }// return this->activityTimer.IsActive();
};