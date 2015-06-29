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
	int width, height;

	float currentConnectIconAlpha;
	float muteIconCenterX;
	float hangupIconCenterX;
	float currentHangupMuteIconAlpha;
	float currentCylinderBarnDoorPosition;

	ofRectangle introSelfieRegion;
	ofRectangle miniSelfieRegion;
	ofRectangle currentSelfieRegion;

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

	bool isTotemInitialized;
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
	//void keyReleased(int key);
	//void mouseMoved(int x, int y);
	//void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	//void mouseReleased(int x, int y, int button);
	//void windowResized(int w, int h);
	void onKeyframe(ofxPlaylistEventArgs& args);

	void ImpersonateRemoteConnection(const string& clientId, ofPtr<ofBaseVideoDraws> source);

	virtual int displayWidth() const;
	virtual int displayHeight() const;
};
