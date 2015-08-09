#include "VideoCaptureAppBase.h"
#include "..\..\SharedCode\VideoConverters.h"
#include "..\..\SharedCode\ofxFFmpegVideoReceiver.h"
#include "ofxGstRTPClientAsVideoSource.h"
#include <vector>
#include <queue>
#include "Windows.h"

namespace
{
	const int SAMPLE_RATE = 22050;
	const int AUDIO_CHANNELS = 1;
	const int AUDIO_BUFFER_FRAME_RATE = 2; // Audio packets per second
	const int AUDIO_BUFFER_CIRCULAR_SIZE = 20;
	const int AUDIO_CHUNK_SIZE = (SAMPLE_RATE * AUDIO_CHANNELS * sizeof(float)) * AUDIO_BUFFER_FRAME_RATE;
	
	//FILE *fp;
	//FILE *fpraw;
}

VideoCaptureAppBase::VideoCaptureAppBase() :
	audioBufferInput(1024 * 16),
	audioBufferOutput(1024 * 1024),
	audioWriteThreadCancel(CreateEventA(NULL, TRUE, FALSE, NULL))
{
	//fp = fopen("raw-s16mono22k.pcm", "wb");
	//fpraw = fopen("raw-f32mono22k.pcm", "wb");
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
	ofSoundStreamListDevices();
	ofSoundStreamSetup(0, 1, this, SAMPLE_RATE, 512, 8);
	//ofSoundStreamStart();
	
	outputStream = ofPtr<ofSoundStream>(new ofSoundStream());
	//outputStream->setup(this, 2, 0, 44100, 512, 8);
	outputStream->setup(this, 1, 0, 22050, 512, 8);

	this->audioWriteThreadHandle = CreateThread(NULL, 0, &VideoCaptureAppBase::AudioWriteThreadStarter, this, 0, NULL);
}

void VideoCaptureAppBase::audioOut(float * output, int bufferSize, int nChannels)
{
	bufferSize *= sizeof(float);

	// Fill whatever we can into the last read buffer
	// That way, if we get nothing it will still be the same samples from before
	if (nChannels == 1)
	{
		for (int i = 0; i < this->remoteVideoSources.size(); ++i)
		{
			if (i == 0)
			{
				auto cbRead = this->remoteVideoSources[0]->audioBuffer.Read(lastAudioOutputBuffer, bufferSize);
			}
			else
			{
				auto cbRead = this->remoteVideoSources[0]->audioBuffer.Read(mixbuffer, bufferSize);
				for (int x = 0; x < cbRead / sizeof(float); ++x)
				{
					((float*)lastAudioOutputBuffer)[x] += ((float*)mixbuffer)[x];
					if (((float*)lastAudioOutputBuffer)[x] > 1) ((float*)lastAudioOutputBuffer)[x] = 1;
					if (((float*)lastAudioOutputBuffer)[x] > -1) ((float*)lastAudioOutputBuffer)[x] = -1;
				}
			}

			memcpy(output, lastAudioOutputBuffer, bufferSize);
		}
	}
	else
	{
		auto cbRead = this->remoteVideoSources[0]->audioBuffer.Read(lastAudioOutputBuffer, bufferSize / 2);
		memcpy(output, lastAudioOutputBuffer, bufferSize / 2);
		memcpy(output + bufferSize / 2, lastAudioOutputBuffer, bufferSize);
		// Convert from mono to stereo
		//for (int i = 0; i < bufferSize / sizeof(float); ++i)
		//{
		//	((float*)output)[i*2]   = ((float*)lastAudioOutputBuffer)[i];
		//	((float*)output)[i*2+1] = ((float*)lastAudioOutputBuffer)[i + 1];
		//}
	}
}

void VideoCaptureAppBase::audioIn(float * input, int bufferSize, int nChannels)
{
	bufferSize *= sizeof(float);
	audioBufferInput.Write((uint8_t*)input, bufferSize);
}

DWORD VideoCaptureAppBase::AudioWriteThreadStarter(LPVOID context)
{
	if (context)
	{
		((VideoCaptureAppBase*)context)->AudioWriteThread();
		return 0;
	}
}

void VideoCaptureAppBase::AudioWriteThread()
{
	while (WaitForSingleObject(this->audioWriteThreadCancel, 0) == WAIT_TIMEOUT)
	{
		if (this->ffmpegVideoBroadcast.get())
		{
			int bytesReceived = audioBufferInput.Read(audioToProcess + audioLeftover, sizeof(audioToProcess) - audioLeftover);
			if (bytesReceived)
			{
				//fwrite(audioToProcess + audioLeftover, 1, bytesReceived, fpraw);

				// Convert to S16 sample depth (IN PLACE)
				float *src = (float *)(audioToProcess + audioLeftover);
				int16_t *dst = (int16_t*)src;
				for (int i = 0; i < bytesReceived / sizeof(float); ++i)
				{
					float samplef = src[i];
					int16_t samplei;
					if (samplef >= 0)
						samplei = (int16_t)(samplef * SHRT_MAX);
					else
						samplei = (int16_t)(samplef * (SHRT_MAX - 1));
					dst[i] = samplei;
				}

				bytesReceived /= 2;
				int bytesEncoded = this->ffmpegVideoBroadcast->WriteAudioFrame(audioToProcess, bytesReceived + audioLeftover);
				//fwrite(audioToProcess, 1, bytesEncoded, fp);
				audioLeftover += bytesReceived - bytesEncoded;
				memmove(audioToProcess, audioToProcess + bytesEncoded, audioLeftover);
			}
		}
	}
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
	//ofSoundStreamClose();
	if (outputStream)
	{
		outputStream->close();
		outputStream.reset();
	}

	SetEvent(this->audioWriteThreadCancel);
	WaitForSingleObject(this->audioWriteThreadHandle, INFINITE);

	this->videoSource->close();
	this->ffmpegVideoBroadcast->Close();

	//fclose(fp);
	//fclose(fpraw);

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