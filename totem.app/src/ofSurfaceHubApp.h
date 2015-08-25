#pragma once

#include "ofMain.h"
#include "VideoCaptureAppBase.h"
#include "UdpDiscovery.h"

class ofSurfaceHubApp : public IVideoCaptureAppBase
{
private:
	class Person
	{
	public:
		std::string name;
		std::string title;
		ofImage avatar;
	};

	class PeerInfo
	{
	public:
		std::string id;
		bool isTotem;
		bool isReady;
		bool isConnectedToSession;
		Person person;
	};

	const int PROFILE_PADDING = 18;
	const int PROFILE_SIZE = 48;

	const ofRectangle PeopleRegion;
	const ofPoint PeopleMargin;
	ofTrueTypeFont StatusFont;
	ofTrueTypeFont TitleFont;
	ofTrueTypeFont NameFont;

	int networkInterfaceId;
	bool hasTotemConnected;
	bool hasSessionStarted;
	UdpDiscovery udpDiscovery;
	ofImage backgroundOpen;
	ofImage backgroundClosed;
	ofImage slide;
	ofImage avatarMask;
	ofImage iconPeople, iconTotem;
	ofRectangle buttonPosition;
	ofRectangle slidePositionOpen;
	ofRectangle slidePositionClosed;
	std::vector<Person> people;
	std::vector<PeerInfo> peers;

	void DrawPerson(Person&, float x = 0, float y = 0);
	std::vector<PeerInfo>::iterator GetPeerFromClientId(const string& clientId);

	void PeerArrived(UdpDiscovery::RemotePeerStatus& peer);
	void PeerLeft(UdpDiscovery::RemotePeerStatus& peer);
	void PeerLeftSession(UdpDiscovery::RemotePeerStatus& peer);
	void PeerJoinedSession(UdpDiscovery::RemotePeerStatus& peer);
	void PeerReadyChanged(UdpDiscovery::RemotePeerStatus& peer);	

	void SetupDiscovery();

	void mousePressed(int x, int y, int button);

public:
	ofSurfaceHubApp();
	void earlyinit(int networkInterfaceId);
	void draw();
	void setup();
	virtual void update();
	virtual void exit();
	virtual int displayWidth() const;
	virtual int displayHeight() const;
};
