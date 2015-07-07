#pragma once

#include "ofMain.h"
#include "ofxPlaylist/src/ofxEventKeyframe.h"

class Utils
{
public:
	template <class T>
	static void DrawCroppedToFit(T& source, int displayWidth, int displayHeight);

	static void DrawCroppedToFit(ofImage& source, int displayWidth, int displayHeight);
	static void DrawCroppedToFit(ofImage& source, int x, int y, int displayWidth, int displayHeight);

	static void DrawCroppedToFit(ofFbo& source, int displayWidth, int displayHeight);
	static void DrawCroppedToFit(ofFbo& source, int x, int y, int displayWidth, int displayHeight);

	static void DrawCroppedToFit(ofBaseVideoDraws& source, ofRectangle area);
	static void DrawCroppedToFit(ofBaseVideoDraws& source, int displayWidth, int displayHeight);
	static void DrawCroppedToFit(ofBaseVideoDraws& source, int x, int y, int displayWidth, int displayHeight);

	static ofRectangle CalculateCroppedRegion(int displayWidth, int displayHeight, int sourceWidth, int sourceHeight);
	
	static ofPtr<ofBaseVideoDraws> CreateVideoSourceFromCamera(int deviceId, int width, int height);
	static ofPtr<ofBaseVideoDraws> CreateVideoSourceFromFile(std::string path);
};

class ofxFunctionKeyframe : public ofxBaseKeyframe {
	bool isDelayed;
	const int delay_steps;

	long startValue;
	const bool isFrameBased;
	int step;

	const std::function<void()> _callback;

public:
	static shared_ptr<ofxBaseKeyframe> create(std::function<void()> callback);

	ofxFunctionKeyframe(const std::function<void()>& callback)
		: isDelayed(FALSE)
		, delay_steps(0)
		, startValue(0)
		, isFrameBased(TRUE)
		, step(0)
		, _callback(callback)
	{							// initialise with message .
		is_idle = FALSE;
		hasStarted = FALSE;
	};

	void start();
	void execute();
	bool delayHasEnded();
	int getDuration()	{ return delay_steps; };
};