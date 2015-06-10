#pragma once
#include <vector>
#include "StickyTimer.h"

class RemoteVideoInfo
{
public:
	StickyTimer activityTimer;
	ofPtr<ofBaseVideoDraws> source;

	RemoteVideoInfo() : activityTimer(10.0f)
	{
	}

	bool IsActive() { return true; }// return this->activityTimer.IsActive();
};