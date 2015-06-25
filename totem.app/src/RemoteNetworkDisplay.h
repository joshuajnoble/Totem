#pragma once

#include "ofMain.h"
#include "ofxPlaylist\src\ofxPlaylist.h"
#include "RemoteVideoInfo.h"

class RemoteNetworkDisplay
{
private:
	ofRectangle viewport;
	ofRectangle drawRegion;
	ofRectangle SingleVideoSize;
	ofRectangle DoubleVideoSize;

	std::vector<ofPtr<CroppedDrawable>> videoSources;

	ofRectangle currentVideoPosition1;
	ofRectangle currentVideoPosition2;
	float video1Alpha;
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

	bool AddVideoSource(ofPtr<CroppedDrawable> source);
	bool RemoveVideoSource(ofPtr<CroppedDrawable> source);

	void onKeyframe(ofxPlaylistEventArgs& args);
	bool CanModify();
};

