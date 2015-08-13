#pragma once

#include "ofMain.h"
#include "VideoCaptureAppBase.h"
#include "UdpDiscovery.h"

class ofSurfaceHubApp : public IVideoCaptureAppBase
{
private:
	class PeerInfo
	{
	public:
		std::string id;
		bool isTotem;
		ofImage profilePicture;
	};

	int networkInterfaceId;
	bool hasTotemConnected;
	UdpDiscovery udpDiscovery;
	ofImage presentation;
	ofImage connectButton;
	ofRectangle buttonPosition;
	std::vector<ofImage> profilePictures;
	std::vector<PeerInfo> peers;

	void PeerArrived(UdpDiscovery::RemotePeerStatus& peer);
	void PeerLeft(UdpDiscovery::RemotePeerStatus& peer);
	void SetupDiscovery();

	void mousePressed(int x, int y, int button);

public:
	void earlyinit(int networkInterfaceId);
	void draw();
	void setup();
	virtual void update();
	virtual void exit();
	virtual int displayWidth() const;
	virtual int displayHeight() const;
};
