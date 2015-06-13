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
	enum UISTATE { UISTATE_STARTUP, UISTATE_INTRO, UISTATE_MAIN };

	float scale = 1.0f;
	int width = 1920, height = 1080;
	
	float currentSelfieWidth;
	float currentSelfieYPosition;
	float currentConnectIconAlpha;
	float muteIconCenterX;
	float hangupIconCenterX;
	float currentHangupMuteIconAlpha;

	float rotateToPosition;
	ofPtr<CylinderDisplay> cylinderDisplay;
	std::string remoteTotemClientId;
	ofPtr<RemoteVideoInfo> remoteTotem;
	ofImage connectIcon;
	ofImage muteIcon;
	ofImage hangupIcon;

	RemoteNetworkDisplay networkDisplay;
	UISTATE state;

	void DrawSelfie();
	virtual void Handle_ClientConnected(RemoteVideoInfo& remote);
	virtual void Handle_ClientDisconnected(RemoteVideoInfo& remote);
	virtual void Handle_ClientStreamAvailable(RemoteVideoInfo& remote);

public:
	//----------------------------------------
	/* standard openFrameworks app stuff */
	void earlyinit();
	void setup();
	virtual void update();
	void draw();
	virtual void exit();
	void keyPressed(int key);
	//void keyReleased(int key);
	//void mouseMoved(int x, int y);
	//void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	//void mouseReleased(int x, int y, int button);
	//void windowResized(int w, int h);
	void onKeyframe(ofxPlaylistEventArgs& args);

	void RegisterTotemVideoSource(RemoteVideoInfo& remote);
	void RegisterRemoteVideoSource(RemoteVideoInfo& remote);

	virtual int displayWidth() const;
	virtual int displayHeight() const;
};
