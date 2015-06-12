#include "ofRemoteApp.h"
#include "WrapFboAsVideo.h"

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

	if (this->remoteTotem)
	{
		this->cylinderDisplay->update();
	}

	this->networkDisplay.update();

	ofSetVerticalSync(false);
}


// ********************************************************************************************************************
void ofRemoteApp::draw()
{
	ofBackground(11,26,38);

	if (this->remoteTotem)
	{
		this->cylinderDisplay->draw();
	}

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
void ofRemoteApp::RegisterRemoteVideoSource(RemoteVideoInfo& remote)
{
	//this->networkDisplay.AddVideoSource(remote.source);
	//this->cylinderDisplay->SetViewAngle(DEFAULT_ROTATION + SHIFTED_OFFSET);
}

// ********************************************************************************************************************
void ofRemoteApp::RegisterTotemVideoSource(RemoteVideoInfo& remote)
{
	//this->remoteTotemClientId = remote.clientId;
	//this->remoteTotemSource = remote.source;
	//this->cylinderDisplay.reset(new CylinderDisplay());
	//this->cylinderDisplay->initCylinderDisplay(1920, 1080);
	//this->cylinderDisplay->SetViewAngle(WAITING_ROTATION);
	//this->cylinderDisplay->setTotemVideoSource(this->remoteTotemSource);
}


// ********************************************************************************************************************
void ofRemoteApp::keyPressed(int key)
{
	if (this->networkDisplay.CanModify())
	{
		//if (key == 'a')
		//{
		//	if (!video1.get())
		//	{
		//		video1 = ofPtr<RemoteVideoInfo>(new RemoteVideoInfo());
		//		video1->source = this->videoSource;
		//		this->networkDisplay.AddVideoSource(video1);
		//	}
		//	else if (!video2.get())
		//	{
		//		video2 = ofPtr<RemoteVideoInfo>(new RemoteVideoInfo());
		//		video2->source = this->videoSource;
		//		this->networkDisplay.AddVideoSource(video2);
		//	}

		//	this->cylinderDisplay->SetViewAngle(DEFAULT_ROTATION + SHIFTED_OFFSET);
		//}
		//else if (key == 'z')
		//{
		//	if (video2.get())
		//	{
		//		if (this->networkDisplay.RemoveVideoSource(video2))
		//		{
		//			video2.reset();
		//		}
		//	}
		//	else if (video1.get())
		//	{
		//		if (this->networkDisplay.RemoveVideoSource(video1))
		//		{
		//			video1.reset();
		//		}

		//		this->cylinderDisplay->SetViewAngle(DEFAULT_ROTATION);
		//	}
		//}
	}
}


// ********************************************************************************************************************
void ofRemoteApp::Handle_ClientConnected(RemoteVideoInfo& remote)
{
	if (remote.isTotem)
	{
		this->remoteTotem.reset(new RemoteVideoInfo(remote));

		//auto wrapped = ofPtr<ofBaseVideoDraws>(new WrapFboAsVideo(remote.source));
		this->remoteTotemClientId = remote.clientId;
		this->cylinderDisplay.reset(new CylinderDisplay());
		this->cylinderDisplay->initCylinderDisplay(1920, 1080);
		this->cylinderDisplay->SetViewAngle(WAITING_ROTATION);
		this->cylinderDisplay->setTotemVideoSource(ofPtr<ofBaseVideoDraws>(new WrapFboAsVideo(remote.source)));
	}
	else
	{
		//auto wrapped = ofPtr<ofBaseVideoDraws>(new WrapFboAsVideo(remote.source));
		this->networkDisplay.AddVideoSource(remote.source);
		if (this->remoteTotem)
		{
			this->cylinderDisplay->SetViewAngle(DEFAULT_ROTATION + SHIFTED_OFFSET);
		}
	}
}

// ********************************************************************************************************************
void ofRemoteApp::Handle_ClientDisconnected(RemoteVideoInfo& remote)
{
	if (remote.isTotem)
	{
		this->remoteTotemClientId = "";
		this->remoteTotem.reset();
	}
	else
	{
		// TODO: This could fail (if it is already animating), so we should queue this up or something.
		this->networkDisplay.RemoveVideoSource(remote.source);
		this->cylinderDisplay.reset();
	}
}


// ********************************************************************************************************************
void ofRemoteApp::Handle_ClientStreamAvailable(RemoteVideoInfo& remote)
{
	if (remote.isTotem)
	{
		this->cylinderDisplay->SetViewAngle(DEFAULT_ROTATION, false);
		this->cylinderDisplay->DoWelcome();
	}
}