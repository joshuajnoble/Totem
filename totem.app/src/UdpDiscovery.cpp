#include "UdpDiscovery.h"
#include <Poco/Mutex.h>
#include <Poco/URI.h>

void UdpDiscovery::setup()
{
	this->sender.Create();
	this->sender.Connect(this->broadcastAddress, this->broadcastPort);
	this->sender.SetNonBlocking(true);
	this->sender.SetEnableBroadcast(true);

	this->receiver.Create();
	this->receiver.Bind(this->broadcastPort);
	this->receiver.SetNonBlocking(true);

	this->nextSendTime = 0;
	this->myid = ofToString(ofRandomf());
	memset(this->incomingMessage, 0, sizeof(this->incomingMessage));
}

UdpDiscovery::~UdpDiscovery()
{
	this->receiver.Close();
	auto jsonPayload = GetNetworkPayload("disconnect");
	SendJsonPayload(jsonPayload);
}

ofxJSONElement UdpDiscovery::GetNetworkPayload(const std::string& action)
{
	ofxJSONElement jsonPayload;
	jsonPayload["id"] = this->myid;
	jsonPayload["version"] = this->version;
	jsonPayload["action"] = action;
	return jsonPayload;
}

void UdpDiscovery::SendJsonPayload(const ofxJSONElement& jsonPayload)
{
	string payload;
	Poco::URI::encode(jsonPayload.getRawString(false), "/", payload);
	this->sender.Send(payload.c_str(), payload.length());
}

void UdpDiscovery::update()
{

	auto currentTime = ofGetElapsedTimef();
	if (currentTime >= this->nextSendTime)
	{
		Poco::Mutex::ScopedLock lock(this->portmutex);
		this->nextSendTime = currentTime + this->broadcastDelay;

		auto jsonPayload = GetNetworkPayload("dns");
		jsonPayload["version"] = this->version;
		jsonPayload["timestamp"] = ofGetSystemTime();
		jsonPayload["videoWidth"] = 1280;
		jsonPayload["videoHeight"] = 720;
		
		// Publish all of our port mappings to the other clients
		for (auto iter = this->remoteClientMap.begin(); iter != this->remoteClientMap.end(); ++iter)
		{
			jsonPayload[iter->second.id] = iter->second.assignedLocalPort;
			SendJsonPayload(jsonPayload);
		}

		// Check for expired remote peers and handle "disconnect" from timeout
		std::vector<string> peersToRemove;
		for (auto iter = this->remoteClientMap.begin(); iter != this->remoteClientMap.end(); ++iter)
		{
			if (currentTime >= iter->second.disconnectTime)
			{
				peersToRemove.push_back(iter->second.id);
			}
		}

		for (auto iter = peersToRemove.begin(); iter != peersToRemove.end(); ++iter)
		{
			HandleDisconnect(*iter);
		}
	}

	auto bytesReceived = this->receiver.Receive(this->incomingMessage, sizeof(this->incomingMessage));
	if (bytesReceived != -1)
	{
		char sender[1024];
		this->receiver.GetRemoteAddr(sender);

		string decodedJson;
		Poco::URI::decode(this->incomingMessage, decodedJson);
		ofxJSONElement jsonPayload;
		if (jsonPayload.parse(decodedJson))
		{
			if (jsonPayload.isMember("id") && jsonPayload.isMember("version") && jsonPayload["version"].asString() == this->version && jsonPayload["id"] != this->myid)
			{
				Poco::Mutex::ScopedLock lock(this->portmutex);
				auto remoteId = jsonPayload["id"].asString();

				if (jsonPayload["action"].asString() == "dns")
				{
					auto peerIter = this->remoteClientMap.find(remoteId);
					if (peerIter == this->remoteClientMap.end())
					{
						HandleDiscovery(jsonPayload, sender);

						// peerIter should be in the list now, if it wasn't already
						peerIter = this->remoteClientMap.find(remoteId);
					}

					peerIter->second.disconnectTime = currentTime + this->broadcastMissingDuration;
				}
				else if (jsonPayload["action"] == "disconnect")
				{
					HandleDisconnect(remoteId);
				}
			}

#ifdef _DEBUG
			//std::stringstream debug;
			//debug << "Received packet from " << sender << " -- " << decodedJson << std::endl;
			//OutputDebugStringA(debug.str().c_str());
			//ofLogNotice("UdpDiscovery") << debug.str().c_str();
#endif
		}

		memset(this->incomingMessage, 0, bytesReceived);
	}
}


void UdpDiscovery::HandleDiscovery(const ofxJSONElement& jsonPayload, const string& remoteAddress)
{
	RemotePeerStatus peer;
	peer.id = jsonPayload["id"].asString();
	peer.assignedLocalPort = this->myNextPort;
	peer.ipAddress = remoteAddress;

	this->remoteClientMap[peer.id] = peer;
	this->myNextPort += this->portIncrement;

#ifdef _DEBUG
	std::stringstream debug;
	debug << "Discovered " << peer.id << " at " << peer.ipAddress << std::endl;
	OutputDebugStringA(debug.str().c_str());
	ofLogNotice("UdpDiscovery") << debug.str().c_str();
#endif
}


void UdpDiscovery::HandleDisconnect(const string& remoteId)
{
	auto peerIter = this->remoteClientMap.find(remoteId);
	if (peerIter != this->remoteClientMap.end())
	{
		auto peer = peerIter->second;
		this->remoteClientMap.erase(peerIter);

#ifdef _DEBUG
		std::stringstream debug;
		debug << "Disconnect sent from " << peer.id << " at " << peer.ipAddress << std::endl;
		OutputDebugStringA(debug.str().c_str());
		ofLogNotice("UdpDiscovery") << debug.str().c_str();
#endif
	}
}
