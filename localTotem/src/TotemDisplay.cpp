#include "TotemDisplay.h"

void TotemDisplay::initTotemDisplay(int count, int width, int height)
{
	this->displayCount = count;
	this->displayWidth = width;
	this->displayHeight = height;
	this->displayRatio = this->displayWidth / static_cast<float>(this->displayHeight);

	this->_output.allocate(this->displayWidth * this->displayCount, this->displayHeight, GL_RGB);
	for (int i = 0; i < this->displayCount; ++i)
	{
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
	bool isFrameNew = false;
	for (int i = 0; i < this->displayCount; ++i)
	{
		auto source = this->videoSources[i];
		if (source.get())
		{
			source->update();
			isFrameNew |= source->isFrameNew();
		}
	}

	if (isFrameNew || this->isFirstTime)
	{
		this->isFirstTime = false;

		this->_output.begin();
		ofClear(0, 0, 255);

		int x = 0;
		for (int i = 0; i < this->displayCount; ++i)
		{
			auto source = this->videoSources[i];
			if (source.get())
			{
				if (source->isFrameNew())
				{
					if (abs(source->getWidth() / static_cast<float>(source->getHeight()) - this->displayRatio) <= 0.2f)
					{
						// Just dirct draw or stretch to fit because the aspect ratios are very close
						source->draw(x, 0, this->displayWidth, this->displayHeight);
					}
					else
					{
						// Crop the image to make it fit reasonably in the space we have available
						ofImage i(source->getPixelsRef());
						auto sourceWidth = source->getWidth();
						auto sourceHeight = source->getHeight();
						if (false && source->getWidth() > source->getHeight())
						{
							auto cropWidth = sourceHeight * this->displayRatio;
							i.drawSubsection(x, 0, this->displayWidth, this->displayHeight, (sourceWidth - cropWidth) / 2, 0, cropWidth, sourceHeight);
						}
						else
						{
							auto cropHeight = sourceWidth / this->displayRatio;
							i.drawSubsection(x, 0, this->displayWidth, this->displayHeight, 0, (sourceHeight - cropHeight) / 2, sourceWidth, cropHeight, sourceWidth);
						}
					}
				}
			}
			else
			{
				// Draw Debug Display
				ofSetColor(0, 255/(i+1), 0);
				ofRect(x, 0, this->displayWidth, this->displayHeight);
				ofSetColor(0);

				int circleSize = 50;
				int circleSpacing = 20;
				int yOff = this->displayHeight / 2 - ((i * circleSpacing) + (i + 1) * (circleSize * 2)) / 2;
				for (int j = 0; j <= i; ++j)
				{
					ofCircle(x + this->displayWidth / 2, yOff, circleSize, circleSize);
					yOff += circleSize * 2 + circleSpacing;
				}

				ofSetColor(255);
			}

			x += this->displayWidth;
		}

		this->_output.end();
	}
}

void TotemDisplay::draw()
{
	this->_output.draw(0, 0);
}