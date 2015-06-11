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

enum NAV_STATE {
	USER_CONTROL, SYSTEM_CONTROL
};

enum USER_DISPLAY_STATE {
	UDS_SHOW_FEED, UDS_SHOW_REMOTE_USER, UDS_SHOW_L_LOCAL_USER, UDS_SHOW_BOTH_LOCAL_USER
};

class ofRemoteApp : public VideoCaptureAppBase
{
private:
	float scale = 1.0f;
	int width = 1920, height = 1080;

	void DrawSelfie();

	float rotateToPosition;
	CylinderDisplay cylinderDisplay;
	std::string remoteTotemClientId;
	ofPtr<ofBaseVideoDraws> remoteTotemSource;

	std::vector<ofPtr<RemoteVideoInfo>> remoteVideoSources;
	RemoteNetworkDisplay networkDisplay;

	virtual void Handle_ClientConnected(string connectionId, ofPtr<ofxGstRTPClient> client, ofPtr<ofFbo> clientVideo);
	virtual void Handle_ClientDisconnected(string connectionId);

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
	//void mousePressed(int x, int y, int button);
	//void mouseReleased(int x, int y, int button);
	//void windowResized(int w, int h);

	void RegisterTotemVideoSource(std::string clientId, ofPtr<ofBaseVideoDraws> source);
	ofPtr<RemoteVideoInfo> RegisterRemoteVideoSource(std::string clientId, ofPtr<ofBaseVideoDraws> source);

	virtual int displayWidth() const;
	virtual int displayHeight() const;
};
