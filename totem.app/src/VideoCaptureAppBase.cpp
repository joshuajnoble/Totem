#include "VideoCaptureAppBase.h"

void VideoCaptureAppBase::setup(int networkInterfaceId, bool isTotemSource)
{
	this->udpDiscovery.setup(this->videoSource->getWidth(), this->videoSource->getHeight(), networkInterfaceId, isTotemSource);
	this->setupStreamManager();

	// Initailize the broadcast stream!
	StreamManager::clientParameters connection;
	auto myIp = this->udpDiscovery.GetLocalAddress();
	auto str = myIp.toString();
	auto dotIndex = str.find_last_of('.');
	connection.ipAddress = "239.0.0." + str.substr(dotIndex + 1);
	connection.remoteVideoPort = 12000;
	connection.remoteAudioPort = 12100;
	this->streamManager.newServer(connection);

	//this->ffmpegEncoder.reset(new EncodeRGBToH264File(m_ffmpeg, "test.h264"));
	//this->ffmpegEncoder->Start(this->videoSource->getWidth(), this->videoSource->getHeight(), 15);

	//this->ffmpegNetworkServer.reset(new FFmpegNetworkServer(m_ffmpeg));
	//this->ffmpegNetworkServer->Start(this->videoSource->getWidth(), this->videoSource->getHeight(), 15, "239.0.0.200:2000");
	
	//this->ffmpegLiveTest.reset(new EncodeRGBToH264Live(m_ffmpeg));
	//this->ffmpegLiveTest->Start(this->videoSource->getWidth(), this->videoSource->getHeight(), 15);

	//TestStreamer testStreamer;
	//testStreamer.Start();

	//auto ss = ofSoundStream();
	//ss.listDevices();
	//ss.setDeviceID(0);
	//auto rval = ss.setup(this, 0, 2, 4400, 256, 4);
	//ss.start();
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
		this->streamManager.newFrame(ref);
		if (this->ffmpegEncoder.get()) this->ffmpegEncoder->WriteFrame(ref.getPixels());
		if (this->ffmpegNetworkServer.get()) this->ffmpegNetworkServer->WriteFrame(ref.getPixels());
		if (this->ffmpegLiveTest.get()) this->ffmpegLiveTest->WriteFrame(ref.getPixels());
	}

	this->streamManager.update();
}

void VideoCaptureAppBase::exit()
{
	ofSoundStreamClose();
	this->streamManager.exit();
}

void VideoCaptureAppBase::newClient(string& args)
{
	RemoteVideoInfo remote;
	remote.clientId = args;
	remote.netClient = this->streamManager.clients[args];
	auto video = this->streamManager.remoteVideos[args];
	remote.width = video->getWidth();
	remote.height = video->getHeight();
	auto wrapped = new CroppedDrawableFbo(this->streamManager.remoteVideos[args]);
	remote.source = ofPtr<CroppedDrawable>(wrapped);
	remote.isTotem = this->udpDiscovery.GetPeerStatus(args).isTotem;
	this->remoteVideoSources.push_back(remote);
	Handle_ClientConnected(remote);
}

void VideoCaptureAppBase::clientDisconnected(string& clientId)
{
	auto found = GetRemoteFromClientId(clientId);
	if (found != this->remoteVideoSources.end())
	{
		auto remote = *found;
		this->remoteVideoSources.erase(found);
		Handle_ClientDisconnected(remote);
	}
}

void VideoCaptureAppBase::clientStreamAvailable(string& clientId)
{
	auto found = GetRemoteFromClientId(clientId);
	if (found != this->remoteVideoSources.end())
	{
		found->hasLiveFeed = true;
		Handle_ClientStreamAvailable(*found);
	}
}

void VideoCaptureAppBase::PeerArrived(UdpDiscovery::RemotePeerStatus& peer)
{
	StreamManager::clientParameters connection;
	connection.clientID = peer.id;
	connection.ipAddress = peer.ipAddress;
	connection.videoWidth = peer.videoWidth;
	connection.videoHeight = peer.videoHeight;
	connection.videoPort = peer.assignedLocalPort;
	connection.audioPort = peer.assignedLocalPort + 100;
	this->streamManager.newClient(connection);
}

void VideoCaptureAppBase::peerReady(UdpDiscovery::RemotePeerStatus& peer)
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
}

void VideoCaptureAppBase::PeerLeft(UdpDiscovery::RemotePeerStatus& peer)
{
	this->streamManager.ClientDisconnected(peer.id);
}

void VideoCaptureAppBase::setupStreamManager()
{
	streamManager.setup(this->videoSource->getWidth(), this->videoSource->getHeight());

	ofAddListener(udpDiscovery.peerArrivedEvent, this, &VideoCaptureAppBase::PeerArrived);
	ofAddListener(udpDiscovery.peerReadyEvent, this, &VideoCaptureAppBase::peerReady);
	ofAddListener(udpDiscovery.peerLeftEvent, this, &VideoCaptureAppBase::PeerLeft);

	ofAddListener(streamManager.newClientEvent, this, &VideoCaptureAppBase::newClient);
	ofAddListener(streamManager.clientDisconnectedEvent, this, &VideoCaptureAppBase::clientDisconnected);
	ofAddListener(streamManager.clientStreamAvailableEvent, this, &VideoCaptureAppBase::clientStreamAvailable);
}

std::vector<RemoteVideoInfo>::iterator VideoCaptureAppBase::GetRemoteFromClientId(const string& clientId)
{
	for (auto iter = this->remoteVideoSources.begin(); iter != this->remoteVideoSources.end(); ++iter)
	{
		auto remote = *iter;
		if (remote.clientId == clientId)
		{
			return iter;
		}
	}

	return this->remoteVideoSources.end();
}