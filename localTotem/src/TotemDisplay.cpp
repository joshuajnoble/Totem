#include "TotemDisplay.h"
#include "Utils.h"
#include <unordered_set>

void TotemDisplay::initTotemDisplay(int count, int width, int height)
{
	this->displayCount = count;
	this->displayWidth = width;
	this->displayHeight = height;
	this->displayRatio = this->displayWidth / static_cast<float>(this->displayHeight);

	for (int i = 0; i < this->displayCount; ++i)
	{
		ofFbo fbo;
		fbo.allocate(this->displayWidth, this->displayHeight, GL_RGB);
		this->_output.push_back(fbo);
		fbo.begin();
		ofClear(0);
		fbo.end();

		this->videoSources.push_back(ofPtr<ofBaseVideoDraws>());
	}
}

void TotemDisplay::setVideoSource(int totemDisplayId, ofPtr<ofBaseVideoDraws> videoSource)
{
	if (totemDisplayId <= this->displayCount)
	{
		this->videoSources[totemDisplayId] = videoSource;
		this->isFirstTime = true;
	}
}

void TotemDisplay::update()
{
	// If the same source is provided on multiple displays, then the first will get updated, but the second one won't (since it's isFrameNew flas will be reset)
	// This set solves that problem for us by remembering if a given object has already indicated that is has a new frame;
	std::unordered_set<intptr_t> sourceIsFrameName;

	for (int i = 0; i < this->displayCount; ++i)
	{
		auto source = this->videoSources[i];
		auto fbo = this->_output[i];
		fbo.begin();

		if (source.get())
		{
			source->update();
			auto srcIntPtr = reinterpret_cast<intptr_t>(source.get());
			if (source->isFrameNew() || sourceIsFrameName.find(srcIntPtr) != sourceIsFrameName.end())
			{
				sourceIsFrameName.insert(srcIntPtr);
				if (abs(source->getWidth() / static_cast<float>(source->getHeight()) - this->displayRatio) <= 0.2f)
				{
					// Just direct draw or stretch to fit because the aspect ratios are very close
					source->draw(0, 0, this->displayWidth, this->displayHeight);
				}
				else
				{
					// Crop the image to make it fit reasonably in the space we have available
					Utils::DrawImageCroppedToFit(ofImage(source->getPixelsRef()), this->displayWidth, this->displayHeight);
				}
			}
		}
		else
		{
			// Draw Debug Display
			ofSetColor(0, 191 / (i + 1) + 64, 0);
			ofRect(0, 0, this->displayWidth, this->displayHeight);

			ofSetColor(0);
			int circleSize = 50;
			int circleSpacing = 20;
			int yOff = this->displayHeight / 2 - ((i * circleSpacing) + (i + 1) * (circleSize * 2)) / 2;
			for (int j = 0; j <= i; ++j)
			{
				ofCircle(this->displayWidth / 2, yOff, circleSize, circleSize);
				yOff += circleSize * 2 + circleSpacing;
				
				//Intentionally overdraw the fbo (width) to prove that the fbo automatically crops output without any problems.
				//ofRect(this->displayWidth / 2, this->displayHeight / (this->displayCount - 1) * i - 25, this->displayWidth, 50);
			}
		}

		ofSetColor(255);
		fbo.end();
	}
}

void TotemDisplay::draw()
{
	for (int i = 0; i < this->displayCount; ++i)
	{
		auto fbo = this->_output[i];
		//fbo.draw(this->displayWidth * i, 0); // Horizontal
		fbo.draw(0, this->displayHeight * i); // Vertical
	}
}

ofFbo& TotemDisplay::getDisplay(int totemDisplayId)
{
	return this->_output[totemDisplayId];
}