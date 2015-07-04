#include "ofRemoteApp.h"
#include "ofxGstRTPClientAsVideoSource.h"
#include "Utils.h"

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
	const float TIME_INTRO_CONNECT_ICON_APPEARS = 1000.0f;
	const ofColor BACKGROUND_COLOR = ofColor(0x08, 0x21, 0x35);

	int SELFIE_FRAME_MARGIN;
	int ICON_MARGIN;
	int ICON_SIZE;

	const string INTRO_TRANSITION_COMPLETE_EVENT = "INTRO_TRANSITION_COMPLETE_EVENT";
	const string CurrentConnectIconAlpha_COMPLETE_EVENT = "CurrentConnectIconAlpha_COMPLETE_EVENT";
}


// ********************************************************************************************************************
void ofRemoteApp::earlyinit(int netid, int w, int h)
{
	this->width = w;
	this->height = h;

	this->networkInterfaceId = netid;
	auto remoteViewAreaX = roundf((.50 + .1667f) * this->width);
	auto remoteViewAreaY = 0;
	auto remoteViewAreaWidth = this->width - remoteViewAreaX;
	auto remoteViewAreaHeight = this->height;

	SELFIE_FRAME_MARGIN = (int)roundf(this->displayHeight() * 0.0052f);
	ICON_SIZE = (int)roundf(this->displayWidth() * 0.0365f);
	ICON_MARGIN = (int)roundf(this->displayWidth() * 0.013f);

	this->miniSelfieRegion.setFromCenter(
		this->width / 2,
		roundf(this->height * 0.025f),
		(int)roundf(this->height * (0.12 * 2) * (10.0 / 16.0)),
		(int)roundf(this->height * (0.12 * 2)));
	this->miniSelfieRegion.y = 0 + (int)roundf(this->height * 0.03);

	this->introSelfieRegion.setFromCenter(
		this->width / 2,
		this->height / 2,
		(int)roundf(this->height * (0.4 * 2) * (10.0 / 16.0)),
		(int)roundf(this->height * (0.4 * 2)));
	this->introSelfieRegion.y = (this->height - this->introSelfieRegion.getHeight()) / 3;

	auto bottomCenterY = this->introSelfieRegion.getBottom() + (this->height - this->introSelfieRegion.getBottom()) / 2;
	connectIconRegion.setFromCenter(this->width / 2, bottomCenterY, ICON_SIZE, ICON_SIZE);

	this->networkDisplay.initializeRemoteNetworkDisplay(ofRectangle(remoteViewAreaX, remoteViewAreaY, remoteViewAreaWidth, remoteViewAreaHeight));
	this->state = UISTATE_STARTUP;
}


// ********************************************************************************************************************
void ofRemoteApp::setup()
{
	ofxKeyframeAnimRegisterEvents(this);

	VideoCaptureAppBase::setup(this->networkInterfaceId);

	connectIcon.loadImage("call.png");
	hangupIcon.loadImage("hangup.png");
	muteIcon.loadImage("mute.png");

	ofSetVerticalSync(false);

	this->streamManager.broadcastVideoBitrate = 4000;
}


// ********************************************************************************************************************
void ofRemoteApp::update()
{
	this->playlist.update();

	VideoCaptureAppBase::update();

	// Wait in the startup state until we confirm a local video source to broadcast
	if (this->state == UISTATE_STARTUP && this->videoSource)
	{
		TransitionTo_UISTATE_INTRO();
	}

	if (this->state == UISTATE_INTRO && this->totemSource && !this->isAnimatingConnectIconAlpha && !this->currentConnectIconAlpha)
	{
		this->isAnimatingConnectIconAlpha = true;
		this->playlist.addKeyFrame(Action::tween(TIME_INTRO_CONNECT_ICON_APPEARS, &this->currentConnectIconAlpha, 1));
		this->playlist.addKeyFrame(Action::event(this, CurrentConnectIconAlpha_COMPLETE_EVENT));
	}
	else if (this->state == UISTATE_INTRO && !this->totemSource && !this->isAnimatingConnectIconAlpha && this->currentConnectIconAlpha == 1.0)
	{
		this->isAnimatingConnectIconAlpha = true;
		this->playlist.addKeyFrame(Action::tween(TIME_INTRO_CONNECT_ICON_APPEARS, &this->currentConnectIconAlpha, 0));
		this->playlist.addKeyFrame(Action::event(this, CurrentConnectIconAlpha_COMPLETE_EVENT));
	}

	if (this->totemSource)
	{
		this->cylinderDisplay->update();
	}

	this->networkDisplay.update();
}


