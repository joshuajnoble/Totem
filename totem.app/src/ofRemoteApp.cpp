#include "ofRemoteApp.h"
//#include "WrapFboAsVideo.h"
#include "ofxGstRTPClientAsVideoSource.h"

#define CYLINDER_PIECE_WIDTH 44
#define CYLINDER_PIECE_HEIGHT 2
#define PIECE_TEXCOORD_WIDTH 720
#define NEO_PIXELS_COUNT 45

using namespace Playlist;

namespace
{
	const float WAITING_ROTATION = 270.0f; // TODO: Why is the "centered" spin icon at 270 deg and not 180 deg?
	const float DEFAULT_ROTATION = 0.0f;
	const float SHIFTED_OFFSET = 25.0f;
	
	const float TIME_INTRO_TRANSITION = 750.0f;
	const float TIME_INTRO_ICONS_APPEAR = 500.0f;
	const ofColor BACKGROUND_COLOR = ofColor(23, 38, 44);

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
	this->isTotemInitialized = false;
}


// ********************************************************************************************************************
void ofRemoteApp::setup()
{
	ofxKeyframeAnimRegisterEvents(this);

	VideoCaptureAppBase::setup();

	connectIcon.loadImage("icon_connect.png");
	hangupIcon.loadImage("icon_hangup.png");
	muteIcon.loadImage("icon_mute.png");

	ofSetVerticalSync(false);
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

	if (this->state == UISTATE_MAIN)
	{
		// TODO: This is a hack for the current state of the networking system, since we don't get any connect/disconnect events
		if (!this->isTotemInitialized && !this->remoteTotem)
		{
			// Look for a totem stream and use it if it exists
			for (auto iter = this->streamManager.clients.begin(); iter != this->streamManager.clients.end(); ++iter)
			{
				auto clientId = iter->first;
				auto client = iter->second;
				auto video = this->streamManager.remoteVideos[clientId];

				if (video->getWidth() / video->getHeight() >= 2)
				{
					auto videoSource = ofPtr<ofBaseVideoDraws>(new ofxGstRTPClientAsVideoSource(client, video->getWidth(), video->getHeight()));

					RemoteVideoInfo remote;
					remote.clientId = clientId;
					remote.source = video;
					remote.videoSource = videoSource;
					remote.isTotem = true;
					this->remoteVideoSources.push_back(remote);

					this->remoteTotem.reset(new RemoteVideoInfo(remote));
					this->remoteTotemClientId = remote.clientId;

					this->cylinderDisplay.reset(new CylinderDisplay());
					this->cylinderDisplay->initCylinderDisplay(1920, 1080);
					this->cylinderDisplay->SetViewAngle(DEFAULT_ROTATION);
					this->cylinderDisplay->setTotemVideoSource(videoSource);

					this->isTotemInitialized = true;
					break;
				}
			}
		}
	}

	if (this->remoteTotem)
	{
		this->cylinderDisplay->update();
	}

	this->networkDisplay.update();
}


// ********************************************************************************************************************
void ofRemoteApp::draw()
{
	ofBackground(BACKGROUND_COLOR);

	if (this->state == UISTATE_INTRO)
	{
		ofPushStyle();
		ofSetRectMode(OF_RECTMODE_CENTER);
		ofEnableAlphaBlending();

		// Draw icons first so they animate out from behind the selfie
		ofSetColor(255, 255, 255, (int)(255 * this->currentHangupMuteIconAlpha));
		this->muteIcon.draw(this->muteIconCenterX, MINI_SELFIE_TOP_MARGIN + this->muteIcon.getHeight() / 2);
		this->hangupIcon.draw(this->hangupIconCenterX, MINI_SELFIE_TOP_MARGIN + this->hangupIcon.getHeight() / 2);

		//Now draw the bottom icon
		ofSetColor(255, 255, 255, (int)(255 * this->currentConnectIconAlpha));
		connectIcon.draw(this->width / 2, this->height - this->connectIcon.height / 2 - 0);

		ofSetRectMode(OF_RECTMODE_CORNER);
		DrawSelfie();

		ofPopStyle();

	}
	else
	{
		if (this->remoteTotem)
		{
			this->cylinderDisplay->draw();

			// TODO: remove this line and setup keyframe animations.
			// currentCylinderBarnDoorPosition, 1 is fully closed while 0 is fully open
			// The animation should start with 1/3 of the screen revealed and then follow open
			// That means animating from .66 to 0
			this->currentCylinderBarnDoorPosition = 0.00f;

			// Only draw the doors if they are on-screen.
			if (this->currentCylinderBarnDoorPosition != 0)
			{
				// Draw the animated "barn doors" that reveal the cylinder
				ofPushStyle();
				ofSetRectMode(OF_RECTMODE_CORNER);
				int barndoorWidth = this->displayWidth() / 2;

				// currentCylinderBarnDoorPosition, 1 is fully closed while 0 is fully open
				int barndoorOffsetX = (int)(this->currentCylinderBarnDoorPosition * barndoorWidth);

				ofSetColor(127, 127, 0); // TODO: should be the background color
				ofRect(-barndoorOffsetX, 0, barndoorWidth, this->displayHeight());
				ofSetColor(127, 0, 127); // TODO: this should be removed and just use the same color as set above
				ofRect(this->displayWidth() / 2 + barndoorOffsetX, 0, barndoorWidth, this->displayHeight());
				ofPopStyle();
			}
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
	ofSetColor(BACKGROUND_COLOR, 255 * (1.0 - currentConnectIconAlpha)); // Selfie margin color with alpha for animations
	ofRect(-(selfieX + SELFIE_FRAME_MARGIN + selfieWidth), selfieY - SELFIE_FRAME_MARGIN, selfieWidth + SELFIE_FRAME_MARGIN * 2, selfieHeight + SELFIE_FRAME_MARGIN * 2);
	ofPopStyle();

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
void ofRemoteApp::ImpesonateRemoteConnection(const string& clientId, ofPtr<ofBaseVideoDraws> video)
{
	RemoteVideoInfo remote;
	remote.clientId = clientId;
	remote.videoSource = video;
	remote.isTotem = video->getWidth() / video->getHeight() >= 2;
	this->remoteVideoSources.push_back(remote);

	if (remote.isTotem)
	{
		this->remoteTotem.reset(new RemoteVideoInfo(remote));

		this->remoteTotemClientId = remote.clientId;
		this->cylinderDisplay.reset(new CylinderDisplay());
		this->cylinderDisplay->initCylinderDisplay(1920, 1080);
		this->cylinderDisplay->SetViewAngle(DEFAULT_ROTATION);
		this->cylinderDisplay->setTotemVideoSource(video);
	}
	else
	{
		//this->networkDisplay.AddVideoSource(video);
		if (this->remoteTotem)
		{
			this->cylinderDisplay->SetViewAngle(DEFAULT_ROTATION + SHIFTED_OFFSET);
		}
	}
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
			this->playlist.addKeyFrame(Action::tween(TIME_INTRO_ICONS_APPEAR / 2, &this->currentHangupMuteIconAlpha, 1.0));
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
		this->cylinderDisplay->setTotemVideoSource(ofPtr<ofBaseVideoDraws>(new ofxGstRTPClientAsVideoSource(remote.netClient, remote.source->getWidth(), remote.source->getHeight())));
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