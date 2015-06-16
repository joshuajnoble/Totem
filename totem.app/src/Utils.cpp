#include "utils.h"

void Utils::DrawImageCroppedToFit(ofImage& source, int displayWidth, int displayHeight)
{
	auto sourceWidth = source.getWidth();
	auto sourceHeight = source.getHeight();
	auto cropRegion = Utils::DrawImageCroppedToFit(displayWidth, displayHeight, sourceWidth, sourceHeight);
	source.getTextureReference().drawSubsection(0, 0, displayWidth, displayHeight, cropRegion.x, cropRegion.y, cropRegion.width, cropRegion.height);
}

void Utils::DrawImageCroppedToFit(ofFbo& source, int displayWidth, int displayHeight)
{
	auto sourceWidth = source.getWidth();
	auto sourceHeight = source.getHeight();
	auto cropRegion = Utils::DrawImageCroppedToFit(displayWidth, displayHeight, sourceWidth, sourceHeight);
	source.getTextureReference().drawSubsection(0, 0, displayWidth, displayHeight, cropRegion.x, cropRegion.y, cropRegion.width, cropRegion.height);
}

void Utils::DrawVideoCroppedToFit(ofBaseVideoDraws& source, int displayWidth, int displayHeight)
{
	auto sourceWidth = source.getWidth();
	auto sourceHeight = source.getHeight();
	auto cropRegion = Utils::DrawImageCroppedToFit(displayWidth, displayHeight, sourceWidth, sourceHeight);
	source.getTextureReference().drawSubsection(0, 0, displayWidth, displayHeight, cropRegion.x, cropRegion.y, cropRegion.width, cropRegion.height);
}

ofRectangle Utils::DrawImageCroppedToFit(int displayWidth, int displayHeight, int sourceWidth, int sourceHeight)
{
	float displayRatio = displayWidth / static_cast<float>(displayHeight);
	ofRectangle cropRegion;
	if (sourceHeight * displayRatio <= sourceWidth)
	{
		auto cropWidth = sourceHeight * displayRatio;
		cropRegion = ofRectangle((sourceWidth - cropWidth) / 2, 0, cropWidth, sourceHeight);
	}
	else
	{
		auto cropHeight = sourceWidth / displayRatio;
		cropRegion = ofRectangle(0, (sourceHeight - cropHeight) / 2, sourceWidth, cropHeight);
	}

	return cropRegion;
}

ofPtr<ofBaseVideoDraws> Utils::CreateVideoSourceFromFile(std::string path)
{
	ofVideoPlayer* player = new ofVideoPlayer();
	ofPtr<ofBaseVideoDraws> rval = ofPtr<ofBaseVideoDraws>(player);
	if (player->loadMovie(path))
	{
		player->setLoopState(OF_LOOP_NORMAL);
		player->play();
	}

	return rval;
}

ofPtr<ofBaseVideoDraws> Utils::CreateVideoSourceFromCamera(int deviceId, int width, int height)
{
	ofVideoGrabber *grabber = new ofVideoGrabber();
	ofPtr<ofVideoGrabber> rval = ofPtr<ofVideoGrabber>(grabber);
	if (deviceId != 0)
	{
		grabber->setDeviceID(deviceId);
	}

	grabber->initGrabber(width, height);
	return rval;
}