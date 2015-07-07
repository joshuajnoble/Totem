#pragma once

#include "ofMain.h"
#include "ofxPlaylist\src\ofxPlaylist.h"
#include "RemoteVideoInfo.h"

class RemoteNetworkDisplay
{
private:
	ofRectangle viewport;
	ofRectangle drawRegion;
	ofRectangle SingleVideoRegion;
	ofRectangle FirstVideoRegion;
	ofRectangle SecondVideoRegion;

	//enum VideoinfoState { INACTIVE = 0, ACTIVE = 1, REMOVING = 2, REMOVED = 3 };
	//struct VideoInfo
	//{
	//	ofPtr<CroppedDrawable> videoSource;
	//};

	std::vector<ofPtr<CroppedDrawable>> videoSources;

	enum ACTIVE_WINDOW_TYPE { WINDOW_TYPE_Single = 0, WINDOW_TYPE_First = 1, WINDOW_TYPE_Second = 2 };
	struct ActiveVideo
	{
		float alpha;
		ACTIVE_WINDOW_TYPE windowType;
		ofRectangle currentRegion;
		ofPtr<CroppedDrawable> videoSource;
		bool removing;
	};

	typedef std::vector<ofPtr<ActiveVideo>> activeVideoList;
	activeVideoList activeVideos;

	ofxPlaylist playlist;

	activeVideoList::iterator GetActiveWindwoByType(ACTIVE_WINDOW_TYPE windowType);
	void RemoveActiveVideo(ofPtr<CroppedDrawable> source);
	void AddActiveVideo(ofPtr<CroppedDrawable> source);

public:
	RemoteNetworkDisplay();
	~RemoteNetworkDisplay();

	void initializeRemoteNetworkDisplay(ofRectangle viewport);
	void update();
	void draw();

	void AddVideoSource(ofPtr<CroppedDrawable> source);
	void RemoveVideoSource(ofPtr<CroppedDrawable> source);
	void RemoveFirstVideoSource();
	void RemoveSecondVideoSource();

	size_t VideoCount() const { return this->videoSources.size(); }

	void onKeyframe(ofxPlaylistEventArgs& args);
};