// ********************************************************************************************************************
void ofRemoteApp::draw()
{
	ofBackground(BACKGROUND_COLOR);
	ofEnableAlphaBlending();

	ofPushStyle();

	// Draw icons first so they animate out from behind the selfie
	ofSetRectMode(OF_RECTMODE_CENTER);
	ofSetColor(255, 255, 255, (int)(255 * this->currentHangupMuteIconAlpha));
	this->muteIcon.draw(this->muteIconCenterX, this->miniSelfieRegion.y + ICON_SIZE / 2, ICON_SIZE, ICON_SIZE);
	this->hangupIcon.draw(this->hangupIconCenterX, this->miniSelfieRegion.y + ICON_SIZE / 2, ICON_SIZE, ICON_SIZE);

	if (this->currentConnectIconAlpha)
	{
		ofSetRectMode(OF_RECTMODE_CORNER);
		ofSetColor(255, 255, 255, (int)(255 * this->currentConnectIconAlpha));
		connectIcon.draw(this->connectIconRegion);
	}

	ofPopStyle();

	if (this->state == UISTATE_MAIN && this->cylinderDisplay)
	{
		this->cylinderDisplay->draw();

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


	DrawSelfie();

	this->networkDisplay.draw();
}


// ********************************************************************************************************************
void ofRemoteApp::DrawSelfie()
{
	// Draw the source video in a small window
	ofPushStyle();

	ofSetColor(BACKGROUND_COLOR, 255 * this->curentSelfieMarginAlpha); // Selfie margin color with alpha for animations
	ofRectangle borderRect;
	borderRect.setFromCenter(
		this->currentSelfieRegion.getCenter(),
		this->currentSelfieRegion.width + SELFIE_FRAME_MARGIN * 2,
		this->currentSelfieRegion.height + SELFIE_FRAME_MARGIN * 2);

	ofSetRectMode(OF_RECTMODE_CORNER);
	ofRect(borderRect);

	ofPushMatrix();

	// Flip the X axis, so we see it mirrored.
	// This means all of the following x values need to be negated and offset.
	ofScale(-1.0, 1.0);
	ofTranslate(-this->width, 0); // This deals with the negated x + offset issue.

	ofSetColor(255);
	Utils::DrawCroppedToFit(*this->videoSource, this->currentSelfieRegion);

	ofPopMatrix();
	ofPopStyle();
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
void ofRemoteApp::NewConnection(const RemoteVideoInfo& remote, ofPtr<ofBaseVideoDraws> video)
{
	this->remoteVideoSources.push_back(remote);

	if (remote.isTotem)
	{
		this->totemSource.reset(new RemoteVideoInfo(remote));

		this->cylinderDisplay.reset(new CylinderDisplay());
		this->cylinderDisplay->initCylinderDisplay(this->width, this->height);
		this->cylinderDisplay->SetViewAngle(DEFAULT_ROTATION);
		this->cylinderDisplay->setTotemVideoSource(video);
	}
	else
	{
		this->networkDisplay.AddVideoSource(remote.source);
		if (this->totemSource)
		{
			this->cylinderDisplay->SetViewAngle(DEFAULT_ROTATION + SHIFTED_OFFSET);
		}
	}
}


// ********************************************************************************************************************
void ofRemoteApp::RemoveRemoteVideoSource(const RemoteVideoInfo& video)
{
	this->networkDisplay.RemoveVideoSource(video.source);

	int videoCount = 0; // Count all non-totem video source while looking for this one
	std::vector<RemoteVideoInfo>::iterator found = this->remoteVideoSources.end();
	for (auto iter = this->remoteVideoSources.begin(); iter != this->remoteVideoSources.end(); ++iter)
	{
		if (!iter->isTotem) ++videoCount;
		if (iter->clientId == video.clientId)
		{
			found = iter;
		}
	}

	if (found != this->remoteVideoSources.end())
	{
		this->remoteVideoSources.erase(found);
		if (videoCount == 1 && this->cylinderDisplay)
		{
			this->cylinderDisplay->SetViewAngle(DEFAULT_ROTATION);
		}
	}
}


// ********************************************************************************************************************
void ofRemoteApp::keyPressed(int key)
{
	if (this->networkDisplay.CanModify())
	{
		int videoCount = 0;
		for (auto iter = this->remoteVideoSources.begin(); iter != this->remoteVideoSources.end(); ++iter)
		{
			if (!iter->isTotem) ++videoCount;
		}

		if (key == 'a' && videoCount < 2)
		{
			RemoteVideoInfo remote;
			remote.clientId = "remotePeerImpersonator";
			remote.source = ofPtr<CroppedDrawable>(new CroppedDrawableVideoDraws(this->videoSource));
			remote.width = this->videoSource->getWidth();
			remote.height = this->videoSource->getHeight();
			remote.isTotem = false;

			NewConnection(remote, this->videoSource);
			if (this->cylinderDisplay) this->cylinderDisplay->SetViewAngle(DEFAULT_ROTATION + SHIFTED_OFFSET);
		}
		else if (key == 'z' && videoCount > 0)
		{
			// Get the last non-totem video that is in our list
			auto iter = this->remoteVideoSources.rbegin();
			for (; iter != this->remoteVideoSources.rend(); ++iter)
			{
				if (!iter->isTotem) break;
			}

			auto video = *iter;
			RemoveRemoteVideoSource(video);
		}
	}
}


// ********************************************************************************************************************
void ofRemoteApp::mousePressed(int x, int y, int button)
{
	if (this->state == UISTATE_INTRO)
	{
		if (button == 0)
		{
			// Did they click on the connect icon?
			if (this->totemSource && this->connectIconRegion.inside(x, y))
			{
				TransitionTo_UISTATE_MAIN();
			}
		}
	}
}

// ********************************************************************************************************************
void ofRemoteApp::TransitionTo_UISTATE_INTRO()
{
	this->state = UISTATE_INTRO;

	this->currentHangupMuteIconAlpha = 0;
	this->currentConnectIconAlpha = 0;
	this->currentSelfieRegion = this->introSelfieRegion;
	this->curentSelfieMarginAlpha = 0;
	this->isAnimatingConnectIconAlpha = false;
}

// ********************************************************************************************************************
void ofRemoteApp::TransitionTo_UISTATE_MAIN()
{
	this->state = UISTATE_INTRO_TO_MAIN;

	this->playlist.addKeyFrame(Action::tween(TIME_INTRO_TRANSITION, &this->currentConnectIconAlpha, 0));
	this->playlist.addToKeyFrame(Action::tween(TIME_INTRO_TRANSITION, &this->curentSelfieMarginAlpha, 1));
	this->playlist.addToKeyFrame(Action::tween(TIME_INTRO_TRANSITION, &this->currentSelfieRegion.x, this->miniSelfieRegion.x));
	this->playlist.addToKeyFrame(Action::tween(TIME_INTRO_TRANSITION, &this->currentSelfieRegion.y, this->miniSelfieRegion.y));
	this->playlist.addToKeyFrame(Action::tween(TIME_INTRO_TRANSITION, &this->currentSelfieRegion.width, this->miniSelfieRegion.width));
	this->playlist.addToKeyFrame(Action::tween(TIME_INTRO_TRANSITION, &this->currentSelfieRegion.height, this->miniSelfieRegion.height));

	auto iconOffsetStart = this->miniSelfieRegion.width / 2 - ICON_SIZE / 2;
	auto iconOffsetEnd = this->miniSelfieRegion.width / 2 + ICON_MARGIN + ICON_SIZE / 2;
	this->hangupIconCenterX = this->width / 2 - iconOffsetStart;
	this->muteIconCenterX = this->width / 2 + iconOffsetStart;
	this->playlist.addKeyFrame(Action::tween(TIME_INTRO_ICONS_APPEAR / 2, &this->currentHangupMuteIconAlpha, 1.0));
	this->playlist.addToKeyFrame(Action::tween(TIME_INTRO_ICONS_APPEAR, &this->hangupIconCenterX, this->width / 2 - iconOffsetEnd));
	this->playlist.addToKeyFrame(Action::tween(TIME_INTRO_ICONS_APPEAR, &this->muteIconCenterX, this->width / 2 + iconOffsetEnd));

	this->playlist.addKeyFrame(Action::event(this, INTRO_TRANSITION_COMPLETE_EVENT));
}

// ********************************************************************************************************************
void ofRemoteApp::TransitionTo_UISTATE_STARTUP()
{
	// TODO: Transition with animations
	// TODO: Mute and hide or reset the remote clients that are still connected ... or maybe just pause our video, so others don't recieve it yet.
	this->state = UISTATE_STARTUP;
}

// ********************************************************************************************************************
void ofRemoteApp::Handle_ClientConnected(RemoteVideoInfo& remote)
{
	auto video = ofPtr<ofBaseVideoDraws>(new ofxGstRTPClientAsVideoSource(remote.netClient, remote.width, remote.height));
	NewConnection(remote, video);
}

// ********************************************************************************************************************
void ofRemoteApp::Handle_ClientDisconnected(RemoteVideoInfo& remote)
{
	if (remote.isTotem)
	{
		this->totemSource.reset();
		TransitionTo_UISTATE_STARTUP();
		this->cylinderDisplay.reset();
	}
	else
	{
		// TODO: This could fail (if it is already animating), so we should queue this up or something.
		RemoveRemoteVideoSource(remote);
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
	else if (args.message == CurrentConnectIconAlpha_COMPLETE_EVENT)
	{
		this->isAnimatingConnectIconAlpha = false;
	}
}