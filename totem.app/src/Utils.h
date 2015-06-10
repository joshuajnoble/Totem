#pragma once

#include "ofMain.h"

class Utils
{
public:
	static void DrawImageCroppedToFit(ofImage& source, int displayWidth, int displayHeight);
	static void DrawImageCroppedToFit(ofFbo& source, int displayWidth, int displayHeight);
	static void DrawVideoCroppedToFit(ofBaseVideoDraws& source, int displayWidth, int displayHeight);
};