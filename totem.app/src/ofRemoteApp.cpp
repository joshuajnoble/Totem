#include "ofRemoteApp.h"
#include "ofxGstRTPClientAsVideoSource.h"
#include "Utils.h"
#include "..\..\SharedCode\VideoConverters.h"
#include "..\..\SharedCode\ofxFFmpegVideoReceiver.h"

//#define SHOW_FPS

#define CYLINDER_PIECE_WIDTH 44
#define CYLINDER_PIECE_HEIGHT 2
#define PIECE_TEXCOORD_WIDTH 720
#define NEO_PIXELS_COUNT 45

using namespace Playlist;

namespace
{
	const float WAITING_ROTATION = 0.0;
	const float DEFAULT_ROTATION = 0.0f;
	const float SHIFTED_OFFSET = 25.0f;

	const float TIME_INTRO_TRANSITION = 750.0f;
	const float TIME_INTRO_ICONS_APPEAR = 500.0f;
	const float TIME_INTRO_CONNECT_ICON_APPEARS = 1000.0f;
	const float TIME_WELCOME_BARNDOORS_OPEN = 2000.0f;
	const ofColor BACKGROUND_COLOR = ofColor(0x08, 0x21, 0x35);

	int SELFIE_FRAME_MARGIN;
	int ICON_MARGIN;
	int ICON_SIZE;

	const string INTRO_TRANSITION_COMPLETE_EVENT = "INTRO_TRANSITION_COMPLETE_EVENT";
	const string CurrentConnectIconAlpha_COMPLETE_EVENT = "CurrentConnectIconAlpha_COMPLETE_EVENT";
	const string CylinderDisplay_WELCOME_COMPLETE_EVENT = "CylinderDisplay:WELCOME_COMPLETE_EVENT";
	const string OpenBarndoors_COMPLETE_EVENT = "OpenBarndoors_COMPLETE_EVENT";
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

	cylinderCache.allocate(this->width, this->height);

#ifdef SHOW_FPS
	ofSetVerticalSync(false);
#else
	ofSetFrameRate(30);
	ofSetVerticalSync(true);
#endif
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

	auto totem = this->totemSource();
	if (this->state == UISTATE_MAIN && totem && totem->peerStatus.isConnectedToSession && !this->doneCylinderWelcome)
	{
		this->doneCylinderWelcome = true;
		this->currentCylinderBarnDoorPosition = 0.33;
		this->playlist.addKeyFrame(Action::tween(TIME_WELCOME_BARNDOORS_OPEN, &this->currentCylinderBarnDoorPosition, 1));
		this->playlist.addKeyFrame(Action::event(this, OpenBarndoors_COMPLETE_EVENT));
	}

	if (!this->isInCall && this->state == UISTATE_INTRO && !this->isAnimatingConnectIconAlpha)
	{
		if (totem && !this->currentConnectIconAlpha)
		{
			this->isAnimatingConnectIconAlpha = true;
			this->playlist.addKeyFrame(Action::tween(TIME_INTRO_CONNECT_ICON_APPEARS, &this->currentConnectIconAlpha, 1));
			this->playlist.addKeyFrame(Action::event(this, CurrentConnectIconAlpha_COMPLETE_EVENT));
		}
		else if (!totem && this->currentConnectIconAlpha == 1.0)
		{
			this->isAnimatingConnectIconAlpha = true;
			this->playlist.addKeyFrame(Action::tween(TIME_INTRO_CONNECT_ICON_APPEARS, &this->currentConnectIconAlpha, 0));
			this->playlist.addKeyFrame(Action::event(this, CurrentConnectIconAlpha_COMPLETE_EVENT));
		}
	}

	UpdateTotemViewAngle();

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

	if (this->cylinderDisplay && this->state == UISTATE_MAIN)
	{
		auto totem = this->totemSource();
		if (totem && this->cylinderDisplay)
		{
			UpdateTotemViewAngle();
			this->cylinderDisplay->update();
			if (totem->remoteVideoSource->isVideoFrameNew() || this->cylinderDisplay->IsDirty())
			{
				this->cylinderCache.begin();
				ofClear(0, 0, 0, 0);
				this->cylinderDisplay->draw();
				this->cylinderCache.end();
			}
		}

		cylinderCache.draw(0, 0);
	
		// Only draw the doors if they are on-screen.
		if (this->currentCylinderBarnDoorPosition != 0)
		{
			// Draw the animated "barn doors" that reveal the cylinder
			ofPushStyle();
			ofSetRectMode(OF_RECTMODE_CORNER);
			int barndoorWidth = this->displayWidth() / 2;

			// currentCylinderBarnDoorPosition, 1 is fully open while 0 is fully opeclosed
			int barndoorOffsetX = (int)(this->currentCylinderBarnDoorPosition * barndoorWidth);

			ofSetColor(BACKGROUND_COLOR);
			ofRect(-barndoorOffsetX, 0, barndoorWidth, this->displayHeight());
			ofRect(this->displayWidth() / 2 + barndoorOffsetX, 0, barndoorWidth, this->displayHeight());
			ofPopStyle();
		}
	}


