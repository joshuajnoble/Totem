#include "VideoCaptureAppBase.h"
#include "..\..\SharedCode\VideoConverters.h"
#include "..\..\SharedCode\ofxFFmpegVideoReceiver.h"
#include "ofxGstRTPClientAsVideoSource.h"

void VideoCaptureAppBase::setup(int networkInterfaceId, bool isTotemSource)
{
	this->udpDiscovery.setup(this->videoSource->getWidth(), this->videoSource->getHeight(), networkInterfaceId, isTotemSource);
	this->setupStreamManager();

	// Initailize the broadcast stream!
	auto myIp = this->udpDiscovery.GetLocalAddress();
	auto str = myIp.toString();
	auto dotIndex = str.find_last_of('.');
	std::string ipAddress("239.0.0." + str.substr(dotIndex + 1));
	int videoPort = 11005;
	this->ffmpegVideoBroadcast.reset(new EncodeRGBToH264Live());
	this->ffmpegVideoBroadcast->Start(ipAddress, to_string(videoPort), this->videoSource->getWidth(), this->videoSource->getHeight(), 15);
}

void VideoCaptureAppBase::audioOut(float * output, int bufferSize, int nChannels)
{

}

void VideoCaptureAppBase::audioIn(float * input, int bufferSize, int nChannels)
{

}

void VideoCaptureAppBase::update()
{
	this->udpDiscovery.update();

	this->videoSource->update();
	if (this->videoSource->isFrameNew())
	{
		auto ref = this->videoSource->getPixelsRef();
		if (this->ffmpegVideoBroadcast.get()) this->ffmpegVideoBroadcast->WriteFrame(ref.getPixels());
	}

	for (auto iter = this->peers.begin(); iter != this->peers.end(); ++iter)
	{
		auto peer = *iter;
		peer.netClient->update();
		if (peer.netClient->isFrameNew())
		{
		}
	}
}

void VideoCaptureAppBase::exit()
{
	ofSoundStreamClose();
	this->videoSource->close();
	this->ffmpegVideoBroadcast->Close();

	for (auto iter = this->remoteVideoSources.begin(); iter != this->remoteVideoSources.end(); ++iter)
	{
		auto remoteVideoSource = *iter;
		remoteVideoSource->Close();
		delete remoteVideoSource;
	}
}

void VideoCaptureAppBase::PeerArrived(UdpDiscovery::RemotePeerStatus& peer)
{
	auto receiver = new ofxFFmpegVideoReceiver(peer.id);
	this->remoteVideoSources.push_back(receiver);

	RemoteVideoInfo remote;
	remote.hasLiveFeed = false;
	remote.netClient = receiver;
	remote.peerStatus = peer;
	ofPtr<ofBaseVideoDraws> videoDraws(new ofxFFmpegVideoReceiverAsVideoSource(receiver));
	remote.videoSource = ofPtr<CroppedDrawable>(new CroppedDrawableVideoDraws(videoDraws));

	receiver->start(peer.ipAddress, peer.port);

	this->peers.push_back(remote);
	this->Handle_ClientConnected(remote);
}

void VideoCaptureAppBase::PeerReady(UdpDiscovery::RemotePeerStatus& peer)
{
	//if (!connected)
	//{
	//	connected = true;
	//	StreamManager::clientParameters connection;
	//	connection.clientID = peer.id;
	//	auto myIp = this->udpDiscovery.GetLocalAddress();
	//	auto str = myIp.toString();
	//	str = "239.0.0." + str.substr(str.length() - 3);
	//	connection.ipAddress = str;// = peer.ipAddress;
	//	connection.remoteVideoPort = peer.assignedRemotePort;
	//	connection.remoteAudioPort = peer.assignedRemotePort + 5;
	//	this->streamManager.newServer(connection);
	//}
	
	auto remote = GetRemoteFromClientId(peer.id);
	if (remote != this->peers.end())
	{
		remote->hasLiveFeed = false;
		this->Handle_ClientStreamAvailable(*remote);
	}
}

void VideoCaptureAppBase::PeerLeft(UdpDiscovery::RemotePeerStatus& peer)
{
	//this->streamManager.ClientDisconnected(peer.id);
	auto remote = GetRemoteFromClientId(peer.id);
	if (remote != this->peers.end())
	{
		this->Handle_ClientDisconnected(*remote);
	}
}

void VideoCaptureAppBase::setupStreamManager()
{
	//streamManager.setup(this->videoSource->getWidth(), this->videoSource->getHeight());

	ofAddListener(udpDiscovery.peerArrivedEvent, this, &VideoCaptureAppBase::PeerArrived);
	ofAddListener(udpDiscovery.peerReadyEvent, this, &VideoCaptureAppBase::PeerReady);
	ofAddListener(udpDiscovery.peerLeftEvent, this, &VideoCaptureAppBase::PeerLeft);

	//ofAddListener(streamManager.newClientEvent, this, &VideoCaptureAppBase::newClient);
	//ofAddListener(streamManager.clientDisconnectedEvent, this, &VideoCaptureAppBase::clientDisconnected);
	//ofAddListener(streamManager.clientStreamAvailableEvent, this, &VideoCaptureAppBase::clientStreamAvailable);
}

std::vector<RemoteVideoInfo>::iterator VideoCaptureAppBase::GetRemoteFromClientId(const string& clientId)
{
	for (auto iter = this->peers.begin(); iter != this->peers.end(); ++iter)
	{
		auto remote = *iter;
		if (remote.peerStatus.id == clientId)
		{
			return iter;
		}
	}

	return this->peers.end();
}