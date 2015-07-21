#pragma once

#include "ofMain.h"
#include "VideoCaptureAppBase.h"
#include "ofxPlaylist\src\ofxPlaylist.h"
#include "ofxCv\src\ofxCv.h"
#include "ThreeSixtyUnwrap.h"
#include "..\..\sharedCode\StreamManager.h"
#include "TotemDisplay.h"

class ofTotemApp : public VideoCaptureAppBase
{
private:
	bool isInitialized = false;

	ofVec2f remotePosition, remoteScale;
	ofVec2f mainPosition, mainScale;
	double doubleM = 2560;
	bool isRemoteSource1Initialized;
	int networkInterfaceId;
	ofVideoPlayer cortanaPlayer;
	TotemDisplay totemDisplay;
	ofPtr<ofBaseVideoDraws> netImpersonate;
	bool cortanaLoadErrorDisplayed = false;

	virtual void Handle_ClientConnected(RemoteVideoInfo& remote);
	virtual void Handle_ClientDisconnected(RemoteVideoInfo& remote);

	void cortanaLoadClip(const string& clipName);
	void cortanaPlayIntro();
	void cortanaPlayIdle();

public:
	//----------------------------------------
	/* standard openFrameworks app stuff */
	void earlyinit(int networkInterfaceId);
	void setup();
	virtual void update();
	void draw();
	virtual void exit();
	void keyPressed  (int key);
	void onKeyframe(ofxPlaylistEventArgs& args);
	virtual int displayWidth() const;
	virtual int displayHeight() const;

	// Public Config
	ofPtr<ofBaseVideoDraws> rawSource;
	bool showOutput= false;

	void ImporsonateRemoteClient(ofPtr<ofBaseVideoDraws> source);
};
