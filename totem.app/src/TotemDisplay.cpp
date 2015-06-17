#include "TotemDisplay.h"
#include "Utils.h"
#include <unordered_set>

// ********************************************************************************************************************
void TotemDisplay::initTotemDisplay(int count, int width, int height)
{
	this->displayCount = count;
	this->displayWidth = width;
	this->displayHeight = height;
	this->displayVertical = false;

	this->LoadXmlOverrideSettings();

	this->displayRatio = this->displayWidth / static_cast<float>(this->displayHeight);
}


// ********************************************************************************************************************
void TotemDisplay::LoadXmlOverrideSettings()
{
	ofXml xml;
	xml.load(ofToDataPath("totemdisplays.xml"));
	xml.setTo("totemDisplay");
	auto xmlScale = xml.getAttribute("scale");
	if (xmlScale.length() > 0)
	{
		this->scale = ofToFloat(xmlScale);
	}

	auto xmlOrientation = ofToLower(xml.getAttribute("orientation"));
	if (xmlOrientation.length() > 0)
	{
		if (xmlOrientation == "horizontal") this->displayVertical = false;
		else if (xmlOrientation == "vertical") this->displayVertical = true;
	}
}


// ********************************************************************************************************************
void TotemDisplay::allocateBuffers()
{
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


// ********************************************************************************************************************
int TotemDisplay::windowWidth() const
{
	if (this->displayVertical)
	{
		return this->displayWidth * this->scale;
	}

	return this->displayWidth * this->displayCount * this->scale;
}


// ********************************************************************************************************************
int TotemDisplay::windowHeight() const
{
	if (this->displayVertical)
	{
		return this->displayHeight * this->displayCount * this->scale;
	}

	return this->displayHeight * this->scale;
}


// ********************************************************************************************************************
void TotemDisplay::setVideoSource(int totemDisplayId, ofPtr<ofBaseVideoDraws> videoSource)
{
	if (totemDisplayId <= this->displayCount)
	{
		this->videoSources[totemDisplayId] = videoSource;
		this->isFirstTime = true;
	}
}


// ********************************************************************************************************************
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
		else if (this->drawTestPattern)
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


// ********************************************************************************************************************
void TotemDisplay::draw()
{
	if (this->scale != 1.0)
	{
		ofPushMatrix();
		ofScale(this->scale, this->scale);
	}

	for (int i = 0; i < this->displayCount; ++i)
	{
		auto fbo = this->_output[i];
		if (this->displayVertical)
		{
			fbo.draw(0, this->displayHeight * i); // Vertical
		}
		else
		{
			fbo.draw(this->displayWidth * i, 0); // Horizontal
		}
	}

	if (this->scale != 1.0)
	{
		ofPopMatrix();
	}
}


// ********************************************************************************************************************
ofFbo& TotemDisplay::getDisplay(int totemDisplayId)
{
	return this->_output[totemDisplayId];
}