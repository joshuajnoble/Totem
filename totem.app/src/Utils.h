#pragma once

#include "ofMain.h"

class Utils
{
public:
	static void DrawImageCroppedToFit(ofImage& source, int displayWidth, int displayHeight);
	static void DrawImageCroppedToFit(ofFbo& source, int displayWidth, int displayHeight);
	static void DrawVideoCroppedToFit(ofBaseVideoDraws& source, int displayWidth, int displayHeight);
	static ofRectangle DrawImageCroppedToFit(int displayWidth, int displayHeight, int sourceWidth, int sourceHeight);
	static ofPtr<ofBaseVideoDraws> CreateVideoSourceFromCamera(int deviceId, int width, int height);
	static ofPtr<ofBaseVideoDraws> CreateVideoSourceFromFile(std::string path);
};