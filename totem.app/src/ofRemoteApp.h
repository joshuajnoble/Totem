#pragma once

#include "ofMain.h"
#include "ofTexture.h"
#include "ofxCv.h"
#include "ofxOsc.h"
#include "ofxGui\src\ofxGui.h"
#include "ofxPlaylist\src\ofxPlaylist.h"
#include "CylinderDisplay.h"
#include "VideoCaptureAppBase.h"
#include "RemoteVideoInfo.h"
#include "StickyTimer.h"
#include "RemoteNetworkDisplay.h"

class ofRemoteApp : public VideoCaptureAppBase
{
private:
	ofxPlaylist playlist;
	enum UISTATE { UISTATE_STARTUP, UISTATE_INTRO, UISTATE_INTRO_TO_MAIN, UISTATE_MAIN };

	float scale = 1.0f;
	int width, height;

	float muteIconCenterX;
	float hangupIconCenterX;
	float currentHangupMuteIconAlpha;
	float currentCylinderBarnDoorPosition;
	float curentSelfieMarginAlpha;
	float currentConnectIconAlpha;
	bool isAnimatingConnectIconAlpha;
	bool canShowRemotes;
	bool doneCylinderWelcome;
	bool isInCall;

	ofRectangle introSelfieRegion;
	ofRectangle miniSelfieRegion;
	ofRectangle connectIconRegion;
	ofRectangle currentSelfieRegion;

	ofPtr<CylinderDisplay> cylinderDisplay;
	ofPtr<RemoteVideoInfo> totemSource;
	ofImage connectIcon;
	ofImage muteIcon;
	ofImage hangupIcon;

	RemoteNetworkDisplay networkDisplay;
	UISTATE state;

	void TransitionTo_UISTATE_STARTUP();
	void TransitionTo_UISTATE_INTRO();
	void TransitionTo_UISTATE_MAIN();
	void RemoveRemoteVideoSource(const RemoteVideoInfo& video);

	void DrawSelfie();
	void WelcomeSequenceComplete();
	virtual void Handle_ClientConnected(RemoteVideoInfo& remote);
	virtual void Handle_ClientDisconnected(RemoteVideoInfo& remote);

	int networkInterfaceId;

public:
	//----------------------------------------
	/* standard openFrameworks app stuff */
	void earlyinit(int networkInterfaceId, int width, int height);
	void setup();
	virtual void update();
	void draw();
	virtual void exit();
	void keyPressed(int key);
	//void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void onKeyframe(ofxPlaylistEventArgs& args);

	virtual void Handle_ClientStreamAvailable(RemoteVideoInfo& remote);
	void NewConnection(const RemoteVideoInfo& remote);

	virtual int displayWidth() const;
	virtual int displayHeight() const;
};
