#include "UdpDiscovery.h"
#include <Poco/Mutex.h>
#include <Poco/URI.h>
#include "Utils.h"

/*
 * OVERVIEW:
 * We don't want the clients/peers to have to configure anything on the network and they can't connect to each other
 * if they don't have the proper settings.  So we need a way for them to discover each other and to be able to
 * connect automatically.  We do that by sending UDP broadcast messages to the subnet that each client can get
 * without having to know the IP Address and Port (or even the existence) of each other peer.
 *
 * DETAILS:
 * UDP is unreliable, so we send a steady quantity of messages continuously.  Each packet is a JSON encoded data
 * record that advertises the information that a remote needs to know (like video width and height).  We don't
 * have to send our own IP address explicitly because it is part of the packet information that is available to
 * each peer when they receive any UDP packet.  You do have to provide a port for each peer to connect on, though.
 * In fact, each peer will need multiple ports for video and audio streaming.
 *
 * The GStreamer system is what actually sends, receives and synchs the video streams.  GStreamer is using a dedicated
 * set of connections between each peer.  That means that we need a unique starting port address for each peer that we
 * want to send video to.  The remote peer will also have a port specifically reserved for us to connect to.  Given
 * the starting port, GStreamer will open up more ports (in sequence) as needed.  For this reason, we have to skip
 * a certain number of ports between each client, so we don't accidentally have an overlap in our port ranges.
 *
 * We send a custom "DNS" packet to advertise our own information at regular intervals.  Whenever we receive a "DNS"
 * packet from a peer, we track it and can then assign local ports specific to that peer.  We then make the peer to
 * port mapping information part of our own "DNS" broadcast.  Each peer that sees the "DNS" packet will check for its
 * own id in that packet and extract the ports that are assigned to them.  In this way, the "DNS" part of the system
 * does not require any of the peers to have to connect or communicate directly between each other.
 *
 * Since we are expecting to see lots of "DNS" messages from each peer, we can track the last time we saw one for
 * each peer.  If we haven't seen a message in a certain amount of time, we treat that peer as lost/disconnected.
 *
 * We don't want all of the clients to immediately connect to each other because we want to promote the concept that
 * the totem connection is the central piece of the conversation.  So when we don't have a totem source, we don't want
 * the peers connected.  We also don't want to be broadcasting a remote stream unless the user has chosen to connect
 * to the active session.  If the totem peer is lost, then everyone should disconnect.  So we have to adjust
 * exactly when we broadcast each of the port records in the "DNS" packet to get the behaviour we want.
 */

void UdpDiscovery::setup(int w, int h, int networkInterfaceId, bool isTotemSource)
{
	bool found = false;
	auto interfaces = GetAllNetworkInterfaces();
	if (networkInterfaceId == -1) // Use default network interface
	{
		auto found = std::find_if(interfaces.begin(), interfaces.end(), [](const Poco::Net::NetworkInterface& i) { return -1 != find_indexof_substr_nocase(i.name(), std::string("ethernet")); });
		if (found != interfaces.end())
		{
			networkInterfaceId = found->index();
		}
		else
		{
			networkInterfaceId = interfaces[0].index();
		}
	}

	this->isTotem = isTotemSource;

	Poco::Net::NetworkInterface interface;
	for (auto iter = interfaces.begin(); !found && iter != interfaces.end(); ++iter)
	{
		if (iter->index() == networkInterfaceId)
		{
			interface = *iter;
			found = true;
		}
	}

	if (!found)
	{
		cout << "The specified network interface id (" << networkInterfaceId << ") was not found." << std::endl;
		ofExit();
	}

	this->interface = interface;

	this->broadcastAddress = GetBroadcastAddress(interface).toString();
	this->myid = MACtoString(this->interface.macAddress());
	this->myid += "/";
	this->myid += ofToString((int)roundf(ofRandomf() * 0xFFFFFF));

	this->videoWidth = w;
	this->videoHeight = h;
	this->sender.Create();
	this->sender.Connect(this->broadcastAddress.c_str(), this->videoBroadcastPort);
	this->sender.SetNonBlocking(true);
	this->sender.SetEnableBroadcast(true);

	this->receiver.Create();
	this->receiver.Bind(this->discoveryBroadcastPort);
	this->receiver.SetNonBlocking(true);

	this->nextSendTime = 0;
	memset(this->incomingMessage, 0, sizeof(this->incomingMessage));

#if defined(TARGET_WIN32) && defined(_DEBUGX)
	if (IsDebuggerPresent())
	{
		this->broadcastMissingDuration = 90.0f; // When debugging don't auto disconnect as aggressively
	}
#endif
}

UdpDiscovery::~UdpDiscovery()
{
	this->receiver.Close();
	auto jsonPayload = GetNetworkPayload("disconnect");
	SendJsonPayload(jsonPayload);
}

