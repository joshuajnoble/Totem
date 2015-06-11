#include "ofRemoteApp.h"

#define CYLINDER_PIECE_WIDTH 44
#define CYLINDER_PIECE_HEIGHT 2
#define PIECE_TEXCOORD_WIDTH 720
#define NEO_PIXELS_COUNT 45

namespace
{
	float lastElapsed;
	float lastSentMouseLocation;
	const float WAITING_ROTATION = 270.0f; // TODO: Why is the "centered" spin icon at 270 deg and not 180 deg?
	const float DEFAULT_ROTATION = 50.0f;
	const float SHIFTED_OFFSET = 25.0f;

	//DEBUG
	ofPtr<RemoteVideoInfo> video1;
	ofPtr<RemoteVideoInfo> video2;
}

// ********************************************************************************************************************
void ofRemoteApp::earlyinit()
{
	this->cylinderDisplay.initCylinderDisplay(1920, 1080);
	int margin = 70;
	int remoteViewOffsetX = 1130 + margin;// (int)(1920 * .75);
	this->networkDisplay.initializeRemoteNetworkDisplay(ofRectangle(remoteViewOffsetX, 70, 1920 - remoteViewOffsetX - margin, 1080 - margin * 2));
}

// ********************************************************************************************************************
void ofRemoteApp::setup()
{
	VideoCaptureAppBase::setupSteamManager();
}


// ********************************************************************************************************************
void ofRemoteApp::update()
{
	VideoCaptureAppBase::update();

	if (this->remoteTotemSource.get())
	{
		this->remoteTotemSource->update();
		if (this->remoteTotemSource->isFrameNew())
		{
			this->cylinderDisplay.update();
		}
	}

	this->networkDisplay.update();

	ofSetVerticalSync(false);
}


// ********************************************************************************************************************
void ofRemoteApp::draw()
{
	ofBackground(11,26,38);

	this->cylinderDisplay.draw();

	DrawSelfie();

	this->networkDisplay.draw();

	if (false)// showInstructions)
	{
		//ofDrawBitmapString(" ?: show instructions \n z: show remote caller \n m: turn on directional mics \n l: look for the left-most participant \n r: look for the right most participant \n space: show the raw v360 feed", 50, 50);
	}
}


// ********************************************************************************************************************
void ofRemoteApp::DrawSelfie()
{
	ofPushMatrix();

	// Scale the video, but also flip the X axis, so we see it mirrored.
	// This means all of the following x values need to be negated and offset.
	ofScale(-this->scale, this->scale);
	
	// Draw the source video in a small window
	auto ratio = this->videoSource->getWidth() / this->videoSource->getHeight();
	int selfieWidth = 320;
	int selfieHeight = selfieWidth / ratio;
	int selfieX = (this->width - selfieWidth) / 2;
	int selfieY = 40;
	int margin = 15;

	ofSetColor(32); // Selfie margin color
	ofRect(-(selfieX + margin + selfieWidth), selfieY - margin, selfieWidth + margin * 2, selfieHeight + margin * 2);

	ofSetColor(255);
	this->videoSource->draw(-(selfieX + selfieWidth), selfieY, selfieWidth, selfieHeight);

	ofPopMatrix();
}


// ********************************************************************************************************************
void ofRemoteApp::exit()
{
	VideoCaptureAppBase::exit();
}


// ********************************************************************************************************************
int ofRemoteApp::displayWidth() const
{
	return this->width * this->scale;
}


// ********************************************************************************************************************
int ofRemoteApp::displayHeight() const
{
	return this->height * this->scale;
}


// ********************************************************************************************************************
void ofRemoteApp::RegisterTotemVideoSource(string clientId, ofPtr<ofBaseVideoDraws> source)
{
	this->remoteTotemClientId = clientId;
	this->remoteTotemSource = source;
	this->cylinderDisplay.SetViewAngle(WAITING_ROTATION);
	this->cylinderDisplay.setTotemVideoSource(this->remoteTotemSource);
}


