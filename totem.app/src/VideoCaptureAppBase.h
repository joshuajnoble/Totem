#pragma once

#include "ofMain.h"
#include "..\..\SharedCode\StreamManager.h"

class VideoCaptureAppBase : public ofBaseApp
{
public:
	virtual ~VideoCaptureAppBase() {};

	virtual int displayWidth() const = 0;
	virtual int displayHeight() const = 0;
	ofPtr<ofBaseVideoDraws> videoSource;

	virtual void update()
	{
		this->videoSource->update();
		if (this->videoSource->isFrameNew())
		{
			this->remoteImage->setFromPixels(this->videoSource->getPixelsRef());
			this->streamManager.newFrame();
		}

		this->streamManager.update();
	}

	virtual void exit()
	{
		this->streamManager.exit();
	}

private:
	void newClient(string& args)
	{
		auto client = this->streamManager.clients[args];
		auto video = this->streamManager.remoteVideos[args];
		Handle_ClientConnected(args, client, video);
	}

	void clientDisconnected(string& clientId)
	{
		Handle_ClientDisconnected(clientId);
	}

protected:
	StreamManager streamManager;
	ofPtr<ofImage> remoteImage;

	virtual void Handle_ClientConnected(string connectionId, ofPtr<ofxGstRTPClient> client, ofPtr<ofFbo> clientVideo) = 0;
	virtual void Handle_ClientDisconnected(string connectionId) = 0;

	void setupSteamManager()
	{
		streamManager.setup(this->videoSource->getWidth(), this->videoSource->getHeight());
		this->remoteImage = ofPtr<ofImage>(new ofImage());
		streamManager.setImageSource(this->remoteImage);
		ofAddListener(streamManager.newClientEvent, this, &VideoCaptureAppBase::newClient);
		ofAddListener(streamManager.clientDisconnectedEvent, this, &VideoCaptureAppBase::clientDisconnected);
	}
};