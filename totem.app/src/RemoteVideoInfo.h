#pragma once
#include <vector>
#include "StickyTimer.h"
#include "ofxGstRTP/src/ofxGstRTPClient.h"
#include "Utils.h"

class CroppedDrawable
{
public:
	virtual void DrawCropped(int width, int height) = 0;
};

class CroppedDrawableFbo : public CroppedDrawable
{
private:
	ofPtr<ofFbo> source;

public:
	CroppedDrawableFbo(ofPtr<ofFbo> source) : source(source)
	{
	}

	virtual void DrawCropped(int width, int height)
	{
		Utils::DrawCroppedToFit(*this->source, width, height);
	}
};

class CroppedDrawableVideoDraws : public CroppedDrawable
{
private:
	ofPtr<ofBaseVideoDraws> source;

public:
	CroppedDrawableVideoDraws(ofPtr<ofBaseVideoDraws> source) : source(source)
	{
	}

	virtual void DrawCropped(int width, int height)
	{
		Utils::DrawCroppedToFit(*this->source, width, height);
	}
};

class RemoteVideoInfo
{
public:
	std::string clientId = "";
	ofPtr<CroppedDrawable> source;
	int width;
	int height;
	StickyTimer activityTimer;
	ofPtr<ofxGstRTPClient> netClient;

	bool isTotem = false;

	RemoteVideoInfo() : activityTimer(10.0f)
	{
	}

	bool IsActive() { return true; }// return this->activityTimer.IsActive();
};