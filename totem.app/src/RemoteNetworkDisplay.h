#pragma once

#include "ofMain.h"
#include "ofxPlaylist\src\ofxPlaylist.h"
#include "RemoteVideoInfo.h"

class RemoteNetworkDisplay
{
private:
	ofRectangle viewport;
	std::vector<ofPtr<ofFbo>> videoSources;

	float video1Left;
	float video1Height;
	float video1Alpha;

	float video2Top;
	float video2Alpha;

	ofxPlaylist playlist;
	bool animatingVideo1Entrance = false;
	bool animatingVideo2Entrance = false;
	bool animatingVideo1Exit = false;
	bool animatingVideo2Exit = false;

public:
	RemoteNetworkDisplay();
	~RemoteNetworkDisplay();

	void initializeRemoteNetworkDisplay(ofRectangle viewport);
	void update();
	void draw();

	bool AddVideoSource(ofPtr<ofFbo> source);
	bool RemoveVideoSource(ofPtr<ofFbo> source);

	void onKeyframe(ofxPlaylistEventArgs& args);
	bool CanModify();
};

