#include "utils.h"

void Utils::DrawImageCroppedToFit(ofImage& source, int displayWidth, int displayHeight)
{
	float displayRatio = displayWidth / static_cast<float>(displayHeight);
	ofImage i(source.getPixelsRef());
	auto sourceWidth = source.getWidth();
	auto sourceHeight = source.getHeight();
	if (sourceHeight * displayRatio <= sourceWidth)
	{
		auto cropWidth = sourceHeight * displayRatio;
		i.drawSubsection(0, 0, displayWidth, displayHeight, (sourceWidth - cropWidth) / 2, 0, cropWidth, sourceHeight);
	}
	else
	{
		auto cropHeight = sourceWidth / displayRatio;
		i.drawSubsection(0, 0, displayWidth, displayHeight, 0, (sourceHeight - cropHeight) / 2, sourceWidth, cropHeight, sourceWidth);
	}
}

void Utils::DrawImageCroppedToFit(ofFbo& source, int displayWidth, int displayHeight)
{
	float displayRatio = displayWidth / static_cast<float>(displayHeight);
	ofPixels pixels;
	pixels.allocate(source.getWidth(), source.getHeight(), OF_IMAGE_COLOR);
	source.readToPixels(pixels);
	ofImage i(pixels);
	auto sourceWidth = source.getWidth();
	auto sourceHeight = source.getHeight();
	if (sourceHeight * displayRatio <= sourceWidth)
	{
		auto cropWidth = sourceHeight * displayRatio;
		i.drawSubsection(0, 0, displayWidth, displayHeight, (sourceWidth - cropWidth) / 2, 0, cropWidth, sourceHeight);
	}
	else
	{
		auto cropHeight = sourceWidth / displayRatio;
		i.drawSubsection(0, 0, displayWidth, displayHeight, 0, (sourceHeight - cropHeight) / 2, sourceWidth, cropHeight, sourceWidth);
	}
}

void Utils::DrawVideoCroppedToFit(ofBaseVideoDraws& source, int displayWidth, int displayHeight)
{
	float displayRatio = displayWidth / static_cast<float>(displayHeight);
	auto sourceWidth = source.getWidth();
	auto sourceHeight = source.getHeight();
	ofRectangle cropRegion;
	if (sourceHeight * displayRatio <= sourceWidth)
	{
		auto cropWidth = sourceHeight * displayRatio;
		cropRegion = ofRectangle((sourceWidth - cropWidth) / 2, 0, cropWidth, sourceHeight);
	}
	else
	{
		auto cropHeight = sourceWidth / displayRatio;
		cropRegion = ofRectangle((sourceHeight - cropHeight) / 2, sourceWidth, cropHeight, sourceWidth);
	}

	source.getTextureReference().drawSubsection(0, 0, displayWidth, displayHeight, cropRegion.x, cropRegion.y, cropRegion.width, cropRegion.height);
}
