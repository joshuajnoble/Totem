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
		int assignedLocalPort;
		int assignedRemotePort;
		float disconnectTime;
		int videoWidth;
		int videoHeight;
	};

private:
	const string version = string("1.0");
	Poco::Mutex portmutex;
	std::string myid;
	std::string broadcastAddress;

	ofxUDPManager sender;
	ofxUDPManager receiver;
	int broadcastPort = 10527;
	float nextSendTime;
	float broadcastDelay = 0.5f;
	float broadcastMissingDuration = 10.0f;
	char incomingMessage[1024];
	int videoWidth, videoHeight;

	std::map<string, RemotePeerStatus> remoteClientMap;
	int myNextPort = 12000;
	const int portIncrement = 10;

	ofxJSONElement GetNetworkPayload(const std::string& action);
	void SendJsonPayload(const ofxJSONElement& jsonPayload);

	void HandleDiscovery(const ofxJSONElement& jsonPayload, const string& remoteAddress);
	void HandleDisconnect(const string& remoteId, bool isTimeout = false);

public:
	virtual ~UdpDiscovery();

	ofEvent<RemotePeerStatus> peerArrivedEvent;
	ofEvent<RemotePeerStatus> peerReadyEvent;
	ofEvent<RemotePeerStatus> peerLeftEvent;

	void setup(int videoWidth, int videoHeight, int networkInterfaceId = -1);
	void update();

	static Poco::Net::NetworkInterface::List GetAllNetworkInterfaces();
	static Poco::Net::IPAddress GetBroadcastAddress(Poco::Net::NetworkInterface interface);
	static std::string MACtoString(const std::vector<unsigned char>& mac, char delimter = ':');
};