#pragma once

#include "ofMain.h"

class WrapFboAsVideo : public ofBaseVideoDraws
{
public:
	ofPtr<ofFbo> rtpClient;
	ofPixels placeholder;

	WrapFboAsVideo(ofPtr<ofFbo> client) : rtpClient(client)
	{
	}

	void update() {};
	void close() { rtpClient.reset(); }

	// ofBaseHasPixles implementation
	unsigned char* getPixels() { return nullptr; }
	ofPixelsRef getPixelsRef() { return placeholder; }
	void draw(float x, float y, float w, float h) { this->rtpClient->draw(x, y, w, h); }
	void draw(float x, float y) { this->rtpClient->draw(x, y); }

	// ofBaseVideoDraws implementation
	float getHeight() { return this->rtpClient->getHeight(); }
	float getWidth() { return this->rtpClient->getWidth(); }
	bool isFrameNew() { return true; }
	ofTexture & getTextureReference() { return this->rtpClient->getTextureReference(); }
	void setUseTexture(bool bUseTex) { this->rtpClient->setUseTexture(bUseTex); }
};