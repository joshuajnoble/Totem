#pragma once

#include "ofMain.h"
#include "..\..\SharedCode\StreamManager.h"

//class WrapFboAsVideo : public ofBaseVideoDraws
//{
//public:
//	ofPtr<ofFbo> rtpClient;
//	ofPixels placeholder;
//
//	WrapFboAsVideo(ofPtr<ofFbo> client) : rtpClient(client)
//	{
//	}
//
//	void update() {};
//	void close() { rtpClient.reset(); }
//
//	// ofBaseHasPixles implementation
//	unsigned char* getPixels() { return nullptr; }
//	ofPixelsRef getPixelsRef() { return placeholder; }
//	void draw(float x, float y, float w, float h) { this->rtpClient->draw(x, y, w, h); }
//	void draw(float x, float y) { this->rtpClient->draw(x, y); }
//
//	// ofBaseVideoDraws implementation
//	float getHeight() { return this->rtpClient->getHeight(); }
//	float getWidth() { return this->rtpClient->getWidth(); }
//	bool isFrameNew() { return true; }
//	ofTexture & getTextureReference() { return this->rtpClient->getTextureReference(); }
//	void setUseTexture(bool bUseTex) { this->rtpClient->setUseTexture(bUseTex); }
//};

class ofxGstRTPClientAsVideoSource : public ofBaseVideoDraws
{
private:
	ofFbo fbo;
	ofPtr<ofxGstRTPClient> client;

public:

	ofxGstRTPClientAsVideoSource(ofPtr<ofxGstRTPClient> client, int width, int height) : client(client)
	{

		this->fbo.allocate(width, height, GL_RGB);
	}

	void close()
	{
		client.reset();
	}

	void update()
	{
		// We don't call this->client.update() because that should already be called in the main window update loop.
		if (this->client->isFrameNewVideo())
		{
			this->fbo.getTextureReference().loadData(this->client->getPixelsVideo());
		}
	}

	// ofBaseHasPixles implementation
	unsigned char* getPixels() { return this->client->getPixelsVideo().getPixels(); }
	ofPixelsRef getPixelsRef() { return this->client->getPixelsVideo(); }
	void draw(float x, float y, float w, float h) { this->fbo.draw(x, y, w, h); }
	void draw(float x, float y) { this->fbo.draw(x, y); }

	// ofBaseVideoDraws implementation
	float getHeight() { return this->fbo.getHeight(); }
	float getWidth() { return this->fbo.getWidth(); }
	bool isFrameNew() { return this->client->isFrameNewVideo(); }
	ofTexture & getTextureReference() { return this->fbo.getTextureReference(); }
	void setUseTexture(bool bUseTex) { this->fbo.setUseTexture(bUseTex); }
};