// ********************************************************************************************************************
ofPtr<RemoteVideoInfo> ofRemoteApp::RegisterRemoteVideoSource(string clientId, ofPtr<ofBaseVideoDraws> source)
{
	auto remote = ofPtr<RemoteVideoInfo>(new RemoteVideoInfo());
	remote->clientId = clientId;
	remote->source = source;
	this->remoteVideoSources.push_back(remote);
	this->networkDisplay.AddVideoSource(remote);

	this->cylinderDisplay.SetViewAngle(DEFAULT_ROTATION + SHIFTED_OFFSET);

	return remote;
}


// ********************************************************************************************************************
void ofRemoteApp::keyPressed(int key)
{
	if (this->networkDisplay.CanModify())
	{
		if (key == 'a')
		{
			if (!video1.get())
			{
				video1 = ofPtr<RemoteVideoInfo>(new RemoteVideoInfo());
				video1->source = this->videoSource;
				this->networkDisplay.AddVideoSource(video1);
			}
			else if (!video2.get())
			{
				video2 = ofPtr<RemoteVideoInfo>(new RemoteVideoInfo());
				video2->source = this->videoSource;
				this->networkDisplay.AddVideoSource(video2);
			}

			this->cylinderDisplay.SetViewAngle(DEFAULT_ROTATION + SHIFTED_OFFSET);
		}
		else if (key == 'z')
		{
			if (video2.get())
			{
				if (this->networkDisplay.RemoveVideoSource(video2))
				{
					video2.reset();
				}
			}
			else if (video1.get())
			{
				if (this->networkDisplay.RemoveVideoSource(video1))
				{
					video1.reset();
				}

				this->cylinderDisplay.SetViewAngle(DEFAULT_ROTATION);
			}
		}
	}
}

class VideoNetworkWrapper : public ofBaseVideoDraws
{
public:
	ofPtr<ofFbo> rtpClient;
	ofPixels placeholder;

	VideoNetworkWrapper(ofPtr<ofFbo> client) : rtpClient(client)
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


// ********************************************************************************************************************
void ofRemoteApp::Handle_ClientConnected(string clientId, ofPtr<ofxGstRTPClient> client, ofPtr<ofFbo> clientVideo)
{
	if (clientVideo->getWidth() / clientVideo->getHeight() < 2)
	{
		auto wrapped = ofPtr<ofBaseVideoDraws>(new VideoNetworkWrapper(clientVideo));
		RegisterRemoteVideoSource(clientId,wrapped);
	}
	else
	{
		auto wrapped = ofPtr<ofBaseVideoDraws>(new VideoNetworkWrapper(clientVideo));
		RegisterTotemVideoSource(clientId, wrapped);
	}
}

// ********************************************************************************************************************
void ofRemoteApp::Handle_ClientDisconnected(string connectionId)
{
	if (this->remoteTotemClientId == connectionId)
	{
		this->remoteTotemClientId = "";
		this->remoteTotemSource.reset();
	}
	else
	{
		for (auto iter = this->remoteVideoSources.begin(); iter != this->remoteVideoSources.end(); ++iter)
		{
			auto client = *iter;
			if (client->clientId == connectionId)
			{
				if (client == video1)
				{
					// TODO: This could fail (if it is already animating), so we should queue this up or something.
					this->networkDisplay.RemoveVideoSource(video1);
					video1.reset();
				}
				else if (client == video2)
				{
					// TODO: This could fail (if it is already animating), so we should queue this up or something.
					this->networkDisplay.RemoveVideoSource(video2);
					video2.reset();
				}

				this->remoteVideoSources.erase(iter);
				break;
			}
		}
	}
}


// ********************************************************************************************************************
void ofRemoteApp::Handle_ClientStreamAvailable(string connectionId)
{
	if (this->remoteTotemClientId == connectionId)
	{
		this->cylinderDisplay.SetViewAngle(DEFAULT_ROTATION, false);
		this->cylinderDisplay.DoWelcome();
	}
}