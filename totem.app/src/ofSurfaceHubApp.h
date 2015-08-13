#pragma once

#include "ofMain.h"
#include "VideoCaptureAppBase.h"
#include "UdpDiscovery.h"

class ofSurfaceHubApp : public IVideoCaptureAppBase
{
private:
	int networkInterfaceId;
	UdpDiscovery udpDiscovery;

	void PeerArrived(UdpDiscovery::RemotePeerStatus& peer);
	void PeerLeft(UdpDiscovery::RemotePeerStatus& peer);
	void SetupDiscovery();

public:
	void earlyinit(int networkInterfaceId);
	void draw();
	void setup();
	virtual void update();
	virtual void exit();
	virtual int displayWidth() const;
	virtual int displayHeight() const;
};
