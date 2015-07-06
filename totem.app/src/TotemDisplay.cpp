#include "TotemDisplay.h"
#include "Utils.h"
#include <unordered_set>

// ********************************************************************************************************************
void TotemDisplay::initTotemDisplay(int count, int width, int height)
{
	this->drawTestPattern = false;
#ifdef _DEBUG
	this->drawTestPattern = true;
#endif

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
void TotemDisplay::update()
{
	if (this->drawTestPattern)
	{
		for (int i = 0; i < this->displayCount; ++i)
		{
			auto fbo = this->_output[i];
			fbo.begin();
			ofPushStyle();
			ofBackground(0);

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

			ofPopStyle();
			fbo.end();
		}
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
void TotemDisplay::drawCloned()
{
	if (!this->_output.size())
	{
		return;
	}

	if (this->scale != 1.0)
	{
		ofPushMatrix();
		ofScale(this->scale, this->scale);
	}

	auto fbo = this->_output[0];

	for (int i = 0; i < this->displayCount; ++i)
	{
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