	DrawSelfie();

#ifdef _DEBUGX
	if (this->cylinderDisplay)
	{
		auto source = this->cylinderDisplay->getTotemVideoSource();
		if (source)
		{
			ofPushMatrix();
			ofScale(0.5, 0.5);
			source->draw(0, 0);
			ofPopMatrix();
		}
	}
#endif

	if (this->canShowRemotes && this->networkDisplay.VideoCount())
	{
		this->networkDisplay.draw();
	}

	//for (int i = 0; i < this->peers.size(); ++i)
	//{
	//	auto peer = this->peers[i];
	//	if (peer.remoteVideoSource && peer.remoteVideoSource->isConnected())
	//	{
	//		auto image = peer.remoteVideoSource->getVideoImage();
	//		image.draw(0, i * (image.getWidth() + 10));
	//	}
	//}

#ifdef SHOW_FPS
	ofDrawBitmapString(ofToString(ofGetFrameRate()), 10, 10);
#endif
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
void ofRemoteApp::NewConnection(const RemoteVideoInfo& remote)
{
	if (remote.peerStatus.isTotem)
	{
		if (!this->cylinderDisplay)
		{
			this->cylinderDisplay.reset(new CylinderDisplay());
			this->cylinderDisplay->initCylinderDisplay(this->width, this->height);
			this->cylinderDisplay->SetViewAngle(WAITING_ROTATION);
			this->currentTotemAngle = WAITING_ROTATION;
			this->cylinderDisplay->setTotemVideoSource(remote.videoDraws, remote.peerStatus.videoWidth, remote.peerStatus.videoHeight);
		}
	}
	else if (!remote.peerStatus.isSurfaceHub)
	{
		this->networkDisplay.AddVideoSource(remote.videoCroppable);
	}
}


// ********************************************************************************************************************
void ofRemoteApp::RemoveRemoteVideoSource(const RemoteVideoInfo& video)
{
	this->networkDisplay.RemoveVideoSource(video.videoCroppable);
}


// ********************************************************************************************************************
void ofRemoteApp::keyPressed(int key)
{
	//auto videoCount = this->networkDisplay.VideoCount();
	//if (key == 'q' && videoCount < 2)
	//{
	//	RemoteVideoInfo remote;
	//	remote.clientId = "remotePeerImpersonator";
	//	remote.source = ofPtr<CroppedDrawable>(new CroppedDrawableVideoDraws(this->videoSource));
	//	remote.width = this->videoSource->getWidth();
	//	remote.height = this->videoSource->getHeight();
	//	remote.isTotem = false;
	//	remote.hasLiveFeed = true;

	//	NewConnection(remote, this->videoSource);
	//}
	//else if (key == 'a' && videoCount > 0)
	//{
	//	this->networkDisplay.RemoveFirstVideoSource();
	//}
	//else if (key == 'z' && videoCount > 0)
	//{
	//	this->networkDisplay.RemoveSecondVideoSource();
	//}
}


// ********************************************************************************************************************
void ofRemoteApp::mousePressed(int x, int y, int button)
{
	auto totem = this->totemSource();

	if (this->state == UISTATE_INTRO && !this->isInCall)
	{
		if (button == 0)
		{
			// Did they click on the connect icon?
			if (totem && this->connectIconRegion.inside(x, y))
			{
				TransitionTo_UISTATE_MAIN();
			}
		}
	}
	else if (this->state == UISTATE_MAIN)
	{
		if (button == 0)
		{
			// Did they click on the hangup icon?
			ofRectangle hangupIconRegion;
			hangupIconRegion.setFromCenter((int)this->hangupIconCenterX, (int)this->miniSelfieRegion.y + ICON_SIZE / 2, ICON_SIZE, ICON_SIZE);
			if (totem && hangupIconRegion.inside(x, y))
			{
				TransitionTo_UISTATE_INTRO();
			}
		}
	}
}


// ********************************************************************************************************************
void ofRemoteApp::mouseReleased(int x, int y, int button)
{
	if (button == 0 && this->cylinderDisplay && this->cylinderDisplay->IsDragging())
	{
		this->cylinderDisplay->DragEnd(ofPoint(x, y));
	}
}

// ********************************************************************************************************************
void ofRemoteApp::mouseDragged(int x, int y, int button)
{
	if (button == 0 && this->cylinderDisplay)
	{
		this->manualTotemAngle = true;

		if (!this->cylinderDisplay->IsDragging())
		{
			this->cylinderDisplay->DragStart(ofPoint(x, y));
		}
		else
		{
			this->cylinderDisplay->DragMove(ofPoint(x, y));
		}
	}
}


// ********************************************************************************************************************
void ofRemoteApp::TransitionTo_UISTATE_INTRO()
{
	this->state = UISTATE_INTRO;
	this->DisconnectSession();

	this->isInCall = false;
	this->doneCylinderWelcome = false;
	this->canShowRemotes = false;
	this->currentHangupMuteIconAlpha = 0;
	this->currentConnectIconAlpha = 0;
	this->currentSelfieRegion = this->introSelfieRegion;
	this->curentSelfieMarginAlpha = 0;
	this->isAnimatingConnectIconAlpha = false;
}

// ********************************************************************************************************************
void ofRemoteApp::TransitionTo_UISTATE_MAIN()
{
	this->isInCall = true;
	this->doneCylinderWelcome = false;
	this->canShowRemotes = false;
	this->ConnectToSession();

	// Transition the selfie view to the "rear view mirror" mode.
	this->isAnimatingConnectIconAlpha = true;
	this->playlist.addKeyFrame(Action::tween(TIME_INTRO_TRANSITION, &this->currentConnectIconAlpha, 0));
	this->playlist.addToKeyFrame(Action::tween(TIME_INTRO_TRANSITION, &this->curentSelfieMarginAlpha, 1));
	this->playlist.addToKeyFrame(Action::tween(TIME_INTRO_TRANSITION, &this->currentSelfieRegion.x, this->miniSelfieRegion.x));
	this->playlist.addToKeyFrame(Action::tween(TIME_INTRO_TRANSITION, &this->currentSelfieRegion.y, this->miniSelfieRegion.y));
	this->playlist.addToKeyFrame(Action::tween(TIME_INTRO_TRANSITION, &this->currentSelfieRegion.width, this->miniSelfieRegion.width));
	this->playlist.addToKeyFrame(Action::tween(TIME_INTRO_TRANSITION, &this->currentSelfieRegion.height, this->miniSelfieRegion.height));

	// Reveal the in-conversation icons
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
	this->playlist.clear();
	this->state = UISTATE_STARTUP;
	this->DisconnectSession();
}

void ofRemoteApp::WelcomeSequenceComplete()
{
	this->canShowRemotes = true;
}

// ********************************************************************************************************************
void ofRemoteApp::Handle_ClientConnected(RemoteVideoInfo& remote)
{
	NewConnection(remote);

	if (remote.peerStatus.isTotem && this->cylinderDisplay)
	{
		this->currentTotemAngle = DEFAULT_ROTATION;
		this->cylinderDisplay->SetViewAngle(DEFAULT_ROTATION, false);
	}
}

// ********************************************************************************************************************
void ofRemoteApp::Handle_ClientDisconnected(RemoteVideoInfo& remote)
{
	if (remote.peerStatus.isTotem)
	{
		this->cylinderDisplay.reset();
		TransitionTo_UISTATE_STARTUP();
	}
	else if (!remote.peerStatus.isSurfaceHub)
	{
		// TODO: This could fail (if it is already animating), so we should queue this up or something.
		RemoveRemoteVideoSource(remote);
	}
}

// ********************************************************************************************************************
void ofRemoteApp::Handle_ClientAngleChanged(RemoteVideoInfo& remote)
{
	auto totem = totemSource();
	if (totem)
	{
		if (int(this->currentTotemAngle) % 360 == remote.peerStatus.totemSourceAngle % 360) return;

		if (std::abs(this->currentTotemAngle - remote.peerStatus.totemSourceAngle) > 180)
		{
			this->currentTotemAngle = remote.peerStatus.totemSourceAngle + 360;
		}
		else
		{
			this->currentTotemAngle = remote.peerStatus.totemSourceAngle;
		}

		UpdateTotemViewAngle();
	}
}

// ********************************************************************************************************************
void ofRemoteApp::onKeyframe(ofxPlaylistEventArgs& args)
{
	if (args.message == INTRO_TRANSITION_COMPLETE_EVENT)
	{
		this->state = UISTATE_MAIN;
		this->isAnimatingConnectIconAlpha = false;
	}
	else if (args.message == CurrentConnectIconAlpha_COMPLETE_EVENT)
	{
		this->isAnimatingConnectIconAlpha = false;
	}
	else if (args.message == OpenBarndoors_COMPLETE_EVENT)
	{
		this->cylinderDisplay->DoWelcome(CylinderDisplay_WELCOME_COMPLETE_EVENT);
	}
	else if (args.message == CylinderDisplay_WELCOME_COMPLETE_EVENT)
	{
		this->currentTotemAngle = CylinderDisplay::NormalizeAngle(this->cylinderDisplay->GetViewAngle());
		this->cylinderDisplay->SetViewAngle(this->currentTotemAngle, false);
		this->canShowRemotes = true;
	}
}

RemoteVideoInfo* ofRemoteApp::totemSource()
{
	auto found = std::find_if(this->peers.begin(), this->peers.end(), [](RemoteVideoInfo& x)->bool { return x.peerStatus.isTotem; });
	return found == this->peers.end() ? nullptr : &(*found);
}

void ofRemoteApp::UpdateTotemViewAngle()
{
	if (this->canShowRemotes && this->cylinderDisplay && this->totemSource() && !this->manualTotemAngle)
	{
		auto angle = this->currentTotemAngle;
		if (this->networkDisplay.VideoCount())
		{
			angle += SHIFTED_OFFSET;
		}

		this->cylinderDisplay->SetViewAngle(angle, true);
	}
}