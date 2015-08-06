#pragma once
#include <vector>
#include "StickyTimer.h"
#include "Utils.h"
#include "UdpDiscovery.h"

class ofxFFmpegVideoReceiver;

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
	UdpDiscovery::RemotePeerStatus peerStatus;
	ofxFFmpegVideoReceiver *netClient;
	ofPtr<CroppedDrawable> videoSource;
	bool hasLiveFeed;
};