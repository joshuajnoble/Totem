#include "ofRemoteApp.h"
#include "WrapFboAsVideo.h"

#define CYLINDER_PIECE_WIDTH 44
#define CYLINDER_PIECE_HEIGHT 2
#define PIECE_TEXCOORD_WIDTH 720
#define NEO_PIXELS_COUNT 45

using namespace Playlist;

namespace
{
	const float WAITING_ROTATION = 270.0f; // TODO: Why is the "centered" spin icon at 270 deg and not 180 deg?
	const float DEFAULT_ROTATION = 50.0f;
	const float SHIFTED_OFFSET = 25.0f;
	
	const float TIME_INTRO_TRANSITION = 750.0f;
	const float TIME_INTRO_ICONS_APPEAR = 250.0f;

	const int INTRO_SELFIE_TOP_MARGIN = 88;
	const int MINI_SELFIE_TOP_MARGIN = 40;
	const int MINI_SELFIE_WIDTH = 320;
	const int SELFIE_FRAME_MARGIN = 10;
	const int ICON_MARGIN = 35;

	const string INTRO_TRANSITION_COMPLETE_EVENT = "INTRO_TRANSITION_COMPLETE_EVENT";
	//DEBUG
	//ofPtr<RemoteVideoInfo> video1;
	//ofPtr<RemoteVideoInfo> video2;
}


// ********************************************************************************************************************
void ofRemoteApp::earlyinit()
{
	int margin = 70;
	int remoteViewOffsetX = 1130 + margin;// (int)(1920 * .75);
	this->networkDisplay.initializeRemoteNetworkDisplay(ofRectangle(remoteViewOffsetX, 70, 1920 - remoteViewOffsetX - margin, 1080 - margin * 2));
	this->currentConnectIconAlpha = 0;
	this->state = UISTATE_STARTUP;
}


// ********************************************************************************************************************
void ofRemoteApp::setup()
{
	ofxKeyframeAnimRegisterEvents(this);
	VideoCaptureAppBase::setupSteamManager();
	connectIcon.loadImage("icon_connect.png");
	hangupIcon.loadImage("icon_hangup.png");
	muteIcon.loadImage("icon_mute.png");
}


// ********************************************************************************************************************
void ofRemoteApp::update()
{
	this->playlist.update();

	VideoCaptureAppBase::update();

	if (this->state == UISTATE_STARTUP)
	{
		if (this->videoSource)
		{
			this->currentHangupMuteIconAlpha = 0;
			this->currentConnectIconAlpha = 1;
			this->currentSelfieYPosition = INTRO_SELFIE_TOP_MARGIN;
			this->currentSelfieWidth = (int)(this->width * 0.75f);

			this->state = UISTATE_INTRO;
		}
	}

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
	ofBackground(0x09,0x22,0x36);

	if (this->state == UISTATE_INTRO)
	{
		DrawSelfie();

		ofPushStyle();
		ofSetRectMode(OF_RECTMODE_CENTER);

		// Draw icons first so they animate out from behind the selfie
		ofSetColor(255, 255, 255, (int)(255 * currentHangupMuteIconAlpha));
		this->muteIcon.draw(this->muteIconCenterX, MINI_SELFIE_TOP_MARGIN + this->muteIcon.getHeight() / 2);
		this->hangupIcon.draw(this->hangupIconCenterX, MINI_SELFIE_TOP_MARGIN + this->hangupIcon.getHeight() / 2);

		//Now draw the bottom icon
		ofSetColor(255, 255, 255, (int)(255 * this->currentConnectIconAlpha));
		connectIcon.draw(this->width / 2, this->height - this->connectIcon.height / 2 - 0);

		ofPopStyle();

	}
	else
	{
		if (this->remoteTotem)
		{
			this->cylinderDisplay->draw();
		}

		ofPushStyle();
		ofSetRectMode(OF_RECTMODE_CENTER);
		this->muteIcon.draw(this->muteIconCenterX, MINI_SELFIE_TOP_MARGIN + this->muteIcon.getHeight() / 2);
		this->hangupIcon.draw(this->hangupIconCenterX, MINI_SELFIE_TOP_MARGIN + this->hangupIcon.getHeight() / 2);
		ofPopStyle();

		DrawSelfie();

		this->networkDisplay.draw();
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
	int selfieWidth = (int)(this->currentSelfieWidth);
	int selfieHeight = selfieWidth / ratio;
	int selfieX = (this->width - selfieWidth) / 2;
	int selfieY = (int)(this->currentSelfieYPosition);

	ofPushStyle();
	ofSetColor(0x09, 0x22, 0x36, 255 * (1.0 - currentConnectIconAlpha)); // Selfie margin color with alpha for animations
	ofRect(-(selfieX + SELFIE_FRAME_MARGIN + selfieWidth), selfieY - SELFIE_FRAME_MARGIN, selfieWidth + SELFIE_FRAME_MARGIN * 2, selfieHeight + SELFIE_FRAME_MARGIN * 2);
	ofPopStyle();

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
void ofRemoteApp::mousePressed(int x, int y, int button)
{
	if (this->state == UISTATE_INTRO)
	{
		if (button == 0)
		{
			this->playlist.addKeyFrame(Action::tween(TIME_INTRO_TRANSITION, &this->currentConnectIconAlpha, 0));
			this->playlist.addToKeyFrame(Action::tween(TIME_INTRO_TRANSITION, &this->currentSelfieYPosition, MINI_SELFIE_TOP_MARGIN));
			this->playlist.addToKeyFrame(Action::tween(TIME_INTRO_TRANSITION, &this->currentSelfieWidth, MINI_SELFIE_WIDTH));

			auto iconOffsetStart = MINI_SELFIE_WIDTH / 2 - this->muteIcon.getWidth() / 2;
			auto iconOffsetEnd = MINI_SELFIE_WIDTH / 2 + ICON_MARGIN + this->muteIcon.getWidth() / 2;
			this->hangupIconCenterX = this->width / 2 - iconOffsetStart;
			this->muteIconCenterX = this->width / 2 + iconOffsetStart;
			this->playlist.addKeyFrame(Action::tween(TIME_INTRO_ICONS_APPEAR / 2, &this->currentHangupMuteIconAlpha, 1));
			this->playlist.addToKeyFrame(Action::tween(TIME_INTRO_ICONS_APPEAR, &this->hangupIconCenterX, this->width / 2 - iconOffsetEnd));
			this->playlist.addToKeyFrame(Action::tween(TIME_INTRO_ICONS_APPEAR, &this->muteIconCenterX, this->width / 2 + iconOffsetEnd));

			this->playlist.addKeyFrame(Action::event(this, INTRO_TRANSITION_COMPLETE_EVENT));
		}
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

// ********************************************************************************************************************
void ofRemoteApp::onKeyframe(ofxPlaylistEventArgs& args)
{
	if (args.message == INTRO_TRANSITION_COMPLETE_EVENT)
	{
		this->state = UISTATE_MAIN;
	}
}