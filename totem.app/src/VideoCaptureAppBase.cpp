#include "VideoCaptureAppBase.h"
#include "..\..\SharedCode\VideoConverters.h"
#include "..\..\SharedCode\ofxFFmpegVideoReceiver.h"
#include "ofxGstRTPClientAsVideoSource.h"
#include <vector>
#include <queue>

const int SAMPLE_RATE = 22050;
const int AUDIO_CHANNELS = 1;
const int AUDIO_BUFFER_FRAME_RATE = 2; // Audio packets per second
const int AUDIO_BUFFER_CIRCULAR_SIZE = 20;
const int AUDIO_CHUNK_SIZE = (SAMPLE_RATE * AUDIO_CHANNELS * sizeof(float)) * AUDIO_BUFFER_FRAME_RATE;

FILE *fp = NULL;

VideoCaptureAppBase::VideoCaptureAppBase() : audioBuffer(1024 * 1024)
{

}

void VideoCaptureAppBase::setup(int networkInterfaceId, bool isTotemSource)
{
	this->udpDiscovery.setup(this->videoSource->getWidth(), this->videoSource->getHeight(), networkInterfaceId, isTotemSource);
	this->setupStreamManager();

	// Initailize the broadcast stream!
	auto myIp = this->udpDiscovery.GetLocalAddress();
	auto str = myIp.toString();
	auto dotIndex = str.find_last_of('.');
	std::string ipAddress("239.0.0." + str.substr(dotIndex + 1));
	uint16_t videoPort = 11005;
	this->ffmpegVideoBroadcast.reset(new EncodeRGBToH264Live());
	this->ffmpegVideoBroadcast->Start(ipAddress, videoPort, this->videoSource->getWidth(), this->videoSource->getHeight(), 15);

	//UdpDiscovery::RemotePeerStatus peer;
	//peer.id = "Local Network Echo";
	//peer.ipAddress = ipAddress;
	//peer.port = videoPort;
	//peer.isTotem = false;
	//peer.videoWidth = 640;
	//peer.videoHeight = 480;
	//this->PeerArrived(peer);

	if (!fp) fp = fopen("rawaudioout.pcm", "wb");

	ofSoundStreamListDevices();
	ofSoundStreamSetup(0, 1, this, SAMPLE_RATE, 512, 8);
	//ofSoundStreamStart();
}

void VideoCaptureAppBase::audioOut(float * output, int bufferSize, int nChannels)
{

}


void VideoCaptureAppBase::audioIn(float * input, int bufferSize, int nChannels)
{
	bufferSize *= sizeof(float);
	audioBuffer.Write((uint8_t*)input, bufferSize);
}

void VideoCaptureAppBase::update()
{
	int bytesWritten = audioBuffer.Read(audioToProcess, sizeof(audioToProcess));
	fwrite(audioToProcess, 1, bytesWritten, fp);

	this->udpDiscovery.update();

	this->videoSource->update();
	if (this->videoSource->isFrameNew())
	{
		auto ref = this->videoSource->getPixelsRef();
		if (this->ffmpegVideoBroadcast.get()) this->ffmpegVideoBroadcast->WriteFrame(ref.getPixels());
	}

	//std::vector<std::vector<ofxFFmpegVideoReceiver *>::iterator> toMove;
	//for (auto iter = this->remoteVideoSourcesConnecting.begin(); iter != this->remoteVideoSourcesConnecting.end(); ++iter)
	//{
	//	auto remoteVideoSource = *iter;
	//	if (remoteVideoSource->isConnected)
	//	{
	//		toMove.push_back(iter);
	//	}
	//}

	//for (auto iter = toMove.begin(); iter != toMove.end(); ++iter)
	//{
	//	this->remoteVideoSources.push_back(**iter);
	//	this->remoteVideoSourcesConnecting.erase(*iter);
	//	this->Handle_ClientConnected(*iter);
	//}


	for (auto iter = this->peers.begin(); iter != this->peers.end(); ++iter)
	{
		auto peer = *iter;
		peer.netClient->update();
		peer.videoDraws->update();
		//peer.videoCroppable->update();
		if (peer.netClient->isFrameNew())
		{
			auto found = std::find(this->remoteVideoSources.begin(), this->remoteVideoSources.end(), peer.netClient);
			if (found == this->remoteVideoSources.end())
			{
				this->remoteVideoSources.push_back(peer.netClient);
				this->Handle_ClientConnected(peer);
			}
		}
	}
}

void VideoCaptureAppBase::exit()
{
	fflush(fp);
	//ofSoundStreamClose();
	fclose(fp);

	this->videoSource->close();
	this->ffmpegVideoBroadcast->Close();

	//for (auto iter = this->remoteVideoSourcesConnecting.begin(); iter != this->remoteVideoSourcesConnecting.end(); ++iter)
	//{
	//	auto remoteVideoSource = *iter;
	//	remoteVideoSource->Close();
	//	delete remoteVideoSource;
	//}
	
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
	//this->remoteVideoSourcesConnecting.push_back(receiver);

	RemoteVideoInfo remote;
	remote.hasLiveFeed = false;
	remote.netClient = receiver;
	remote.peerStatus = peer;
	remote.videoDraws = ofPtr<ofBaseVideoDraws>(new ofxFFmpegVideoReceiverAsVideoSource(receiver));
	remote.videoCroppable = ofPtr<CroppedDrawable>(new CroppedDrawableVideoDraws(remote.videoDraws));

	receiver->start(peer.ipAddress, peer.port);

	this->peers.push_back(remote);
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
	for (auto iter = this->remoteVideoSources.begin(); iter != this->remoteVideoSources.end(); ++iter)
	{
		auto remoteVideoSource = *iter;
		if (remoteVideoSource->clientId == peer.id)
		{
			remoteVideoSource->Close();
			delete remoteVideoSource;
			this->remoteVideoSources.erase(iter);
			break;
		}
	}

	auto remote = GetRemoteFromClientId(peer.id);
	if (remote != this->peers.end())
	{
		auto peer = *remote;
		this->peers.erase(remote);
		this->Handle_ClientDisconnected(peer);
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