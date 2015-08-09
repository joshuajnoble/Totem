#pragma once

#include "ofMain.h"
#include "RemoteVideoInfo.h"
#include "UdpDiscovery.h"
#include "..\..\SharedCode\FFmpegImports.h"

class EncodeRGBToH264Live;
class ofxFFmpegVideoReceiver;

class VideoCaptureAppBase : public ofBaseApp
{
public:
	VideoCaptureAppBase();
	virtual ~VideoCaptureAppBase() {};

	virtual int displayWidth() const = 0;
	virtual int displayHeight() const = 0;
	ofPtr<ofBaseVideoDraws> videoSource;
	ofMutex videoLock;
	volatile bool hasNewVideo = false;

	virtual void setup(int networkInterfaceId, bool isTotem = false);
	virtual void update();
	virtual void exit();

private:
	void PeerArrived(UdpDiscovery::RemotePeerStatus& peer);
	void PeerLeft(UdpDiscovery::RemotePeerStatus& peer);
	
	void PeerJoinedSession(UdpDiscovery::RemotePeerStatus& peer);
	void PeerLeftSession(UdpDiscovery::RemotePeerStatus& peer);

	static DWORD WINAPI AudioWriteThreadStarter(LPVOID);
	void AudioWriteThread();
	HANDLE audioWriteThreadHandle;
	HANDLE audioWriteThreadCancel;

protected:
	uint8_t lastAudioOutputBuffer[512*sizeof(float) * 2];
	uint8_t mixbuffer[512 * sizeof(float) * 2];
	RingBuffer audioBufferInput;
	RingBuffer audioBufferOutput;
	int audioLeftover = 0;;
	uint8_t audioToProcess[1024 * 1024];
	ofPtr<ofSoundStream> outputStream;

	UdpDiscovery udpDiscovery;
	std::auto_ptr<EncodeRGBToH264Live> ffmpegVideoBroadcast;

	std::vector<RemoteVideoInfo> peers;
	std::vector<RemoteVideoInfo>::iterator GetRemoteFromClientId(const string& clientId);

	void ConnectToSession();
	void DisconnectSession();

	virtual void Handle_ClientConnected(RemoteVideoInfo& remote) = 0;
	virtual void Handle_ClientDisconnected(RemoteVideoInfo& remote) = 0;
	virtual void Handle_ClientStreamAvailable(RemoteVideoInfo& remote) {};

	void setupDiscovery();

	virtual void audioOut(float * output, int bufferSize, int nChannels);
	virtual void audioIn(float * input, int bufferSize, int nChannels);
	//virtual void audioRequested(float * output, int bufferSize, int nChannels);
	//virtual void audioReceived(float * input, int bufferSize, int nChannels);	
};