UdpDiscovery::RemotePeerStatus UdpDiscovery::GetPeerStatus(const std::string& peerId)
{
	return this->remoteClientMap[peerId];
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
	// This is locking myNextPort, remoteClientMap and incomingMessage.  That is why it is at function scope right now.
	Poco::Mutex::ScopedLock lock(this->portmutex);

	auto currentTime = ofGetElapsedTimef();
	if (currentTime >= this->nextSendTime)
	{
		this->nextSendTime = currentTime + this->broadcastDelay;

		auto jsonPayload = GetNetworkPayload("dns");
		jsonPayload["version"] = this->version;
		jsonPayload["timestamp"] = ofGetSystemTime();
		jsonPayload["videoWidth"] = this->videoWidth;
		jsonPayload["videoHeight"] = this->videoHeight;
		jsonPayload["totem"] = this->isTotem;
		
		// Publish all of our port mappings to the other clients
		//for (auto iter = this->remoteClientMap.begin(); iter != this->remoteClientMap.end(); ++iter)
		//{
		//	jsonPayload[iter->second.id] = iter->second.assignedLocalPort;
		//}

		SendJsonPayload(jsonPayload);

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
			HandleDisconnect(*iter, true);
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
				auto remoteId = jsonPayload["id"].asString();

				if (jsonPayload["action"].asString() == "dns")
				{
					auto peerIter = this->remoteClientMap.find(remoteId);
					if (peerIter == this->remoteClientMap.end())
					{
						HandleDiscovery(jsonPayload, sender);

						// peerIter should be in the map now, if it wasn't already
						peerIter = this->remoteClientMap.find(remoteId);
					}

					peerIter->second.disconnectTime = currentTime + this->broadcastMissingDuration;

					// The remote port won't be here the first time, so keep watching for it.
					// Once the remote peer gets one of our dns packets, it will update it's dns packet with a port for us.
					//auto currentRemotePort = peerIter->second.assignedRemotePort;
					//if (currentRemotePort == 0 && jsonPayload.isMember(this->myid)) 
					//{
					//	auto newRemotePort = jsonPayload[this->myid].asString();
					//	peerIter->second.assignedRemotePort = jsonPayload[this->myid].asInt();
					//	ofNotifyEvent(this->peerReadyEvent, peerIter->second, this);
					//}
				}
				else if (jsonPayload["action"] == "disconnect")
				{
					HandleDisconnect(remoteId);
				}
			}

#ifdef _DEBUGX
			std::stringstream debug;
			debug << "Received packet from " << sender << " -- " << decodedJson << std::endl;
			OutputDebugStringA(debug.str().c_str());
			ofLogNotice("UdpDiscovery") << debug.str().c_str();
#endif
		}

		memset(this->incomingMessage, 0, bytesReceived);
	}
}


void UdpDiscovery::HandleDiscovery(const ofxJSONElement& jsonPayload, const string& remoteAddress)
{
	RemotePeerStatus peer;
	peer.id = jsonPayload["id"].asString();
	peer.port = this->videoBroadcastPort;
	peer.ipAddress = "239.0.0." + remoteAddress.substr(remoteAddress.length() - 3);
	peer.videoWidth = jsonPayload["videoWidth"].asInt();
	peer.videoHeight = jsonPayload["videoHeight"].asInt();
	peer.isTotem = jsonPayload["totem"].asBool();

	this->remoteClientMap[peer.id] = peer;
	//this->myNextPort += this->portIncrement;

	ofNotifyEvent(this->peerArrivedEvent, peer, this);
	ofNotifyEvent(this->peerReadyEvent, peer, this);

#ifdef _DEBUG
	std::stringstream debug;
	debug << "Discovered " << peer.id << " at " << peer.ipAddress << std::endl;
	OutputDebugStringA(debug.str().c_str());
	ofLogNotice("UdpDiscovery") << debug.str().c_str();
#endif
}


void UdpDiscovery::HandleDisconnect(const string& remoteId, bool isTimeout)
{
	auto peerIter = this->remoteClientMap.find(remoteId);
	if (peerIter != this->remoteClientMap.end())
	{
		auto peer = peerIter->second;
		this->remoteClientMap.erase(peerIter);

		ofNotifyEvent(this->peerLeftEvent, peer, this);

#ifdef _DEBUG
		std::stringstream debug;
		debug << "Disconnect " << (isTimeout ? "due to timeout" : "sent") << " from " << peer.id << " at " << peer.ipAddress << std::endl;
		OutputDebugStringA(debug.str().c_str());
		ofLogNotice("UdpDiscovery") << debug.str().c_str();
#endif
	}
}


Poco::Net::NetworkInterface::List UdpDiscovery::GetAllNetworkInterfaces()
{
	auto interfaces = Poco::Net::NetworkInterface::list();

	Poco::Net::NetworkInterface::List rval;
	for (auto iter = interfaces.begin(); iter != interfaces.end(); ++iter)
	{
		auto interface = *iter;
#if _DEBUGX
		auto a = interface.displayName();
		auto b = interface.adapterName();
		auto c = interface.name();
#endif
		if (interface.macAddress().size() && !interface.isLoopback() && interface.supportsIPv4() && interface.macAddress().size())
		{
			rval.push_back(interface);
		}
	}

	return rval;
}

std::string UdpDiscovery::MACtoString(const std::vector<unsigned char>& mac, char delimter)
{
	string macString;
	macString.reserve(mac.size() * 3 - 1);

	char s[4]; // Temp buffer

	// Get the first segment
	auto i = mac.begin();
	sprintf(s, "%02X", *i++);
	macString.append(s);

	// Add all the rest with a delimter
	for (; i != mac.end(); ++i)
	{
		sprintf(s, "%c%02X", delimter, *i);
		macString.append(s);
	}

	return macString;
}

Poco::Net::IPAddress UdpDiscovery::GetBroadcastAddress(Poco::Net::NetworkInterface interface)
{
	auto bytes = (unsigned long *)interface.address().addr();
	auto mask = (unsigned long *)(~interface.subnetMask()).addr();
	long broadcastBytes = (*bytes | *mask);
	auto broadcastAddress = Poco::Net::IPAddress(&broadcastBytes, 4);
	return broadcastAddress;
}

Poco::Net::IPAddress UdpDiscovery::GetLocalAddress()
{
	return this->interface.address();
}