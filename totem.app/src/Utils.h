#pragma once

#include "ofMain.h"

class Utils
{
public:
	template <class T>
	static void DrawCroppedToFit(T& source, int displayWidth, int displayHeight);

	static void DrawCroppedToFit(ofImage& source, int displayWidth, int displayHeight);
	static void DrawCroppedToFit(ofImage& source, int x, int y, int displayWidth, int displayHeight);

	static void DrawCroppedToFit(ofFbo& source, int displayWidth, int displayHeight);
	static void DrawCroppedToFit(ofFbo& source, int x, int y, int displayWidth, int displayHeight);

	static void DrawCroppedToFit(ofBaseVideoDraws& source, int displayWidth, int displayHeight);
	static void DrawCroppedToFit(ofBaseVideoDraws& source, int x, int y, int displayWidth, int displayHeight);

	static ofRectangle CalculateCroppedRegion(int displayWidth, int displayHeight, int sourceWidth, int sourceHeight);
	
	static ofPtr<ofBaseVideoDraws> CreateVideoSourceFromCamera(int deviceId, int width, int height);
	static ofPtr<ofBaseVideoDraws> CreateVideoSourceFromFile(std::string path);
};