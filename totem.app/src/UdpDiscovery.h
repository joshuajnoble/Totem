#pragma once

#include "ofMain.h"
#include "ofxNetwork/src/ofxNetwork.h"
#include "ofxJSON/src/ofxJSON.h"
#include <Poco/Net/NetworkInterface.h>

class UdpDiscovery
{
public:
	struct RemotePeerStatus
	{
		std::string id;
		std::string ipAddress;
		float disconnectTime;
		int port;
		int videoWidth;
		int videoHeight;
		bool isTotem;
	};

private:
	const string version = string("2.0");
	Poco::Mutex portmutex;
	std::string myid;
	std::string broadcastAddress;

	ofxUDPManager sender;
	ofxUDPManager receiver;
	float nextSendTime;
	float broadcastDelay = 0.25f;
	float broadcastMissingDuration = 20.0f;
	char incomingMessage[1024];
	int videoWidth, videoHeight;
	bool isTotem;

	std::map<string, RemotePeerStatus> remoteClientMap;
	int discoveryBroadcastPort = 11000;
	int videoBroadcastPort = 11005;
	int AudioBroadcastPort = 11010;

	ofxJSONElement GetNetworkPayload(const std::string& action);
	void SendJsonPayload(const ofxJSONElement& jsonPayload);

	void HandleDiscovery(const ofxJSONElement& jsonPayload, const string& remoteAddress);
	void HandleDisconnect(const string& remoteId, bool isTimeout = false);

	Poco::Net::NetworkInterface interface;

public:
	virtual ~UdpDiscovery();

	ofEvent<RemotePeerStatus> peerArrivedEvent;
	ofEvent<RemotePeerStatus> peerReadyEvent;
	ofEvent<RemotePeerStatus> peerLeftEvent;

	void setup(int videoWidth, int videoHeight, int networkInterfaceId = -1, bool isTotem = false);
	void update();
	RemotePeerStatus GetPeerStatus(const std::string& peerId);
	Poco::Net::IPAddress GetLocalAddress();

	static Poco::Net::NetworkInterface::List GetAllNetworkInterfaces();
	static Poco::Net::IPAddress GetBroadcastAddress(Poco::Net::NetworkInterface interface);
	static std::string MACtoString(const std::vector<unsigned char>& mac, char delimter = ':');
};